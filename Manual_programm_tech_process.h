
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
 *	0-		Технологическая операция - штамповка тарелка 160х200	 			 		 * 
 *****************************************************************************************/
 
 // Операция штамповки







struct 
ProductCounter *auto_counters [2] = {&Counter_product};
uint32_t 		auto_counter_raw [2] = {0,0};
uint32_t 		auto_counter_event[2] = {1,1};

uint16_t Temperature_mode_1_on[]  ={190,190};
uint16_t Temperature_mode_1_off[] = {200,200};

 
struct Tech_transition

Plate_160x200_init,
PLate_160x200_check_temp,
Plate_160x200_n1,
Plate_160x200_n2,
Plate_160x200_n3,
Plate_160x200_n4,
Plate_160x200_n5,
Plate_160x200_n6,
Plate_160x200_n7;

/* Иницилизировать тех операцию для запуска циклов переходов */

struct Tech_transition Plate_160x200_init ={
	
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

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | TECH_PROCESS_PAUSE, 
 .next_transition = &PLate_160x200_check_temp,
  
};
 

struct Tech_transition PLate_160x200_check_temp ={
	
 .transition_type = WAIT_TEMP_FOR_COUNTINUE,
 .transition_information = "Wait work temperature for countniue \n",	 	
 .check_sensors = 0,		
 
 .device_on_off={0},
 .transition_delay={0},	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = 0,
 .next_transition = &Plate_160x200_n1,
 
};


/* Первый переход, 
 * выждать 1 сек и 
 * включить пневмоциллиндр 
 */
 
struct Tech_transition Plate_160x200_n1 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information = "pause 1sec + Pnemvocylinder ON \n",	 	
 
 .check_sensors = {0},
 	
 .transition_delay= {0,550000000},
  		
 .device_on_off={
	.pin   ={PNEVMOCYLINDR},
	.value ={HIGH},
	.N_pin = 1,
	},
		 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE |
							TECH_PROCESS_DEVICE_ON_OFF, 
							 
 .next_transition = &Plate_160x200_n2,
 
};


/*
 * Второй переход, 
 * выдержать паузу
 * включить клапан ваккума- захват новой заготовки,
 * 
 */
 
 struct Tech_transition Plate_160x200_n2 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information = "Pause 2,5 sec + Vacuum_valve  ON \n ",	 	
 .check_sensors = 0,		
 
  .transition_delay={
   .tv_sec =  2,
   .tv_nsec = 300000000,
    },
    
 .device_on_off={
	.pin   ={VACUUM_VALVE},
	.value ={HIGH},
	.N_pin = 1,
	},
	 	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE | TECH_PROCESS_DEVICE_ON_OFF,
							 
 .next_transition = &Plate_160x200_n3,
 
};
 
/*
 * Третий переход, 
 * выдержать паузу чтоб взть заготовку
 * затем выключить пневмоциллиндр,
 * включить толкатель сброса заготовки
 */
 
 struct Tech_transition Plate_160x200_n3 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information ="pause 210mc + Pnemvocylinder OFF + limiter on \n",	 	
 
 .check_sensors = {0},
 
 .transition_delay= {
	.tv_sec =  0,
	.tv_nsec = 270000000,
	},		
		    
 .device_on_off={
	.pin   ={PNEVMOCYLINDR,RELE_LIMITER},
	.value ={LOW,HIGH},
	.N_pin = 2,
	},
 	 
  .dependency = NULL,
  .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE | TECH_PROCESS_DEVICE_ON_OFF,
							
 .next_transition = &Plate_160x200_n4,
 
};
 
/*
 * 4 переход, 
 * 
 * выдержать паузу. для возрата пневмоциллиндра,
 * выключить толкатель заготовки
 */ 
 struct Tech_transition Plate_160x200_n4 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information ="pause 400mc + limiter off \n",	 	
 
 .check_sensors = {
	.pin   ={LIMIT_SWITCH},
	.value ={LOW},
	.N_pin = 0,	// пока не проверять 
	 },
	 
 .transition_delay= {
	 .tv_sec = 0,
	 .tv_nsec = 400000000,
		    },
		    	 		
 .device_on_off={
	.pin   ={RELE_LIMITER},
	.value ={LOW},
	.N_pin = 1,
	},
	 
  .dependency = NULL,
  .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE | TECH_PROCESS_DEVICE_ON_OFF,
 .next_transition = &Plate_160x200_n5,
 
};
/* 5 переход, 
 * выдержать паузу 210 mc. для возрата толкателя
 * отключить ваккум 
*/ 

 struct Tech_transition Plate_160x200_n5 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information ="pause 210mc + vacuum off \n",	 	
 
 .check_sensors = {0},	
 
 .transition_delay= {
	 .tv_sec =  0,
	 .tv_nsec = 210000000,
	},
 	
 .device_on_off={
	.pin   ={VACUUM_VALVE},
	.value ={LOW},
	.N_pin = 1,
	},
 
  .dependency = NULL,
  .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE |
							TECH_PROCESS_DEVICE_ON_OFF,
													
 .next_transition = &Plate_160x200_n1,
 
};

