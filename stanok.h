
 /* 
 * Author: Aleksey Mamontov
 * mamontov1@gmail.com
 
 */
//
#include "nGPIO/stanok_gpio.h"

// Работа c tft экраном по схеме: i2c (400-1700 кГц) ->mcp23017->ili9486 -8bit 
#include "tft_i2c/tft_i2c.h"

// Иницилизация экрана на ili9486 -> схема подключения 8 бит.
#include "tft_i2c/ili9486.h"

// Термодатчик до 6шт. по i2c 
#include "thermo_i2c/i2c_max6675.h"

// Здесь хранится все данные для построения экрана и выводимый текст 
#include "stanok-txt.h"

// Структура переходов и операций cтанка
#include "stanok-tech_process.h"




/*
+--------------------------------------------------------------------+
|Определяем микросхемы на плате и строим структуру GPIO 			 |
|выставляем направление пинов и значения на порту.					 |
|активный сигнал (0 или 1) и т.п. 									 |
+--------------------------------------------------------------------+*/

#define pin_IN 1
#define pin_OUT 0
#define HIGH 1
#define LOW 0
#define pin_pullUP 1
#define pin_no_pullUP 0
#define pin_no_invert 0
#define pin_invert 1




MCP23017 chip1 = {"/dev/i2c-0",0x20};

// Пины используются Термоблоками pin 8-clk,9-cs,10-data,11-data,12 -data, -13 data, входы data можно использовать 
// как высокоскростной оптовход 1мбит (H11L1)	

// Строим gpio pin  для структруры STANOK_GPIO  /////////////////////

struct nPIN rele_TEN_matrix ={
	 .chip= &chip1,
	 .bit = 4,					 //pin 4 - simistors 220v
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};
struct nPIN rele_TEN_punch ={
     .chip= &chip1,
	 .bit = 5,					 //pin 5 - simistors 220v
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};
struct nPIN Pnevmocylindr ={
	 .chip= &chip1,
	 .bit = 3,					 //pin 3 - simistors 220v
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};
struct nPIN Vacuum_valve ={
	 .chip= &chip1,
	 .bit = 2,					 //pin 2 - simistors 220v
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};
struct nPIN rele_1 ={
	 .chip= &chip1,
	 .bit = 1,					//pin 1 - simistors 220v
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};
struct nPIN rele_2 ={
	 .chip= &chip1,
	 .bit = 0,					 //pin 0 - simistors 220v
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};
//pin6 - оптопара PC817 вход транзистор (база)		
struct nPIN Sensor_product ={
	 .chip= &chip1,
	 .bit = 6,					
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_invert,
	 .pin_functions = &mcp230xx,
};
//pin8 - оптопара PC817 вход на замыкание +5v.
struct nPIN Limit_switch ={
	 .chip= &chip1,
	 .bit = 8,					
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};
//pin9 - оптопара PC817 вход транзистор (база).
struct nPIN Limit_switch_2 ={
	 .chip= &chip1,
	 .bit = 9,				   
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_invert,
	 .pin_functions = &mcp230xx,
};
//pin7 - transistor -open drain
struct nPIN rele_3 ={
	 .chip= &chip1,
	 .bit = 7,					
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};
//pin14 - строб для чтения из MAX6675, сигнал CLK
struct nPIN max6675_clk ={
	 .chip= &chip1,
	 .bit = 14,					
	 .direction = pin_OUT,	  
	 .default_value = HIGH,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};
//pin15 - выбор группы микросхем MAX6675, сигнал CS
struct nPIN max6675_cs ={
	 .chip= &chip1,
	 .bit = 15,					
	 .direction = pin_OUT,	  
	 .default_value = HIGH,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};
//pin10 - оптопара вход данных MISO c MAX6675
struct nPIN max6675_miso0 ={
	 .chip= &chip1,
	 .bit = 10,				   
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};

//pin11 - оптопара вход c MAX6675 или можно использовать как оптовход до 1мбитс
struct nPIN max6675_miso1 ={
	 .chip= &chip1,
	 .bit = 11,				   //pin11 - оптопара вход c MAX6675
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};

//pin12 - оптопара вход c MAX6675 или можно использовать как оптовход до 1мбитс
struct nPIN max6675_miso2 ={
	 .chip= &chip1,
	 .bit = 12,				   
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};
//pin13 - оптопара вход c MAX6675 или можно использовать как оптовход до 1мбитс
struct nPIN max6675_miso3 ={
	 .chip= &chip1,
	 .bit = 13,				   
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 .pin_functions = &mcp230xx,
};


