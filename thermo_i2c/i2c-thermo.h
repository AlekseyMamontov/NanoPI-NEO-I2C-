/*
 * Одновременное чтение температуры с нескольких чипов (max6675,max31855)
 * до 6 шт. по i2c c помощью микросхемы MCP23017 в Linux.
 * Возможно с 14шт. если нужно :)
 * 
 * Simultaneous reading of temperature from several chips (max6675, max31855)
 * to 6 pcs. on i2c using the MCP23017 chip in Linux.
 * Perhaps with 14pcs. if needed :)
 * 
 * (c) Mamontov Aleksey
 *  https://github.com/AlekseyMamontov
 * 
 * Data on temperature readings are stored in these structures.
 * dependencies
 * mcp_23017.h сhip
 * tft-i2c.h  display
 */
  
//#include <mcp_23017.h>
#include <signal.h>
#include <time.h>  


#define MAX_thermal_sensor 4  // 1...6 or 14))


/* --------------------------------------------------------------------*
 * Структура для опроса чипов температуры c помощью mcp23017(18) по i2c*
 *---------------------------------------------------------------------*/

struct Thermosensors{
	
	struct nGPIO *gpio;
	  	
	/* 
	 номер пина который выбран как CLK
	 исходя из него выбирается регистр (порт) GPIOA или GPIOB
	*/
	uint8_t  pin_clk;
	
	/*пин который выбран для CS
	 желательно выбирать пин 7(15), так как из-за аппаратной ошибки чипа mcp23017
	 этот пин надо выставлять только на выход. 
	 при выборе пинов GPIOA7,GPIOB7 как вход - если в момент чтения из него происходит 
	 смена уровня на нем, происходит сбой с выставлением сигнала STOP на линии.
	 проблема не решена. errata mcp23017 microchip.
	*/
	uint8_t  pin_cs;
	
	// массив номеров пинов выбраные как MISO, должны находится на том же
	// порту А или В, где и пины  CLK,CS. 	 
	uint8_t  pin_miso[MAX_thermal_sensor];
	
	// количество подключенных  MAX6675	
	uint8_t  N_thermal_sensors; // <= 6
		
	// сырые данные из MAX6675  16bit 
	uint16_t thermo_raw[MAX_thermal_sensor];

	// корректировка температуры
	int8_t   t_correct[MAX_thermal_sensor];

	// хранится температура  без 0,25 (целое число)
	int16_t  thermo_data[MAX_thermal_sensor];	

	// текстовое представление температуры
	uint8_t  thermo_char[MAX_thermal_sensor][6];
	uint8_t  thermo_char_visible[MAX_thermal_sensor];
	uint8_t  N_text_char;
	
	// подключена ли термопара или нет 0 -yes
	// 4 - нет термолары. 6 - нет устройства  
	
	uint8_t  check_thermocouple[MAX_thermal_sensor];	
		
/* 
* По даташиту время опроса не должно быть меньше 250мс, если Max6675
* опрашивать чаще то будет постоянно сбрасываться ацп и результат зависнет  
* nanosleep ( struct timespec , NULL); 
*/	

	struct itimerspec time;

	// Переменная для цикла опроса 0 - не время ), 1 - опросить микросхемы
	int time_cycle;


	// Опрос температуры в отдельном потоке 
	pthread_t pthread_id;
	void (*pthread_reading_temperature)(void *);

	// Опрос с помощью таймера

	timer_t timer;	
	struct sigevent timer_evp;	
	
	//Определить функцию для реакции на таймер
	void (*timer_handler)();
	
	//Иницилизировать опрос температуры в цикле (создать и запустить таймер)
	void (*init_timer)(struct Thermosensors *);
    void (*init_timer_atrrib)(struct Thermosensors *,int *,time_t *,long *);

	
	void (*temperature_reading)(struct Thermosensors *);
	int (*temperature_raw_processing)(struct Thermosensors *);
	void (*temperature_data_processing)(struct Thermosensors *);
};

/* --------------------------------------------------------------------*
 * Опрос температуры c помощью отдельного потока - лучший вариант	   *
 *---------------------------------------------------------------------*/

void pthread_reading_thermosensors(void *temp){
	
 struct Thermosensors *temp_sensors;
 
 temp_sensors = (struct Thermosensors *) temp;
	
 while(temp_sensors->time_cycle){

 temp_sensors->temperature_raw_processing(temp_sensors);
 temp_sensors->temperature_data_processing(temp_sensors);
 
 nanosleep(&temp_sensors->time.it_value, NULL);
 
};	
	
pthread_exit(NULL);	
};


