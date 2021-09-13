
 /* 
 * Author: Aleksey Mamontov
 * mamontov1@gmail.com
 
 */




//////////////////////////////////////////////////////////////////////////////////////////                                                                                       ..
// Технологические операции и переходы													//
// test - programm																		//
//////////////////////////////////////////////////////////////////////////////////////////
/*
struct Tech_transition{

 uint8_t 				 transition_type; // Тип перехода (константа) 
 unsigned char 	 *transition_information; // Описание перехода, для вывода инфы на экран      
 
 struct Tech_pins		check_sensors; // Ожидать события на данных пинах, чтоб войти (выйти) в(из) переход
 struct Tech_pins 		device_on_off; // Включаемое и выключаемое оборудование 
 struct timespec	 transition_delay; // Время для паузы	_nanosleep, timer
 
 struct Tech_dependency *dependency; // Цепочка зависимостей 
 uint8_t	  		   N_dependency; // длина цепочки

 uint8_t 	     move_to_next_transition; // Маска событий которые долны быть выполнены в переходе
 uint8_t			   temporary_counter; // временный счетчик
 
 struct Tech_transition *next_transition; // Ccылка на следующей переход
 struct Tech_transition *alternative_next_transition; // Переход в случаи не возможночсти выполнения условия
 */
 


/*****************************************************************************************
 *	6-		Технологическая операция - штамповка ручная -управление от кнопки	 		 *		 			 * 
 *****************************************************************************************/
struct Tech_transition
Semiautomat_init_pause,
Semiautomat_check_temp,
Semiautomat_n1,
Semiautomat_n2,
Semiautomat_n3;

struct 
ProductCounter *semiautomat_counters [2] = {&Counter_product};
uint32_t 		semiautomat_counter_raw [2] = {0,0};
uint32_t 		semiautomat_counter_event[2] = {1,1};
uint16_t 		semiautomat_temperature_mode_on[]  ={195,195};
uint16_t 		semiautomat_temperature_mode_off[] = {205,205};

// время удержания заготовки в штампе (sec,nsec). 
struct timespec semiautomat_pause ={7,0};
struct timespec D200_data_pause ={7,0};
struct timespec D275_data_pause ={7,0};
struct timespec D300_data_pause ={7,0};
struct timespec D370_data_pause ={7,0};



/* Иницилизировать тех операцию для запуска циклов переходов */

struct Tech_transition Semiautomat_init_pause ={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = "semiautomat init \n",	 	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin   ={PNEVMOCYLINDR},
	.value ={LOW},
	.N_pin = 1,
	},
 .transition_delay={
   .tv_sec = 1,
   .tv_nsec = 0,	 
  },	 
 .dependency = NULL,
 
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF |
							TECH_PROCESS_PAUSE, 

 .next_transition = &Semiautomat_check_temp,
};
 

struct Tech_transition Semiautomat_check_temp ={
	
 .transition_type = WAIT_TEMP_FOR_COUNTINUE,
 .transition_information = "Wait work temperature for countniue",	 	
 .check_sensors = 0,		
 
 .device_on_off={0},
 .transition_delay={0},	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = 0, // TECH_PROCESS_WAIT_DEPENDCY, //пока не надо 
 .next_transition = &Semiautomat_n1,
 
};

// 1 переход ждем нажатия кнопки для старта

struct Tech_transition Semiautomat_n1 = {
	
 .transition_type = WAIT_SENSORS_ANTI_RATTLING,
 .transition_information = "\n Wait key for start ",	 	
 
 .check_sensors ={
	.pin   ={LIMIT_SWITCH},
	.value ={HIGH},
	.N_pin = 1,
	},
 .transition_delay={
   .tv_sec = 0,
   .tv_nsec = 10000000, // 10ms
   },
	 
  .dependency = NULL,
  .N_dependency = 0,	 


 .move_to_next_transition = TECH_PROCESS_SENSORS_WAIT | TECH_PROCESS_PAUSE, 
 .temporary_counter = 3,
 .next_transition = &Semiautomat_n2,
 
};



/* 2 переход, дождаться отпускания кнопки 
 * включить пневмоциллиндр*/
 
 