// Формируем окончательную структуру GPIO Cтанка

struct nGPIO Stanok_gpio = {
	
	.pin ={
		&rele_TEN_matrix, //pin 0
		&rele_TEN_punch,  //pin 1
		&Pnevmocylindr,   //pin 2  
		&Vacuum_valve,
		&rele_1,
		&rele_2,
		&rele_3,
		&Sensor_product,
		&Limit_switch,
		&Limit_switch_2,   //pin 9

		&max6675_clk,      // pin10
		&max6675_cs,       // pin11
		&max6675_miso0,
		&max6675_miso1,
		&max6675_miso2,
		&max6675_miso3,
		
		NULL,
	},
	
	.set = Stanok_gpio_set_pin,
	.get = Stanok_gpio_get_pin,
	.get_async = Stanok_gpio_get_async,
	.direction = Stanok_gpio_direction_pin,
	.init = Stanok_gpio_init_pin,
	.info_pin =  Stanok_gpio_info_pin,
		
};


#define TEN_MATRIX 0
#define TEN_PUNCH 1	
#define PNEVMOCYLINDR 2 
#define VACUUM_VALVE 3

#define SENSOR_PRODUCT  7
#define LIMIT_SWITCH 	8
#define LIMIT_SWITCH_2  9

#define RELE_1 4
#define RELE_2 5
#define RELE_3 9 

#define MAX6675_CLK 10
#define MAX6675_CS  11
#define MAX6675_MISO_0 12
#define MAX6675_MISO_1 13
#define MAX6675_MISO_2 14
#define MAX6675_MISO_3 15


	 
/*
+------------------------------------------------------+
| Подключаем термопары к mcp23017, схема подключения   |
| термопары Clk 0,CS-1, MiSO 2-7 бит (до 6шт,) 		   |
+------------------------------------------------------+*/
		
struct Thermosensors MAX6675={
	
	.gpio = &Stanok_gpio,					//cm.i2c-max6675
	.check_thermocouple = {0},
	.N_thermal_sensors = 2,

	.pin_clk = MAX6675_CLK,				// номер бит в который выбран в этом порту CLK
	.pin_cs =  MAX6675_CS,
	.pin_miso = {MAX6675_MISO_0,
				 MAX6675_MISO_1,
				 MAX6675_MISO_2,
				 MAX6675_MISO_3},
	.t_correct = {0,0},
	
	.time = {
		.it_value={		
			.tv_sec = 0,     // Время опроса примерно 500 милисекунд, достаточно ))
			.tv_nsec = 500000000},
		
		.it_interval ={
			.tv_sec = 0,
			.tv_nsec = 0}
	},
	.time_cycle = 1,  
	
	.timer_evp = {
	 .sigev_notify = SIGEV_SIGNAL,
     .sigev_signo = SIGUSR1,
     .sigev_value = {.sival_ptr = &MAX6675.timer},
	},
	
	
	
	.init_timer = thermosensors_init_timer,
	.temperature_reading = thermosensors_temperature_reading,
	.temperature_raw_processing = max6675_read_raw,
	.temperature_data_processing = max6675_data_to_char,
	.pthread_reading_temperature = pthread_reading_thermosensors,
	.timer_handler = max6675_timer_cycle,
	
	};
	
 			
					
/* Структура блока термостатат

struct Thermostat{

struct Thermosensors *sensors;

	int16_t  *temperatura_data;

	uint16_t  temperatura_on;  // Температура включения
	uint16_t temperatura_off;  // Температура выключения 

	struct MCP23017_PIN *pin_on[5];  // массив содержит пины которые реагируют 
	uint8_t  N_pin_on;			     // на температуру включение - HIGH; 
	struct MCP23017_PIN *pin_off[5]; // массив содержит пины которые реагируют 
	uint8_t  N_pin_off;				 // на температуру отключение - LOW; 

	struct TFT_info_block info_block;

void (*check_temperature)(struct Thermostat *); 

};


+---------------------------------------------------------------------+
|				Определяем Термостат для матрицы					  |																  |
| text - stanok-txt.h 												  |
+---------------------------------------------------------------------+
*/

struct Thermostat TEN_matrix = {
		