/*****************************************************************************************
 *	1-		Технологическая операция - штамповка тарелка 200х140	 			 		 * 
 *****************************************************************************************/
struct Tech_transition
Plate_200x140_init,
PLate_200x140_check_temp,
Plate_200x140_n1,
Plate_200x140_n2,
Plate_200x140_n3,
Plate_200x140_n4,
Plate_200x140_n5,
Plate_200x140_n6;

/* Иницилизировать тех операцию для запуска циклов переходов */

struct Tech_transition Plate_200x140_init ={
	
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
 .next_transition = &PLate_200x140_check_temp,
 
};
 

struct Tech_transition PLate_200x140_check_temp ={
	
 .transition_type = WAIT_TEMP_FOR_COUNTINUE,
 .transition_information = "Wait work temperature for countniue \n",	 	
 .check_sensors = 0,		
 
 .device_on_off={0},
 .transition_delay={0},	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = 0, // TECH_PROCESS_WAIT_DEPENDCY, //пока не надо 
 .next_transition = &Plate_200x140_n1,
 
};

/* 
 * Первый переход, 
 * выдержать паузу 1 с
 * включить пневмоциллиндр - прижать заготовку и поднять присоски на захват заготовки.
 */
 
struct Tech_transition Plate_200x140_n1 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information = "pause 0,9 sec + Pneumocylinder ON \n",	
 
 .transition_delay= {0,900000000}, // sec,nsec		
 
 .device_on_off={ 
	 .pin   ={PNEVMOCYLINDR,PNEVMOCYLINDR_2},
	 .value ={HIGH,HIGH},
	 .N_pin = 2,
	},
 	 
  .dependency = NULL,
  .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE | TECH_PROCESS_DEVICE_ON_OFF, 
							
 .next_transition = &Plate_200x140_n2,
};

/*
 * выдержать пауз 0.5 сек
 * убрать ограничитель заготовки  RELE_LIMITER
 */

struct Tech_transition Plate_200x140_n2 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 
 .transition_information = "pause 0,5 sec + убрать ограничитель заготовки \n",	
  	
 .transition_delay= {0,500000000}, // sec,nsec		
 
 .device_on_off={ 
	 .pin   ={RELE_LIMITER},
	 .value ={LOW},
	 .N_pin = 1,
	},
 	 
  .dependency = NULL,
  .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE | TECH_PROCESS_DEVICE_ON_OFF, 
							
 .next_transition = &Plate_200x140_n3,
};

/* 
 * выдержать паузу 1 сек
 * включить клапан ваккума - захват новой заготовки,
 * убрать ограничитель заготовки
 */
 
 struct Tech_transition Plate_200x140_n3 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information = "Pause 0,8 sec  + включить ваккум \n ",	 	
		
 .transition_delay={ .tv_sec = 0, .tv_nsec = 800000000 },
 .device_on_off={
	.pin   ={VACUUM_VALVE},
	.value ={HIGH},
	.N_pin = 1,
	},
	
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE | TECH_PROCESS_DEVICE_ON_OFF, 
							 
 .next_transition = &Plate_200x140_n4,
 
};
 
/* 
 * выдержать паузу 500 мс.
 * выключить пневмоцилиндры, поднять пуансон и опустить новую заготовку,
 */
 
 struct Tech_transition Plate_200x140_n4 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information ="pause 0,8 - поднять пнвмоцилиндр и опустить заготовку \n",
 
 .transition_delay= {0,800000000},	
 
 .device_on_off={
	.pin   ={PNEVMOCYLINDR,PNEVMOCYLINDR_2},
	.value ={LOW,LOW},
	.N_pin = 2,
	},
		 
  .dependency = NULL,
  .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | TECH_PROCESS_PAUSE,							
 .next_transition = &Plate_200x140_n5,
 
};
 /*
  * выдержать паузу - время необходимое для выемки штамповки  700 мс
  * 
  * затем отключить ваккум
  */
  
 struct Tech_transition Plate_200x140_n5 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information ="pause 700 mc + вакуум  off + включить ограничитель заготовки\n",
 	 	
 
 .transition_delay= {0,700000000},
 		
 .device_on_off={
	.pin   ={VACUUM_VALVE,RELE_LIMITER},
	.value ={LOW,HIGH},
	.N_pin = 2,
	},

  .move_to_next_transition = TECH_PROCESS_PAUSE | 
							 TECH_PROCESS_DEVICE_ON_OFF, 
	 
  .dependency = NULL,
  .N_dependency = 0,	 

 .next_transition = &Plate_200x140_n1,
 };