struct Tech_transition Semiautomat_n2 ={
	
 .transition_type = SENSORS_AND_DEVICE_TRANSITION,
 .transition_information = "\n Wait key and Pnemvocylinder ON ",	 	
 
 .check_sensors = {
	.pin   ={LIMIT_SWITCH},
	.value ={LOW},
	.N_pin = 1,
	},
    .device_on_off={
	.pin   ={PNEVMOCYLINDR},
	.value ={HIGH},
	.N_pin = 1,
	},

 .transition_delay= 0,
	 
  .dependency = NULL,
  .N_dependency = 0,	 

 .move_to_next_transition =  TECH_PROCESS_SENSORS_WAIT | TECH_PROCESS_DEVICE_ON_OFF, 
 .next_transition = &Semiautomat_n3,
 
};


/*
 * 3 переход, 
 * выдержать паузу 
 * выключить пневмоциллиндр
 */
 
 struct Tech_transition Semiautomat_n3 ={
	
 .transition_type = DATA_PAUSE_AND_DEVICE_TRANSITION,
 .transition_information = "\nPause 15сек ,off device",	 	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin   ={PNEVMOCYLINDR},
	.value ={LOW},
	.N_pin = 1,
	},
 .transition_delay={
   .tv_sec = 7,
   .tv_nsec = 0,	 
  },	 
 .dependency = NULL,
 .N_dependency = 0,	 


 .move_to_next_transition = TECH_PROCESS_PAUSE | TECH_PROCESS_DEVICE_ON_OFF , 
 .next_transition = &Semiautomat_n1,
 
};
 
/*****************************************************************************************
 *	2-		Технологическая операция - штамповка тарелка д200	 			 			 * 
 *****************************************************************************************/


struct Tech_transition Plate_d200_init ={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = "Plate_d200_init",	 	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin =	 {PNEVMOCYLINDR,VACUUM_VALVE,RELE_LIMITER},
	.value = {0,0,0},
	.N_pin = 3,
  },
 
 .transition_delay={.tv_sec =  1, .tv_nsec = 0,},	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | 
							TECH_PROCESS_PAUSE, 
 .next_transition = &Semiautomat_check_temp,
  
};


/*****************************************************************************************
 *	3-		Технологическая операция - штамповка тарелка д275 			 				 * 
 *****************************************************************************************/

struct Tech_transition Plate_d275_init={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = "Plate_160x200_init",	
  	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin   ={PNEVMOCYLINDR,VACUUM_VALVE,RELE_LIMITER},
	.value ={0,0,0},
	.N_pin = 3,
	},
 .transition_delay={
   .tv_sec = 1,
   .tv_nsec = 0,	 
  },	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | TECH_PROCESS_PAUSE, 
 .next_transition = &Semiautomat_check_temp,
};

/*****************************************************************************************
 *	 4 -		Технологическая операция - штамповка тарелка д300	 			 			 * 
 *****************************************************************************************/
 

struct Tech_transition Plate_d300_init={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = "Plate_d200_init",	 	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin =	 {PNEVMOCYLINDR,VACUUM_VALVE,RELE_LIMITER},
	.value = {0,0,0},
	.N_pin = 3,
  },
 
 .transition_delay={.tv_sec =  1, .tv_nsec = 0,},	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | 
							TECH_PROCESS_PAUSE, 
 .next_transition = &Semiautomat_check_temp,
  
};

/*****************************************************************************************
 *	 5 -		Технологическая операция - штамповка тарелка д300	 			 			 * 
 *****************************************************************************************/


struct Tech_transition Plate_d300_init={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = "Plate_d200_init",	 	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin =	 {PNEVMOCYLINDR,VACUUM_VALVE,RELE_LIMITER},
	.value = {0,0,0},
	.N_pin = 3,
  },
 
 .transition_delay={.tv_sec =  1, .tv_nsec = 0,},	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | 
							TECH_PROCESS_PAUSE, 
 .next_transition = &Semiautomat_check_temp,
  
};



/*****************************************************************************************
 *	6-		Технологическая операция - штамповка тарелка 160х200	 			 		 * 
 *****************************************************************************************/
 
 // Операция штамповки