	.sensors = &MAX6675,
		
	.temperatura_data = &MAX6675.thermo_data[0], //Канал 1 температуры 

    .status_on_off = 0,
     
	.temperatura_on = 30,
	.temperatura_off = 35,
	
	.pin_on = {TEN_MATRIX},
	.N_pin_on = 1,
	
	.pin_off = {TEN_MATRIX},
	.N_pin_off = 1,
	
	.info_block = {
			.init_block = TEN_matrix_init,	
      
			.visible_block = {&MAX6675.thermo_char_visible[0]},
      
			.change_block = {TEN_matrix_temperature,
							 TEN_matrix_on,
							 TEN_matrix_off},							 
			.change_text = {TEN_matrix_temperature_txt,
							TEN_matrix_on_txt,
							TEN_matrix_off_txt},
			
			.new_change_txt = {MAX6675.thermo_char[0]}, // чтение температуры из канала 1
						
			.N_change_block = 3,
			.N_fast_change_block = 1,

			.N_change_char_text = {5,5,5},

			.Change_InfoBlock = {tft_fast_change_number}, // тест 
			
			},
			
	.check_temperature = thermostat_check,
	.enabled_device = Stanok_enabled_device,
	.disabled_device = Stanok_disabled_device,
};

/*
+-------------------------------------------------------------------------+
|				Определяем Термостат для пуансона						  |
+-------------------------------------------------------------------------+
*/
	
struct Thermostat TEN_punch = {
		
	.temperatura_data = &MAX6675.thermo_data[1],//Канал 2 температуры 	
	
	.status_on_off = 0,
	
	.temperatura_on = 200,
	.temperatura_off = 207,
	
	.pin_on = {TEN_PUNCH},
	.N_pin_on = 1,
	
	.pin_off = {TEN_PUNCH},
	.N_pin_off = 1,
	
	.info_block = {
		
			.init_block = TEN_punch_init,
			
			.visible_block = {&MAX6675.thermo_char_visible[1]},
				     
			.change_block = {TEN_punch_temperature,
							 TEN_punch_on,
							 TEN_punch_off},							 
			.change_text = {TEN_punch_temperature_txt,
							TEN_punch_on_txt,
							TEN_punch_off_txt},
			.new_change_txt = {MAX6675.thermo_char[1]}, // чтение температуры из канала 0
			
			.N_change_block = 3,
			.N_fast_change_block = 1,
			
			.N_change_char_text = {5,5,5},
			
			.Change_InfoBlock = {tft_fast_change_number},						
	},
	.check_temperature = thermostat_check,
	.enabled_device = Stanok_enabled_device,
	.disabled_device = Stanok_disabled_device,
};

/*
+-------------------------------------------------------------------------+
|				Создаем блок счетчика готовой продукции					  |
+-------------------------------------------------------------------------+
*/

struct ProductCounter{
	
// количество событий 		
	uint32_t counter_raw; 				
// значение на которое увеличвается счетчик
	uint32_t value_inc_the_counter;		
// при такой кратности должно срабатывать событие, например 1/10,1/25 и 1/1000
	uint32_t counter_event; 			
// будущие событие									
	uint32_t future_counter_raw_event;									
	
	uint8_t pin_counter;   // изменение на этом пине  увеличивает счетчик 	
	
	uint8_t *sequence_of_events;        // массив цепочки событий	(010) - например _/\_
	uint8_t N_positions_in_the_event;   // номер позиции в очереди
	uint8_t N_sequence_of_events;	    // длина массива цепочки событий 

		
	uint8_t pin_on[5];  // массив содержит пины которые реагируют 
	uint8_t  N_pin_on;			     // включением - при кратности ; 
	uint8_t pin_off[5]; // массив содержит пины которые реагируют 
	uint8_t  N_pin_off;				 // отключением - наступлении события

	struct TFT_info_block info_block;

	void (*check_event)(struct ProductCounter *);
	void (*counter_raw_to_char)(struct TFT_info_block *,uint8_t,uint32_t,uint32_t); 
	void (*activation_counter_event)(struct ProductCounter *);
	};

// проверить наступило ли событие для счетчика 

