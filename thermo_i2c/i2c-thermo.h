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
 * 
 * tft-i2c.h  display
 */
  
#include <time.h>  
#define MAX_thermal_sensor 4  // 1...6 or 14))
#define MAX_THERMOSTAT 7


/* --------------------------------------------------------------------*
 * Структура для опроса чипов температуры c помощью mcp23017(18) по i2c*
 *---------------------------------------------------------------------*/

struct Thermosensors{
	
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
	uint8_t  pin_miso[MAX_thermal_sensor];
	
	// количество подключенных  MAX6675	
	uint8_t  N_thermal_sensors; // <= 14
		
	// сырые данные из MAX6675  16bit 
	uint16_t thermo_raw[MAX_thermal_sensor];

	// корректировка температуры
	int8_t   t_correct[MAX_thermal_sensor];

	// хранится температура  без 0,25 (целое число)
	int16_t  thermo_data[MAX_thermal_sensor];	

	// текстовое представление температуры для tft_i2c
	
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
	
	int  (*temperature_raw_processing)(struct Thermosensors *);
	void (*temperature_data_processing)(struct Thermosensors *);
};

/*--------------------------------------------------------------------------*
 *	 Термостат - блок отслеживающий температуру, включающий или отключающий *
 *	 различные устройства													*
 *--------------------------------------------------------------------------*/

struct Thermostat{

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
	void (*enabled_device)(uint8_t);
	void (*disabled_device)(uint8_t);

};

/*--------------------------------------------------------------------------*
 *	 Для создания потока -  отслеживающего температуру и термостаты 		*
 * 	 включающие или отключающий различные устройства						*
 *--------------------------------------------------------------------------*/
 
struct Thermostat_Blocks {

struct Thermosensors *thermosensors;
struct Thermostat    *thermostat [MAX_THERMOSTAT];
uint8_t    			 num_thermostat;
uint8_t 					 status;

pthread_t pthread_id;
void (*pthread_thermostat_blocks)(void *);

}; 


/* --------------------------------------------------------------------*
 * Опрос температуры c помощью отдельного потока - лучший вариант	   *
 *---------------------------------------------------------------------*/

void pthread_reading_thermoblock(void *temp){
	
 struct Thermostat_Blocks *thermoblock = (struct Thermostat_Blocks *) temp;
 struct Thermosensors  *thermosensors = thermoblock->thermosensors;
 
 while(thermoblock->status){

	thermosensors->temperature_raw_processing(thermosensors);
	thermosensors->temperature_data_processing(thermosensors);
	
 for(int i=0;i< thermoblock->num_thermostat; i++) 
 
    ((struct Thermostat*)thermoblock->thermostat[i])->check_temperature(((struct Thermostat*)thermoblock->thermostat[i]));
 
 
 nanosleep(&thermosensors->time.it_value, NULL);
 
};	
	
pthread_exit(NULL);	
};

/* --------------------------------------------------------------------*
 * Обновление температуры в термостате ON и OFF для экрана			   *
 *---------------------------------------------------------------------*/
void change_temp_on_off_thermostat(struct Thermostat *stat){
	
	tft_data_to_char (&stat->info_block, 1, // номер элемента в блока
					   stat->info_block.N_change_char_text[1],
					   stat->temperatura_on);
	tft_data_to_char (&stat->info_block, 2, 
					   stat->info_block.N_change_char_text[2],
					   stat->temperatura_off);		
}; 

/* --------------------------------------------------------------------*
 * Запуск потока для опроса температуры и выставить пины как занятые   *
 *---------------------------------------------------------------------*/
 
void pthread_start_thermoblock(struct Thermostat_Blocks *thermoblock){
	
 thermoblock->status = 1;

 struct Thermostat *stat = *(thermoblock->thermostat);
 
 for (int i=0; i< thermoblock->num_thermostat;i++)
  
		change_temp_on_off_thermostat(stat+i); // Выставляем начальные значения 
	 
 int potok = pthread_create(&thermoblock->pthread_id,
							NULL,
							(void*)thermoblock->pthread_thermostat_blocks,
							(void*)thermoblock);
 if (potok){printf("ERROR; return code from pthread_create() is %d\n", potok);
			exit(-1);};		
};

/* --------------------------------------------------------------------*
 * Проверка  температуры, включение или выключение устройств		   *
 *---------------------------------------------------------------------*/

  
void thermostat_check(struct Thermostat *thermostat){
	
 static struct nGPIO_DATA device;	
	
 int16_t temp = *(thermostat->temperatura_data);
 
 if (thermostat->temperatura_off == thermostat->temperatura_on) return;
				
	if(thermostat->status_on_off){
	
// Блок включил все устройства, теперь проверяем температуру 
// на превышения указаного порога отключения  
		
		if (temp >= thermostat->temperatura_off && thermostat->N_pin_off){
			
			device.pin.group = thermostat->pin_off;
			device.num_pin = thermostat->N_pin_off;
			gpio_set_command(&Stanok_gpio,DISABLED_DEVICE_GROUP,&device);
						
			thermostat->status_on_off = 0;
		};

// Блок выключил все устройства,  теперь проверяем температуру 
// на меньшие значения порога включения 		
	}else{ 
		if (temp <= thermostat->temperatura_on && thermostat->N_pin_on){
			
			device.pin.group = thermostat->pin_on;
			device.num_pin = thermostat->N_pin_on;			
			gpio_set_command(&Stanok_gpio,ENABLED_DEVICE_GROUP,&device);
			
			//for (int i=0; i < thermostat->N_pin_on; i++)
			//		thermostat->enabled_device(thermostat->pin_on[i]);
			
							
			thermostat->status_on_off = 1;
		};
	
	};
};