struct 
ProductCounter *auto_counters [2] = {&Counter_product};
uint32_t 		auto_counter_raw [2] = {0,0};
uint32_t 		auto_counter_event[2] = {1,1};

uint16_t Temperature_mode_1_on[]  ={190,190};
uint16_t Temperature_mode_1_off[] = {200,200};


/* Иницилизировать тех операцию для запуска циклов переходов */

struct Tech_transition Plate_160x200_init={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = "Plate_160x200_init",	 	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin =	 {PNEVMOCYLINDR,VACUUM_VALVE,RELE_LIMITER},
	.value = {LOW,LOW,LOW},
	.N_pin = 3,
  },
 
 .transition_delay={.tv_sec =  1, 
					.tv_nsec = 0,},	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | 
							TECH_PROCESS_PAUSE, 
							
 .next_transition = &Semiautomat_check_temp,
  
};




/*****************************************************************************************
 *	1-		Технологическая операция - штамповка тарелка 200х140	 			 		 * 
 *****************************************************************************************/


/* Иницилизировать тех операцию для запуска циклов переходов */

struct Tech_transition Plate_200x140_init={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = "Plate_200x140_init",	 	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin={PNEVMOCYLINDR, VACUUM_VALVE, RELE_LIMITER,PNEVMOCYLINDR_2,RELE_LIMITER2},
	.value ={LOW,LOW,LOW,LOW},
	.N_pin = 4,
	},
 .transition_delay={.tv_sec = 1, .tv_nsec = 0},
 	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | TECH_PROCESS_PAUSE, 
 .next_transition = &Semiautomat_check_temp,
 
};
 


/*****************************************************************************************
 			Технологоческие операции
 
 struct Tech_operation{

 // блок термостат
 struct Thermostat_Blocks  *thermoblock;
 uint16_t 			    *temperatura_on;
 uint16_t 	  		   *temperatura_off;
 uint8_t	 	 	 			 N_temp;
 
 // блок счетчиков
 struct ProductCounter **counters; // указатель на массив указателей счетчиков
 uint32_t			*counter_raw; // начальные значения на счетчиков
 uint32_t 		  *counter_event; // насколько за раз увеличивается счетчик
 uint8_t  			   N_counter; // количество счетчиков

 // разные данные для операции
 void *data;

struct Tech_transition *first_transition;
struct Tech_transition  *exit_transition;
}; 											 		 * 
 
 
 *****************************************************************************************/



 


struct Tech_operation Stanok_All_operation[] = {


	// тарелка D200 	

	{
	.thermoblock = &Stanok_termoblock,
	.temperatura_on  = Temperature_mode_1_on,
	.temperatura_off = Temperature_mode_1_off,
	.N_temp = 1, //нагревается одна сторона

	.counters = auto_counters,
	.counter_raw =	 auto_counter_raw, 
	.counter_event = auto_counter_event, 
    .N_counter = 1,	
		
	.data = &D200_data_pause,	
		
	.first_transition = &Plate_d200_init,
	.exit_transition = &Plate_d200_init,	
	},

	// тарелка D275 	

	{
	.thermoblock = &Stanok_termoblock,
	.temperatura_on  = Temperature_mode_1_on,
	.temperatura_off = Temperature_mode_1_off,
	.N_temp = 1, //нагревается одна сторона

	.counters = auto_counters,
	.counter_raw =	 auto_counter_raw, 
	.counter_event = auto_counter_event, 
    .N_counter = 1,	
    
    .data = &D275_data_pause,
	
	.first_transition = &Plate_d275_init,
	.exit_transition = &Plate_d275_init,	
	},
		
	// тарелка D300	
	
	{	 	
	.thermoblock = &Stanok_termoblock,
	.temperatura_on  = Temperature_mode_1_on,
	.temperatura_off = Temperature_mode_1_off,
	.N_temp = 2,

	.counters = auto_counters,
	.counter_raw =	 auto_counter_raw, 
	.counter_event = auto_counter_event, 
    .N_counter = 1,	
    
    .data = &D300_data_pause,
		
	.first_transition = &Plate_d300_init,
	.exit_transition = &Plate_d300_init,	
	},
		
	// тарелка d370
	