/*****************************************************************************************
 *	2-		Технологическая операция - штамповка тарелка д200	 			 			 * 
 *****************************************************************************************/
struct Tech_transition

Plate_d200_init_pause,
PLate_d200_check_temp,
Plate_d200_n1,
Plate_d200_n2,
Plate_d200_n3,
Plate_d200_n4,
Plate_d200_n5,
Plate_d200_n6,
Plate_d200_n7;

/* Иницилизировать тех операцию для запуска циклов переходов */

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
 .next_transition = &PLate_d200_check_temp,
  
};
 

struct Tech_transition PLate_d200_check_temp ={
	
 .transition_type = WAIT_TEMP_FOR_COUNTINUE,
 .transition_information = "Wait work temperature for countniue \n",	 	
 .check_sensors = 0,		
 
 .device_on_off={0},
 .transition_delay={0},	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = 0,
 .next_transition = &Plate_d200_n1,
 
};


/* Первый переход, 
 * выждать 1 сек и 
 * включить пневмоциллиндр 
 */
 
struct Tech_transition Plate_d200_n1 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information = "pause 1sec + Pnemvocylinder ON \n",	 	
 
 .check_sensors = {0},
 	
 .transition_delay= {1,0},
  		
 .device_on_off={
	.pin   ={PNEVMOCYLINDR},
	.value ={HIGH},
	.N_pin = 1,
	},
		 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE |
							TECH_PROCESS_DEVICE_ON_OFF, 
							 
 .next_transition = &Plate_d200_n2,
 
};


/*
 * Второй переход, 
 * выдержать паузу
 * включить клапан ваккума- захват новой заготовки,
 * 
 */
 
 struct Tech_transition Plate_d200_n2 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information = "Pause 2 sec 500mc  + Vacuum_valve  ON \n ",	 	
 .check_sensors = 0,		
 
  .transition_delay={
   .tv_sec =  2,
   .tv_nsec = 500000000,
    },
    
 .device_on_off={
	.pin   ={VACUUM_VALVE},
	.value ={HIGH},
	.N_pin = 1,
	},
	 	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE |
							TECH_PROCESS_DEVICE_ON_OFF,
							 
 .next_transition = &Plate_d200_n3,
 
};
 
/*
 * Третий переход, 
 * выдержать паузу чтоб взть заготовку
 * затем выключить пневмоциллиндр,
 * включить толкатель сброса заготовки
 */
 
 struct Tech_transition Plate_d200_n3 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information ="pause 210mc + Pnemvocylinder OFF + limiter on \n",	 	
 
 .check_sensors = {0},
 
 .transition_delay= {
	.tv_sec =  0,
	.tv_nsec = 210000000,
	},		
		    
 .device_on_off={
	.pin   ={PNEVMOCYLINDR,RELE_LIMITER},
	.value ={LOW,HIGH},
	.N_pin = 2,
	},
 
	 
  .dependency = NULL,
  .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE | 
							TECH_PROCESS_DEVICE_ON_OFF,
 .next_transition = &Plate_d200_n4,
 
};
 
/*
 * 4 переход, 
 * 
 * выдержать паузу. для возрата пневмоциллиндра,
 * выключить толкатель заготовки
 */ 
 struct Tech_transition Plate_d200_n4 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information ="pause 400mc + limiter off \n",	 	
 
 .check_sensors = {
	.pin   ={LIMIT_SWITCH},
	.value ={LOW},
	.N_pin = 0,	// пока не проверять 
	 },
	 
 .transition_delay= {
	 .tv_sec = 0,
	 .tv_nsec = 400000000,
		    },
		    	 		
 .device_on_off={
	.pin   ={RELE_LIMITER},
	.value ={LOW},
	.N_pin = 1,
	},

 	 
  .dependency = NULL,
  .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE | 
							TECH_PROCESS_DEVICE_ON_OFF,
 .next_transition = &Plate_d200_n5,
 
};
/* 5 переход, 
 * выдержать паузу 210 mc. для возрата толкателя
 * отключить ваккум 
*/ 

 struct Tech_transition Plate_d200_n5 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
 .transition_information ="pause 210mc + vacuum off \n",	 	
 
 .check_sensors = {0},	
 
 .transition_delay= {
	 .tv_sec =  0,
	 .tv_nsec = 210000000,
	},
 	
 .device_on_off={
	.pin   ={VACUUM_VALVE},
	.value ={LOW},
	.N_pin = 1,
	},
 
  .dependency = NULL,
  .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_PAUSE |
							TECH_PROCESS_DEVICE_ON_OFF,
													
 .next_transition = &Plate_d200_n1,
 
};
 
 