void pthread_start_thermosensors(struct Thermosensors *thermosensor){

 thermosensor->time_cycle = 1;
 
 int potok = pthread_create(&thermosensor->pthread_id,
							NULL,
							(void*)thermosensor->pthread_reading_temperature,
							(void *) thermosensor);
 if (potok){printf("ERROR; return code from pthread_create() is %d\n", potok);
			exit(-1);
			};
		
};

/* --------------------------------------------------------------------*
 * Опрос температуры c помощью сигналов, не самый лучший вариант   	   *
 *---------------------------------------------------------------------*/

void thermosensors_temperature_reading(struct Thermosensors *thermosensor){

	if(!thermosensor->time_cycle) return ;	
		thermosensor->temperature_raw_processing(thermosensor);
		thermosensor->temperature_data_processing(thermosensor);
		thermosensor->time_cycle = 0;
	if(timer_settime(thermosensor->timer,0,&thermosensor->time,NULL)) perror ("timer no start max6675_temperature_polling");
};
/* 
 * Иницилизировать опрос температуры в цикле исходя из структуры Thermosensors 
*/
void thermosensors_init_timer(struct Thermosensors *thermosensor){

// определить функцию для реакции на таймер
	signal(thermosensor->timer_evp.sigev_signo,thermosensor->timer_handler);
// создать таймер цикла
	if(timer_create(CLOCK_REALTIME,&thermosensor->timer_evp,&thermosensor->timer)) perror ("timer no_create max6675_init_polling"); 
// запустить таймер
	if(timer_settime(thermosensor->timer,0,&thermosensor->time,NULL)) perror ("timer no start max6675_init_polling");
};
/* 
 * Иницилизировать опрос температуры, выставить чип, сигнал, секунды, наносекунды 
 */ 	
void thermosensors_init_timer_atrrib(struct Thermosensors *thermosensor,
								int n_signal,
								time_t n_second, 
								long n_nanosecond){

	thermosensor->timer_evp.sigev_signo = n_signal;
	thermosensor->time.it_value.tv_sec = n_second;
	thermosensor->time.it_value.tv_nsec = n_nanosecond;

return thermosensors_init_timer(thermosensor);
};
// example
//void max6675_timer_cycle(){MAX6675.timer_cycle = 1;};


/*--------------------------------------------------------------------------*
 *	 Термостат - блок отслеживающий температуру, включающий или отключающий *
 *	 различные устройства													*
 *--------------------------------------------------------------------------*/

struct Thermostat{

struct Thermosensors *sensors;

	int16_t  *temperatura_data;

	uint16_t temperatura_on;  // Температура включения
	uint16_t temperatura_off; // Температура выключения 

	uint8_t  status_on_off;		// какой режим на данный момент
	
	uint8_t pin_on[5];  // массив содержит пины которые реагируют 
	uint8_t  N_pin_on;			     // на температуру включение - HIGH; 
	uint8_t pin_off[5]; // массив содержит пины которые реагируют 
	uint8_t  N_pin_off;				 // на температуру отключение - LOW; 

	struct TFT_info_block info_block;

void (*check_temperature)(struct Thermostat *);
int (*enabled_device)(uint8_t);
int (*disabled_device)(uint8_t);

};

/*
 *  Проверка  температуры, включение или выключение устройств  
*/
  
void thermostat_check(struct Thermostat *thermostat){

 int16_t temp = *(thermostat->temperatura_data);
				
	if(thermostat->status_on_off){
	
// Блок включил все устройства, теперь проверяем температуру 
// на превышения указаного порога отключения  
		
		if (temp >= thermostat->temperatura_off && thermostat->N_pin_off){
			
			for (int i=0; i < thermostat->N_pin_off; i++){
					thermostat->disabled_device (thermostat->pin_off[i]);
			};
			thermostat->status_on_off = 0;
		};

// Блок выключил все устройства,  теперь проверяем температуру 
// на меньшие значения порога включения 		
	}else{ 
		if (temp <= thermostat->temperatura_on && thermostat->N_pin_on){
			
			for (int i=0; i < thermostat->N_pin_on; i++){
					thermostat->enabled_device(thermostat->pin_on[i]);
			};				
			thermostat->status_on_off = 1;
		};
	
	};
};