void counter_check_event(struct ProductCounter *counter){
	
 uint8_t verify_bit = counter->info_block.get_pin(counter->pin_counter); 
	
		
  if (verify_bit == counter->sequence_of_events[counter->N_positions_in_the_event]) return; // данные значение не изменилось выходим
  
  if(++(counter->N_positions_in_the_event) < counter->N_sequence_of_events) {//printf("verify_bit %d ; counter->N_positions_in_the_event %d \n",verify_bit,counter->N_positions_in_the_event);
	return;} // часть события произошла, выходим цепочка еще не закончилась, 

	
	 
	counter->N_positions_in_the_event = 0; 										// цепочка завершилась.						
	counter->counter_raw += counter->value_inc_the_counter;	//printf("countercounter_raw %d \n",counter->counter_raw);// увеличить счетчик	
	counter->counter_raw_to_char(&counter->info_block,0,10,counter->counter_raw); // преобразовать число в текст.
			 
			   if (counter->counter_event != 0  && counter->counter_raw == counter->future_counter_raw_event){ //если отслеживаем кратность
				   counter->future_counter_raw_event += (counter->counter_event);							   // запускаем реакцию на событие
				   counter->activation_counter_event(counter); 
				}; 					 		  	
};



uint8_t packet_event [4]={0,1,0};
void activation_counter_event(){};


 
// Регистрируем счетчик
	
	struct ProductCounter Counter_product ={
		
		.counter_raw = 0,
		.value_inc_the_counter = 1,
		.counter_event = 100,
		.future_counter_raw_event = 100, 
		
		.sequence_of_events = packet_event,
		.N_positions_in_the_event = 0,
		.N_sequence_of_events = 2,
		 
		.pin_counter = SENSOR_PRODUCT,		
		.pin_on = {0},
		.N_pin_on = 0,
		.pin_off ={0},
		.N_pin_off = 0,
		
		.info_block = {
			
			.init_block = Counter_product_init,	

			.visible_block = {&Counter_product_visible},
			
      		.change_block =   {Counter_product_change},
			.change_text =	  {Counter_product_change_txt},
			.new_change_txt = {Counter_product_new_change_txt},
			
			.N_change_block = 1,
			.N_fast_change_block = 1,
			
			.N_change_char_text = {10},
			.Change_InfoBlock = {tft_fast_change_number},
			.get_pin = Stanok_gpio_get_async,
			},
			
		.activation_counter_event = activation_counter_event,
		.check_event = counter_check_event,
		.counter_raw_to_char = tft_data_to_char, 
		 
		};


//////////////////////////////////////////////////////////////////////////////
//	Индикатор включения и выключения устройств, значения входных датчиков   //
//////////////////////////////////////////////////////////////////////////////


struct Indikator{	
	    
     	struct TFT_info_block info_block;
};

// Определение структуры индикатора включаемых устройств

struct Indikator Indikators_devices={
		
 .info_block = {   
     .init_block = init_indikator_devices,	
     
     .visible_block = {&TEN_punch_led_visible,
					   &TEN_matrix_led_visible,
					   &pneumatic_cylinder1_visible,
					   &pneumatic_klapan1_led_visible},
     
     .change_block = {TEN_punch_led,
					  TEN_matrix_led,
					  pneumatic_cylinder1_led,
					  pneumatic_klapan1_led
					  },
	.change_text = {TEN_punch_led_txt,
					TEN_matrix_led_txt,
					pneumatic_cylinder1_led_txt,
					pneumatic_klapan1_led_txt
					},			
	 .new_change_txt = {0}, 
					
			.N_change_block = 4,
			.N_fast_change_block = 4,
			
			.N_change_char_text = {0},
			
 // пины устройств которые мы контролируем на изменения			
		.pin = {TEN_PUNCH,
				TEN_MATRIX,
				PNEVMOCYLINDR,
				VACUUM_VALVE}, 	    
						    
	.indikator_on_color ={0x0400,0x0400,0x400,0x400},							    	
	.indikator_off_color = {0,0,0,0},		
			
	.Change_InfoBlock = {tft_fast_change_led,
						 tft_fast_change_led,
						 tft_fast_change_led,
						 tft_fast_change_led
						},	
						
	.get_pin = Stanok_gpio_get_async,					
							
	}
};

//Indikator Sensors 
//Строим структуру отображающую обьекты (порты на вход) 				   
					   
struct Indikator Indikators_sensors={
	