/*****************************************************************************************
 *	3-		Технологическая операция - штамповка тарелка д370 			 				 * 
 *****************************************************************************************/
struct Tech_transition

Plate_d370_init_pause,
PLate_d370_check_temp,
Plate_d370_n1,
Plate_d370_n2,
Plate_d370_n3,
Plate_d370_n4;

struct Tech_transition Plate_d370_init ={
	
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
 .next_transition = &PLate_d370_check_temp,
 
};
struct Tech_transition PLate_d370_check_temp ={
	
 .transition_type = WAIT_TEMP_FOR_COUNTINUE,
 .transition_information = "Wait work temperature for countniue",	 	
 .check_sensors = 0,		
 
 .device_on_off={0},
 .transition_delay={0},	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = 0, // TECH_PROCESS_WAIT_DEPENDCY, //пока не надо 
 .next_transition = &Plate_d370_n1,
 
};

/* Первый переход, включить пневмоциллиндр и дождатся его подвода к заготовке.*/
 
struct Tech_transition Plate_d370_n1 ={
	
 .transition_type = DEVICE_AND_SENSORS_TRANSITION,
 .transition_information = "\nPnemvocylinder ON ",	 	
 
 .check_sensors = {
	.pin   ={LIMIT_SWITCH},
	.value ={HIGH},
	.N_pin = 0,	// пока не проверять 
	 },		
 .device_on_off={
	.pin   ={PNEVMOCYLINDR},
	.value ={HIGH},
	.N_pin = 1,
	},

 .transition_delay= 0,
	 
  .dependency = NULL,
  .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | TECH_PROCESS_SENSORS_WAIT, 
 .next_transition = &Plate_d370_n2,
 
};


/*
 * Второй переход, 
 * включить клапан ваккума- захват новой заготовки,
 * убрать заслонку - чтоб выпала заготовка
 * выдержать предыдущию заготовку в штампе.
 */
 
struct Tech_transition Plate_d370_n2 ={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = "\nVacuum_valve and rele_limiter ON \n Pause 6 sec",	 	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin   ={VACUUM_VALVE,RELE_LIMITER},
	.value ={HIGH,HIGH},
	.N_pin = 2,
	},
 .transition_delay={
   .tv_sec = 18,
   .tv_nsec = 0,	 
  },	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | TECH_PROCESS_PAUSE, 
 .next_transition = &Plate_d370_n3,
 
};
 
/*
 * Третий переход, выключить пневмоциллиндр,
 * и дождатся его возврата.
 */
 
struct Tech_transition Plate_d370_n3 ={
	
 .transition_type = DEVICE_AND_SENSORS_TRANSITION,
 .transition_information ="\nPnemvocylinder OFF",	 	
 
 .check_sensors = {
	.pin   ={LIMIT_SWITCH},
	.value ={LOW},
	.N_pin = 0,	// пока не проверять 
	 },		
 .device_on_off={
	.pin   ={PNEVMOCYLINDR},
	.value ={LOW},
	.N_pin = 1,
	},

 .transition_delay= 0,
	 
  .dependency = NULL,
  .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | TECH_PROCESS_SENSORS_WAIT, 
 .next_transition = &Plate_d370_n4,
 
};
 
/* 
 * Четвертый переход, 
 * закрыть заслонку,отключить ваккум,
 * выдержать 3 секунду.
 *
 */
struct Tech_transition Plate_d370_n4 ={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = "\nVacuum_valve and rele_limiter OFF \n Pause 1 sec",	 	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin   ={VACUUM_VALVE,RELE_LIMITER},
	.value ={LOW,LOW},
	.N_pin = 2,
	},
 .transition_delay={
   .tv_sec = 3,
   .tv_nsec = 0,	 
  },	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | TECH_PROCESS_PAUSE, 
 .next_transition = &Plate_d370_n1,
};