	{	
	.thermoblock = &Stanok_termoblock,
	.temperatura_on  = Temperature_mode_1_on,
	.temperatura_off = Temperature_mode_1_off,
	.N_temp = 2,//нагревается две стороны штампа	

	.counters = auto_counters,
	.counter_raw =	 auto_counter_raw, 
	.counter_event = auto_counter_event, 
    .N_counter = 1,

	.data = &D370_data_pause,

	.first_transition = &Plate_d370_init,
	.exit_transition = &Plate_d370_init,	
	},
	
	// тарелка 160х200
	
	{	 	
	.thermoblock = &Stanok_termoblock,
	.temperatura_on  = Temperature_mode_1_on,
	.temperatura_off = Temperature_mode_1_off,
	.N_temp = 2,//нагревается две стороны штампа	
	
	.counters = auto_counters,
	.counter_raw =	 auto_counter_raw, 
	.counter_event = auto_counter_event, 
    .N_counter = 1,
    
    .data = &P160x200_data_pause,
    
	
	.first_transition = &Plate_160x200_init,
	.exit_transition = &Plate_160x200_init,	
	},
	
	// тарелка 200х140	
	
	{ 	
	.thermoblock = &Stanok_termoblock,
	.temperatura_on  = Temperature_mode_1_on,
	.temperatura_off = Temperature_mode_1_off,
	.N_temp = 2,	

	.counters = auto_counters,
	.counter_raw =	 auto_counter_raw, 
	.counter_event = auto_counter_event, 
    .N_counter = 1,
	
	.first_transition = &P200x140_init,
	.exit_transition = &P200x140_init,	
	},

	// Полуавтоматический режим 	

	{
	.thermoblock = &Stanok_termoblock,
	.temperatura_on  = semiautomat_temperature_mode_on,
	.temperatura_off = semiautomat_temperature_mode_off,
	.N_temp = 1, //нагревается одна сторона

	.counters = semiautomat_counters,
	.counter_raw =	 semiautomat_counter_raw, 
	.counter_event = semiautomat_counter_event, 
    .N_counter = 1,

	.data = &semiautomat_pause,

	.first_transition = &Semiautomat_init_pause,
	.exit_transition =  &Semiautomat_init_pause,	
	},

	
	
	
};


 //submenu_programm1.text = programm_D200_txt_menu;




struct Tech_process Stanok_process={
	
	.gpio = &Stanok_gpio,
	
	.operation= Stanok_All_operation,
	.N_operation = 7,
	.current_number_operation = 0,
	.current_operation = Stanok_All_operation,
	
	.current_transition = NULL,
	.move_to_next_transition = 0,
	
	.lock_command = PTHREAD_MUTEX_INITIALIZER,
	.position_buffer= 0,
	.num_commands = 0,
	.buffer_command = 0,
	.status_work_flow = 1,	
		
};


/*
 struct Tech_process{
				
	struct nGPIO 	 *gpio;
	struct nGPIO_DATA data;	

	// Операции
	
	struct Tech_operation *operation;
	uint8_t 		  	 N_operation;
	uint8_t    		current_number_operation;
	struct Tech_operation *current_operation;
	

	// Переходы
	struct Tech_transition *current_transition;
	uint8_t		       move_to_next_transition;
	uint8_t            hide_init_on_tranistion;
				
	// Буфер команд			
	pthread_mutex_t 	lock_command;	
_Atomic	uint8_t 	 position_buffer;
_Atomic uint8_t 		num_commands;	
	uint8_t  	 buffer_command[256];
	void   	   	   *buffer_data[256];	
	uint8_t 		status_work_flow;

	// TIMER		
	timer_t 		  	  timer_tech_process; // таймер
	struct itimerspec set_pause_tech_process; // временной интервал таймера
		
	// SIGNAL
	sigset_t 		  sig_mask_tech_process;  // маска_сигналов
	struct sigaction  	   act_tech_process;  // настроить сигнал
	struct sigevent   		   signal_timer;  // то что передает сигнал в функцию;
						
	// Запустить поток, обслуживающий техпроцесс;
	pthread_t 				  pthread_id;
	void (*pthread_Tech_process)(void *);	
		
};

  
 */