     .info_block = {
     
     .init_block = init_indikator_sensors,
     
     .visible_block = {&sensors_1_led_visible,
					   &sensors_2_led_visible,
					   &sensors_3_led_visible},	
     
     .change_block ={ sensors_1_led,
					  sensors_2_led,
					  sensors_3_led,
					  },
														 
	 .change_text = {sensors_1_led_txt,
					 sensors_2_led_txt,
					 sensors_3_led_txt,					 
					 },
			
	 .new_change_txt = {0}, 
						
			.N_change_block = 3,
			.N_fast_change_block = 3,
			
			.N_change_char_text = {0},
			
// пины устройств которые мы контролируем

		.pin = {SENSOR_PRODUCT,
			    LIMIT_SWITCH,
			    LIMIT_SWITCH_2}, 
			
		.indikator_on_color ={0xfc00,
							  0xfc00,
							  0xfc00
							  },							    	
		.indikator_off_color = {0,0,0,0},
							
			.Change_InfoBlock = {
						 tft_fast_change_led,
						 tft_fast_change_led,
						 tft_fast_change_led,
						},		
			.get_pin = Stanok_gpio_get_async,			
												
		}
	};


////////////////////////////////////////////////////////////////////////////
//                    menu block
////////////////////////////////////////////////////////////////////////////


struct Menu_line start_stanok, program_process, setting_device, exit_stanok,
submenu_ten_matrix,submenu_ten_punch,exit_setup_menu,submenu_counter,submenu_programm1,
submenu_programm2,submenu_programm3,submenu_programm4,submenu_programm5,submenu_program_user,
submenu_program_nop,exit_programm_menu,submenu_programm6;


struct TFT_menu Stanok_Menu={

	.selected_line = &start_stanok,
	
	.n_selected_menu_line = 0,
	
	.visible_selection_on ={ menu_line_1_on,
							 menu_line_2_on,
							 menu_line_3_on,
							 menu_line_4_on
							},
	.visible_selection_off={menu_line_1_off,
							menu_line_2_off,
							menu_line_3_off,
							menu_line_4_off
							},							
	
	.tft_menu_up = tft_menu_up,
	.tft_menu_down = tft_menu_down,
	.tft_menu_enter = tft_menu_enter,
	.tft_menu_change = tft_menu_change,
	.tft_clean_change_text = tft_clean_change_text,

	
	.info_block = {
		
		
		.init_block = TFT_menu_init,
		
		.visible_block ={&menu_line_1_visible,
						 &menu_line_2_visible,
						 &menu_line_3_visible,
						 &menu_line_4_visible},	
      
		.change_block = {menu_line_1_on,
						 menu_line_2_off,
						 menu_line_3_off,
						 menu_line_4_off},
											 
		.change_text = {menu_line_1_txt,
						menu_line_2_txt,
						menu_line_3_txt,
						menu_line_4_txt},
				  
		.new_change_txt ={	start_process_txt_menu,
							program_process_txt_menu,
						    setting_device_txt_menu,
						    exit_process_txt_menu}, // чтение данных сенсоров


			.N_change_block = 4,
			.N_fast_change_block = 4,
			
			.N_change_char_text = {MAX_line_char,
								   MAX_line_char,
								   MAX_line_char,
								   MAX_line_char},	
								   
			.Change_InfoBlock = {tft_fast_change_number,
								 tft_fast_change_number,
								 tft_fast_change_number,
								 tft_fast_change_number,
								 },					   
	}	
};

//------------------------- Main menu - главное меню ------//

struct Menu_line start_stanok = { .text =  start_process_txt_menu,							  					  
								  .prev =  NULL,
								  .next =  &program_process,
								  .exit =  NULL,
								  .activation_menu_line = tft_menu_no_activation,								 
};
struct Menu_line program_process = {.text =  program_process_txt_menu,		
								 .prev =  &start_stanok,
								 .next =  &setting_device,
								 .exit =  NULL,
								 .submenu = &exit_programm_menu,
								 .activation_menu_line = tft_menu_submenu,
};
struct Menu_line setting_device = { .text =  setting_device_txt_menu,																				
								 .prev =  &program_process,
								 .next =  &exit_stanok,
								 .exit =  NULL,
								 .submenu = &exit_setup_menu,
								 .activation_menu_line = tft_menu_submenu,
};	 
struct Menu_line exit_stanok = { .text =  exit_process_txt_menu,								 
								 .prev =  &setting_device,
								 .next =  NULL,
								 .exit =  NULL,
								 .activation_menu_line = tft_menu_no_activation,								 
};
//----------------------------------Submenu Start ----------------------------------------//
struct Menu_line exit_start_menu;
struct Menu_line submenu_pause;
struct Menu_line submenu_continue;
struct Menu_line submenu_one_pass;