/*****************************************************************************************
 *	4-		Технологическая операция - штамповка тарелка д300 			 				 * 
 *****************************************************************************************/
struct Tech_transition

Plate_d300_init_pause,
PLate_d300_check_temp;
//Plate_d300_n1,
//Plate_d300_n2,
//Plate_d300_n3,
//Plate_d300_n4;

struct Tech_transition Plate_d300_init ={
	
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
 .next_transition = &PLate_d300_check_temp,
 
};
 struct Tech_transition PLate_d300_check_temp ={
	
 .transition_type = WAIT_TEMP_FOR_COUNTINUE,
 .transition_information = "Wait work temperature for countniue",	 	
 .check_sensors = 0,		
 
 .device_on_off={0},
 .transition_delay={0},	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = 0, // TECH_PROCESS_WAIT_DEPENDCY, //пока не надо 
 .next_transition = &Plate_d370_n1,
 
};




/*****************************************************************************************
 *	5-		Технологическая операция - штамповка тарелка д275 			 				 * 
 *****************************************************************************************/
struct Tech_transition
Plate_d275_init_pause,
PLate_d275_check_temp;
//Plate_d275_n1,
//Plate_d275_n2,
//Plate_d275_n3,
//Plate_d275_n4;

struct Tech_transition Plate_d275_init ={
	
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
 .next_transition = &PLate_d300_check_temp,
 
};
 struct Tech_transition PLate_d275_check_temp ={
	
 .transition_type = WAIT_TEMP_FOR_COUNTINUE,
 .transition_information = "Wait work temperature for countniue",	 	
 .check_sensors = 0,		
 
 .device_on_off={0},
 .transition_delay={0},	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = 0, // TECH_PROCESS_WAIT_DEPENDCY, //пока не надо 
 .next_transition = &Plate_d370_n1,
 
};




/*****************************************************************************************
 *	6-		Технологическая операция - штамповка ручная -управление от кнопки	 		 *		 			 * 
 *****************************************************************************************/
struct Tech_transition
Manual_mode_init_pause,
Manual_mode_check_temp,
Manual_mode_n1,
Manual_mode_n2,
Manual_mode_n3;

struct 
ProductCounter  *manual_counters [2] = {&Counter_product};
uint32_t 		manual_counter_raw [2] = {0,0};
uint32_t 		manual_counter_event[2] = {1,1};
uint16_t 		manual_temperature_mode_on[]  ={200,202};
uint16_t 		manual_temperature_mode_off[] = {205,206};



/* Иницилизировать тех операцию для запуска циклов переходов */

struct Tech_transition Manual_mode_init_pause ={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = "semi_auto init \n",	 	
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

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | TECH_PROCESS_PAUSE, 
 .next_transition = &Manual_mode_check_temp,
 
};
 

struct Tech_transition Manual_mode_check_temp ={
	
 .transition_type = WAIT_TEMP_FOR_COUNTINUE,
 .transition_information = "Wait work temperature for countniue",	 	
 .check_sensors = 0,		
 
 .device_on_off={0},
 .transition_delay={0},	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = 0, // TECH_PROCESS_WAIT_DEPENDCY, //пока не надо 
 .next_transition = &Manual_mode_n1,
 
};

// 1 переход ждем нажатия кнопки для старта

struct Tech_transition Manual_mode_n1 = {
	
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
 .next_transition = &Manual_mode_n2,
 
};



/* 2 переход, дождаться отпускания кнопки 
 * включить пневмоциллиндр*/
 
 
struct Tech_transition Manual_mode_n2 ={
	
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
 .next_transition = &Manual_mode_n3,
 
};


/*
 * 3 переход, 
 * выдержать паузу 
 * выключить пневмоциллиндр
 */
 
 struct Tech_transition Manual_mode_n3 ={
	
 .transition_type = PAUSE_AND_DEVICE_TRANSITION,
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
 .next_transition = &Manual_mode_n1,
 
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
 void *attrib;

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
	
	.first_transition = &Plate_200x140_init,
	.exit_transition = &Plate_200x140_init,	
	},

	// Полуавтоматический режим 	

	{
	.thermoblock = &Stanok_termoblock,
	.temperatura_on  = manual_temperature_mode_on,
	.temperatura_off = manual_temperature_mode_off,
	.N_temp = 1, //нагревается одна сторона

	.counters = manual_counters,
	.counter_raw =	 manual_counter_raw, 
	.counter_event = manual_counter_event, 
    .N_counter = 1,

	.first_transition = &Manual_mode_init_pause,
	.exit_transition = &Manual_mode_init_pause,	
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