//----------------------------Submenu settings - вкладка настройки------------------------//

struct Menu_line exit_setup_menu={ .text =  prev_txt_menu,							  					  
								  .prev =  NULL,
								  .next =  &submenu_ten_matrix,
								  .exit =  &start_stanok,	
								  .activation_menu_line = tft_menu_exit,							 
};
struct Menu_line submenu_ten_matrix={ .text =  Thermostat_matrix_txt_menu,							  					  
								  .prev =  &exit_setup_menu,
								  .next =  &submenu_ten_punch,
								  .exit =  NULL,
								  .activation_menu_line = tft_menu_no_activation,								 
};
struct Menu_line submenu_ten_punch={ .text =  Thermostat_punch_txt_menu,							  					  
								  .prev =  &submenu_ten_matrix,
								  .next =  &submenu_counter,
								  .exit =  NULL,
								  .activation_menu_line = tft_menu_no_activation,								 
};
struct Menu_line submenu_counter={ .text = Counter_txt_menu,							  					  
								  .prev =  &submenu_ten_punch,
								  .next =  NULL,
								  .exit =  NULL,
								  .activation_menu_line = tft_menu_no_activation,								 
};
struct Menu_line submenu_user_menu;

//----------------------------Submenu programm -вкладка программ станка ------------------//

struct Menu_line exit_programm_menu={ .text =  prev_txt_menu,							  					  
								  .prev =  NULL,
								  .next =  &submenu_programm1,
								  .exit =  &start_stanok,	
								  .activation_menu_line = tft_menu_exit,							 
};
struct Menu_line submenu_programm1={ .text =  programm1_txt_menu,							  					  
								  .prev =  &exit_programm_menu,
								  .next =  &submenu_programm2,
								  .exit =  NULL,	
								  .activation_menu_line = tft_menu_no_activation,							 
};
struct Menu_line submenu_programm2={ .text =  programm2_txt_menu,							  					  
								  .prev =  &submenu_programm1,
								  .next =  &submenu_programm3,
								  .exit =  NULL,	
								  .activation_menu_line = tft_menu_no_activation,							 
};
struct Menu_line submenu_programm3={ .text =  programm3_txt_menu,							  					  
								  .prev =  &submenu_programm2,
								  .next =  &submenu_programm4,
								  .exit =  NULL,	
								  .activation_menu_line = tft_menu_no_activation,							 
};

struct Menu_line submenu_programm4={ .text =  programm4_txt_menu,							  					  
								  .prev =  &submenu_programm3,
								  .next =  &submenu_programm5,
								  .exit =  NULL,	
								  .activation_menu_line = tft_menu_no_activation,							 
};
struct Menu_line submenu_programm5={ .text =  programm5_txt_menu,							  					  
								  .prev =  &submenu_programm4,
								  .next =  &submenu_programm6,
								  .exit =  NULL,	
								  .activation_menu_line = tft_menu_no_activation,							 
};
struct Menu_line submenu_programm6={ .text =  programm6_txt_menu,							  					  
								  .prev =  &submenu_programm5,
								  .next =  &submenu_program_user,
								  .exit =  NULL,	
								  .activation_menu_line = tft_menu_no_activation,							 
};

struct Menu_line submenu_program_user={ .text =  program_user_txt_menu,							  					  
								  .prev = &submenu_programm6,
								  .next =  NULL,
								  .exit =  NULL,	
								  .activation_menu_line = tft_menu_no_activation,							 
};

struct Menu_line submenu_program_nop={ .text = space_txt_menu,							  					  
								  .prev =  &submenu_program_user,
								  .next =  NULL,
								  .exit =  NULL,	
								  .activation_menu_line = tft_menu_no_activation,							 
};

//----------------------Submenu Exit - вкладка выхода или рестарта программы --------------//

struct Menu_line exit_start_menu;
struct Menu_line submenu_exit_programm;
struct Menu_line submenu_restart_stanok;
struct Menu_line sub;
//-----------------------------------------------------------------------------------------//





/*
+---------------------------------------------------------------+
| 		Адрес mcp23017 на шине к которой подключен	 			|
| 		экран на чипе ili9486(81) + разрешение экрана 			|
+---------------------------------------------------------------+*/

struct TFT_i2c ili9486 = {
	.chip = {.i2_bus ="/dev/i2c-1",	// cm.MC23017.h
			 .address = 0x27,
			 .msg[0] = {0}
			},
	.image_max_x = 480,
	.image_max_y = 320,
	.init_display = init_ili9486, //cm. ili9486.h
	};


///////// Определяем главное окно для вывода на весь экран //////////
	
struct TFT_windows window_full ={
		.image_x0 = 0,
		.image_y0 = 0,
		.image_x1 = 480,
		.image_y1 = 320,
		.cursor_x = 0,
		.cursor_y = 0,
		.color_font = color_WHITE,
		.color_background = color_BLACK,
		.font = console8pt	
	};	

		
///////////// Хранит все данные о панели на которую выводится информация ////// 
	
struct TFT_screen_panel TFT_screen_1 = {
		.window = &window_full,
		.tft_model = &ili9486,
		
		.info_block ={
			&TEN_matrix.info_block,
			&TEN_punch.info_block,

			&Counter_product.info_block,
			
			&Indikators_devices.info_block,
			&Indikators_sensors.info_block,
			
			
			&Stanok_Menu.info_block,			
			},
		.N_info_block = 6,	
	};




////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                        ..
// Технологические операции и переходы
// test - programm
///////////////////////////////////////////////////////////////////////////////////////////
/*
 * struct Tech_transition{

	
 	struct MCP23017_PIN *pin_on[MAX_tech_pin];  
	struct MCP23017_PIN *pin_off[MAX_tech_pin];
	struct MCP23017_PIN *pin_waiting_event[MAX_tech_pin]; 
		 
	uint8_t  N_pin_on; 		     
	uint8_t  N_pin_off;	 
	uint8_t  N_pin_waiting;
	
	uint8_t  hide_initial_operations;
	 
    long transition_delay; 

	uint8_t move_to_next_transition;  

	struct Tech_transition *next;
				
	void (*activation_transition)(struct Tech_transition *);
	void (*pin_waiting)(struct Tech_transition *);
};
 * 
 */
 
 struct Tech_transition first_transition,two_transition,three_transition;
 

struct Tech_transition Init_transition={
	
	.pin_off = {PNEVMOCYLINDR,
				VACUUM_VALVE,
				RELE_1},
				
	.N_pin_on = 0,
	.N_pin_off = 3,
	.N_pin_waiting = 0,
	.hide_initial = 0,
	.transition_delay = 0,
	.move_to_next_transition = 0,
	
	.next = &first_transition,
	
	.pin_waiting = pin_waiting_transition,
	.activation_transition = inital_transition,
};



struct Tech_transition first_transition={
	
	.pin_on =  {PNEVMOCYLINDR},
	.N_pin_on = 1,
	.N_pin_off = 0,
	.N_pin_waiting = 0,
	.hide_initial = 0,
	.transition_delay = 10,
	.move_to_next_transition = 0,
	
	.next = &two_transition,
	
	.pin_waiting = pin_waiting_transition,
	.activation_transition = inital_transition,
};

struct Tech_transition two_transition={
	
	.pin_on =  {1,2,},
	.pin_off =  {0},
	
	.N_pin_on = 2,
	.N_pin_off = 1,
	.N_pin_waiting = 0,
	.hide_initial = 0,
	.transition_delay = 7,
	.move_to_next_transition = 0,
	
	.next = &three_transition,
	
	.pin_waiting = pin_waiting_transition,
	.activation_transition = inital_transition,
};


struct Tech_transition three_transition={
	
	.pin_off =  {1,2,},
			
	.pin_on =  {0},
	
	.pin_waiting_event = {6},
	
	.N_pin_on = 0,	
	.N_pin_off = 2,
	.N_pin_waiting = 1,
	.hide_initial = 0,

	.transition_delay = 0,

	.move_to_next_transition = 0,
	.next = &first_transition,
	
	.pin_waiting = pin_waiting_transition,
	.activation_transition = inital_transition,
};












// Операции

struct Tech_operation Test_operation={
	
	.first_transition = &Init_transition,
	.current_transition = &Init_transition,
	.launch_transition = launch_transition,	
		
	};

// Обработка таймера

void timer_operations(){
	Test_operation.current_transition->move_to_next_transition = 1;
};















