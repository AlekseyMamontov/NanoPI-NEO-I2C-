
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
#include "thermo_i2c/i2cmax6675.h"

// Cчетчики готовой продукции
#include "counter_product/pth_counter.h"

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



////////////////////////////////////////////

#define CHIP1 0
#define MAX_CHIP 2

MCP23017 chip1 = { .i2_bus = "/dev/i2c-0",
				   .address = 0x20,
				   //.lock_registr_GPIO = PTHREAD_MUTEX_INITIALIZER,
				   //.lock_registr_OLAT = PTHREAD_MUTEX_INITIALIZER,
				   .lock_registr = PTHREAD_MUTEX_INITIALIZER,		   
				   };


struct nPIN_chip Stanok_chips[MAX_CHIP] ={

//mcp23017 addr 0x20	
{
 .chip = &chip1,
 .id_chip = nGPIO_CHIP1,
 .pin_functions = &mcp230xx,
},

//ttp229 addr 0x57
{
 .chip = NULL,
 .id_chip = 0,
 .pin_functions = NULL,
},
	
};

// Пины используются Термоблоками pin 8-clk,9-cs,10-data,11-data,12 -data, -13 data, входы data можно использовать 
// как высокоскростной оптовход 1мбит (H11L1)	

// Строим gpio pin  для структруры STANOK_GPIO  /////////////////////

#define MAX_PIN 16
struct nPIN Stanok_pins[MAX_PIN]={
	
//pin 0 - simistors 220v
#define RELE_LIMITER 0
{
	 .n_chip= CHIP1,
	 .bit = 0,					 
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
},
//pin 1 - simistors 220v
#define RELE_2 1
{
	 .n_chip= CHIP1,
	 .bit = 1,					
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
},
//pin 2 - simistors 220v
#define VACUUM_VALVE 2	
{
	 .n_chip= CHIP1,
	 .bit = 2,					 
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
},
//pin 3 - simistors 220v
#define PNEVMOCYLINDR 3
{
	 .n_chip= CHIP1,
	 .bit = 3,					 
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
},
//pin 4 - simistors 220v
#define TEN_MATRIX 4
{
	 .n_chip= CHIP1,
	 .bit = 4,					 
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
},
//pin 5 - simistors 220v
#define TEN_PUNCH 5
{
     .n_chip= CHIP1,
	 .bit = 5,					 
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
},
//pin6 - оптопара PC817 вход транзистор (база)	
#define SENSOR_PRODUCT 6
{
	 .n_chip= CHIP1,
	 .bit = 6,					
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_invert,
	 .active_pin = 1,
},
//pin7 - transistor -open drain
#define RELE_3 7
{
	 .n_chip= CHIP1,
	 .bit = 7,					
	 .direction = pin_OUT,	  
	 .default_value = LOW,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
},
//pin8 - оптопара PC817 вход на замыкание +5v.
#define LIMIT_SWITCH 8
{
	 .n_chip= CHIP1,
	 .bit = 8,					
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
},
//pin9 - оптопара PC817 вход транзистор (база).
#define LIMIT_SWITCH_2 9
{
	 .n_chip= CHIP1,
	 .bit = 9,				   
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_invert,
},
//pin10 - оптопара вход данных MISO c MAX6675
#define MAX6675_MISO_0 10
{
	 .n_chip= CHIP1,
	 .bit = 10,				   
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
},
//pin11 - оптопара вход c MAX6675 или можно использовать как оптовход до 1мбитс
#define MAX6675_MISO_1 11
{
	 .n_chip= CHIP1,
	 .bit = 11,				   
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
	 
},
//pin12 - оптопара вход c MAX6675 или можно использовать как оптовход до 1мбитс
#define MAX6675_MISO_2 12
{
	 .n_chip= CHIP1,
	 .bit = 12,				   
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_no_invert,
},
//pin1 - оптопара вход c MAX6675 или можно использовать как оптовход до 1мбитс
#define MAX6675_MISO_3 13
{
	 .n_chip= CHIP1,
	 .bit = 13,				   
	 .direction = pin_IN,	  
	 .default_value = pin_no_pullUP,
	 .on = HIGH, 
	 .ipol = pin_no_invert,

},
//pin14 - строб для чтения из MAX6675, сигнал CLK
#define MAX6675_CLK 14
{
	 .n_chip= CHIP1,
	 .bit = 14,					
	 .direction = pin_OUT,	  
	 .default_value = HIGH,
	 .on = HIGH, 
	 .ipol = pin_no_invert,

},
//pin15 - выбор группы микросхем MAX6675, сигнал CS
#define MAX6675_CS  15
{
	 .n_chip= CHIP1,
	 .bit = 15,					
	 .direction = pin_OUT,	  
	 .default_value = HIGH,
	 .on = HIGH, 
	 .ipol = pin_no_invert,

},

};


// Формируем окончательную структуру GPIO Cтанка

struct nGPIO Stanok_gpio = {

	.pin = Stanok_pins,
	.n_pin = MAX_PIN,

	.chips = Stanok_chips,
	.num_chip = MAX_CHIP,	

	.set = Stanok_gpio_set_pin,
	.set_async = Stanok_gpio_set_async,
	.get = Stanok_gpio_get_pin,
	.get_async = Stanok_gpio_get_async,
	.direction = Stanok_gpio_direction_pin,
	.init = Stanok_gpio_init_pin,
	//.info_pin =  Stanok_gpio_info_pin,
	
	.lock_command = PTHREAD_MUTEX_INITIALIZER,	

	// При первом старте иницилизировать nGPIO
	// это важно
	.buffer_command = {INIT_GPIO},
	.buffer_data = {0},
	.position_buffer = 0,
	.num_commands = 1,	
		
};


	 
/*
+------------------------------------------------------+
| Подключаем термопары к mcp23017, схема подключения   |
| термопары Clk 0,CS-1, MiSO 2-7 бит (до 6шт,) 		   |
+------------------------------------------------------+*/
		
struct Thermosensors MAX6675={
	
	//.gpio = &Stanok_gpio,					//cm.i2c-max6675
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
		
	
	.temperature_raw_processing = max6675_read_raw,
	.temperature_data_processing = max6675_data_to_char,
	
	
};
	
/*
+---------------------------------------------------------------------+
|				Определяем Термостат для матрицы					  |																  |
| text - stanok-txt.h 												  |
+---------------------------------------------------------------------+
*/
//struct Thermostat TEN_matrix;

struct Thermostat TEN_matrix = {
				
	.temperatura_data = &MAX6675.thermo_data[0], //Канал 1 температуры 

    .status_on_off = 0,
     
	.temperatura_on = 30,
	.temperatura_off = 36,
	
	.pin_on = {TEN_MATRIX,
			   },
	.N_pin_on = 1,
	
	.pin_off = {TEN_MATRIX,
				},
	.N_pin_off = 1,
	
	.info_block = {
			.init_block = TEN_matrix_init,	
      
			.visible_block = {&MAX6675.thermo_char_visible[0],
							  &TEN_matrix_on_visible,
							  &TEN_matrix_off_visible},
								
			.change_block = {TEN_matrix_temperature,
							 TEN_matrix_on,
							 TEN_matrix_off},							 
			.change_text = {TEN_matrix_temperature_txt,
							TEN_matrix_on_txt,
							TEN_matrix_off_txt},
			
			.new_change_txt = {MAX6675.thermo_char[0],// чтение температуры из канала 1
							   TEN_new_matrix_on_txt,
							   TEN_new_matrix_off_txt}, 
		
						
			.N_change_block = 3,
			.N_fast_change_block = 3,

			.N_change_char_text = {5,5,5},

			.Change_InfoBlock = {tft_fast_change_number,
								 tft_fast_change_number,
								 tft_fast_change_number}, // тест 
			
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
	
	.temperatura_on = 35,
	.temperatura_off = 37,
	
	.pin_on = {TEN_PUNCH},
	.N_pin_on = 1,
	
	.pin_off = {TEN_PUNCH},
	.N_pin_off = 1,
	
	
	
	
	.info_block = {
		
			.init_block = TEN_punch_init,
			
			.visible_block = {&MAX6675.thermo_char_visible[1],
							  &TEN_punch_on_visible,
							  &TEN_punch_off_visible},
				     
			.change_block = {TEN_punch_temperature,
							 TEN_punch_on,
							 TEN_punch_off},							 
			.change_text = {TEN_punch_temperature_txt,
							TEN_punch_on_txt,
							TEN_punch_off_txt},
			.new_change_txt = {MAX6675.thermo_char[1],
							   TEN_new_punch_on_txt,
							   TEN_new_punch_off_txt}, // чтение температуры из канала 0
			
			.N_change_block = 3,
			.N_fast_change_block = 3,
			
			.N_change_char_text = {5,5,5},
			
			.Change_InfoBlock = {tft_fast_change_number,
								 tft_fast_change_number,
								 tft_fast_change_number },						
	},
	.check_temperature = thermostat_check,
	.enabled_device = Stanok_enabled_device,
	.disabled_device = Stanok_disabled_device,
};

/*
+-------------------------------------------------------------------------+
|				Поток термоблоков										  |
+-------------------------------------------------------------------------+
*/

struct Thermostat_Blocks Stanok_termoblock = {
	
	.thermosensors = &MAX6675,
	.thermostat = { &TEN_matrix,
					&TEN_punch,},					
	.num_thermostat = 2,
	.status = 1,
	.pthread_thermostat_blocks = pthread_reading_thermoblock, 
};

/*
+-------------------------------------------------------------------------+
|				Создаем блок счетчика - готовой продукции  и т.д.		  |
+-------------------------------------------------------------------------+


*/
uint8_t packet_event [7]={1,1,0,0};
 
// Регистрируем счетчик
	
struct ProductCounter Counter_product ={
		
		.counter_raw = 0,
		.value_inc_the_counter = 1,
		.counter_event = 100,
		.future_counter_raw_event = 100, 
		
		.sequence_of_events = packet_event,
		.N_positions_in_the_event = 0,
		.N_sequence_of_events = 3,
		 
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
			.get_pin = Stanok_gpio_get,
			},
			
		.activation_counter_event = activation_counter_event,
		.check_event = counter_check_event,
		.counter_raw_to_char = tft_data_to_char, 
		 
		};

struct PthreadsCounter_cycle Stanok_counters ={
	
	.counter = {
		&Counter_product,
		NULL,
		NULL,
		},
	.time_pause = {
			.it_value={		
			.tv_sec = 0,     
			.tv_nsec = 50000000},
		
	.it_interval ={
			.tv_sec = 0,
			.tv_nsec = 0}
		},
		
	.pthread_block_counters = check_block_counters,	
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
						
	.get_pin = Stanok_gpio_get,					
							
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
			.get_pin = Stanok_gpio_get,			
												
		}
	};


////////////////////////////////////////////////////////////////////////////
//                    menu block
////////////////////////////////////////////////////////////////////////////


struct Menu_line start_stanok, program_process, setting_device, exit_stanok,
submenu_ten_matrix,submenu_ten_punch,exit_setup_menu,submenu_counter,submenu_programm1,
submenu_programm2,submenu_programm3,submenu_programm4,submenu_programm5,submenu_programm6,
submenu_program_user,submenu_program_nop,exit_programm_menu,submenu_programm,
exit_operation_menu,pause_transition_menu,continue_transition_menu,one_pass_transition_menu;


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
	.tft_menu_change = tft_menu_changes,
	//.tft_clean_change_text = tft_clean_change_text,

	
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
											 
		.change_text = {start_process_txt_menu,
						program_process_txt_menu,
						setting_device_txt_menu,
						exit_process_txt_menu},
				  
		.new_change_txt ={	menu_line_1_txt,
							menu_line_2_txt,
						    menu_line_3_txt,
						    menu_line_4_txt}, // чтение данных сенсоров


			.N_change_block = 4,
			.N_fast_change_block = 4,
			
			.N_change_char_text = {MAX_line_char,
								   MAX_line_char,
								   MAX_line_char,
								   MAX_line_char},	
								   
			.Change_InfoBlock = {tft_print_change_block,
								 tft_print_change_block,
								 tft_print_change_block,
								 tft_print_change_block,
								 },					   
	}	
};

/*     ------- Main menu - главное меню -----
 
 Cтарт штамповки
 Программы станка
 Настройка
 Выйти 
*/


struct Menu_line 

start_stanok = { 
	.text =  start_process_txt_menu,							  					  
	.prev =  NULL,
	.next =  &program_process,
	.exit =  NULL,
	.submenu = &exit_operation_menu,
	.activation_menu_line = Tech_process_start,//запуск операции 								 
};								
struct Menu_line 

program_process = {
	.text =  program_process_txt_menu,		
	.prev =  &start_stanok,
	.next =  &setting_device,
	.exit =  NULL,
	.submenu = &exit_programm_menu,
	.activation_menu_line = tft_menu_submenu,
};
struct Menu_line 

setting_device = { 
	.text =  setting_device_txt_menu,																				
	.prev =  &program_process,
	.next =  &exit_stanok,
	.exit =  NULL,
	.submenu = &exit_setup_menu,
	.activation_menu_line = tft_menu_submenu,
};	 
struct Menu_line 

exit_stanok = { 
	.text =  exit_process_txt_menu,								 
	.prev =  &setting_device,
	.next =  NULL,
	.exit =  NULL,
	.activation_menu_line = tft_menu_no_activation,								 
};

/* ---------Submenu Start ------------- 
 *  
 Закончить
 Остановить
 Продолжить
 Пошагово
*/


struct Menu_line 

exit_operation_menu={ 	
	.text =  exit_operation_txt_menu,																				
	.prev =  NULL,
	.next =  &pause_transition_menu,
	.exit =  &start_stanok,
	.submenu = NULL,
	.activation_menu_line = tft_menu_exit,
};
struct Menu_line 

pause_transition_menu={
	.text =  pause_transition_txt_menu,																				
	.prev =  &exit_operation_menu,
	.next =  &continue_transition_menu,
	.exit =  NULL,
	.submenu = NULL,
	.activation_menu_line = tft_menu_no_activation,
};
struct Menu_line 

continue_transition_menu={	
	.text = continue_transition_txt_menu,																				
	.prev =  &pause_transition_menu,
	.next =  &one_pass_transition_menu,
	.exit =  NULL,
	.submenu = NULL,
	.activation_menu_line = tft_menu_no_activation,
};
struct Menu_line 

one_pass_transition_menu={
	.text =  one_pass_transition_txt_menu,																				
	.prev =  &continue_transition_menu,
	.next =  NULL,
	.exit =  NULL,
	.submenu = NULL,
	.activation_menu_line = tft_menu_no_activation,
};

/*-------Submenu settings - вкладка настройки------------//

...
Т-стат матрицы
Т-стат пуансона
Счетчик изделий

*/
struct Menu_line 

exit_setup_menu={
	.text =  prev_txt_menu,							  					  
	.prev =  NULL,
	.next =  &submenu_ten_matrix,
	.exit =  &start_stanok,	
	.activation_menu_line = tft_menu_exit,							 
};
struct Menu_line 

submenu_ten_matrix={ 
	.text =  Thermostat_matrix_txt_menu,							  					  
	.prev =  &exit_setup_menu,
	.next =  &submenu_ten_punch,
	.exit =  NULL,
	.activation_menu_line = tft_menu_no_activation,								 
};
struct Menu_line 

submenu_ten_punch={
	.text =  Thermostat_punch_txt_menu,							  					  
	.prev =  &submenu_ten_matrix,
	.next =  &submenu_counter,
	.exit =  NULL,
	.activation_menu_line = tft_menu_no_activation,								 
};
struct Menu_line 

submenu_counter={ 
	.text = Counter_txt_menu,							  					  
	.prev =  &submenu_ten_punch,
	.next =  NULL,
	.exit =  NULL,
	.activation_menu_line = tft_menu_no_activation,								 
};
struct Menu_line submenu_user_menu;

//---------Submenu programm -вкладка программ станка -------//

struct Menu_line 

exit_programm_menu={ 
	.text =  prev_txt_menu,							  					  
	.prev =  NULL,
	.next =  &submenu_programm1,
	.exit =  &start_stanok,	
	.activation_menu_line = tft_menu_exit,							 
};
struct Menu_line 

submenu_programm1={ 
	.text =  programm1_txt_menu,							  					  
	.prev =  &exit_programm_menu,
	.next =  &submenu_programm2,
	.exit =  NULL,	
	.activation_menu_line = tft_menu_no_activation,							 
};
struct Menu_line 

submenu_programm2={ 
	.text =  programm2_txt_menu,							  					  
	.prev =  &submenu_programm1,
	.next =  &submenu_programm3,
	.exit =  NULL,	
	.activation_menu_line = tft_menu_no_activation,							 
};
struct Menu_line 

submenu_programm3={ 
	.text =  programm3_txt_menu,							  					  
	.prev =  &submenu_programm2,
	.next =  &submenu_programm4,
	.exit =  NULL,	
	.activation_menu_line = tft_menu_no_activation,							 
};

struct Menu_line 

submenu_programm4={ 
	.text =  programm4_txt_menu,							  					  
	.prev =  &submenu_programm3,
	.next =  &submenu_programm5,
	.exit =  NULL,	
	.activation_menu_line = tft_menu_no_activation,							 
};
struct Menu_line 

submenu_programm5={ 
	.text =  programm5_txt_menu,							  					  
	.prev =  &submenu_programm4,
	.next =  &submenu_programm6,
	.exit =  NULL,	
	.activation_menu_line = tft_menu_no_activation,							 
};
struct Menu_line 

submenu_programm6={ 
	.text =  programm6_txt_menu,							  					  
	.prev =  &submenu_programm5,
	.next =  &submenu_program_user,
	.exit =  NULL,	
	.activation_menu_line = tft_menu_no_activation,							 
};

struct Menu_line 

submenu_program_user={ 
	.text =  program_user_txt_menu,							  					  
	.prev = &submenu_programm6,
	.next =  NULL,
	.exit =  NULL,	
	.activation_menu_line = tft_menu_no_activation,							 
};

struct Menu_line 

submenu_program_nop={ 
	.text = space_txt_menu,							  					  
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
	.chip = {.i2_bus ="/dev/i2c-1",	
			 .address = 0x27,
			 .lock_registr = PTHREAD_MUTEX_INITIALIZER,
			 .registr = {0},
			 .msg[0] = {0},			 
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
		
		
		.lock_command = PTHREAD_MUTEX_INITIALIZER,	
		.buffer_command = {
			TFT_INIT,
			TFT_FAST_CLEAR,
			TFT_BUILD_INFO_BLOCK,
			TFT_UPDATE_CHANGE_BLOCK,
			},
		.buffer_data = {
			NULL,
			NULL,
			NULL,
			NULL,
			},
		.position_buffer = 0,
		.num_commands = 4,
			
		//.status_command = 0,
		.command = 255,
		
		.time = {
		.it_value={		
			.tv_sec = 0,     
			.tv_nsec = 500000},
		
		.it_interval ={
			.tv_sec = 0,
			.tv_nsec = 0}
		},
		
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




//////////////////////////////////////////////////////////////////////////////////////////                                                                                       ..
// Технологические операции и переходы													//
// test - programm																		//
//////////////////////////////////////////////////////////////////////////////////////////
/*
struct Tech_transition{

 uint8_t 				 transition_type; // Тип перехода (константа) 
 uint8_t 		 *transition_information; // Описание перехода, для вывода инфы на экран      
 
 struct Tech_pins		check_sensors; // Ожидать события на данных пинах, чтоб войти (выйти) в(из) переход
 struct Tech_pins 		device_on_off; // Включаемое и выключаемое оборудование 
 struct timespec	 transition_delay; // Время для паузы	_nanosleep, timer
 
 struct Tech_dependency *dependency; // Цепочка зависимостей 
 uint8_t	  		   N_dependency; // длина цепочки

 uint8_t 	     move_to_next_transition; // Маска событий которые долны быть выполнены в переходе
 struct Tech_transition *next_transition; // Ccылка на следующей переход

 */
 
/*****************************************************************************************
 *			Технологоческая операция - штамповка тарелка 160х200	 			 		 * 
 *****************************************************************************************/
 
 
 
struct Tech_transition
Plate_160x200_init_pause,
Plate_160x200_n1,
Plate_160x200_n2,
Plate_160x200_n3,
Plate_160x200_n4;

/* Иницилизировать тех операцию для запуска циклов переходов */

struct Tech_transition 

Plate_160x200_init ={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = NULL,	 	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin   ={PNEVMOCYLINDR,VACUUM_VALVE,RELE_LIMITER},
	.value ={LOW,LOW,LOW},
	.N_pin = 3,
	},
 .transition_delay={
   .tv_sec = 1,
   .tv_nsec = 0,	 
  },	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | TECH_PROCESS_PAUSE, 
 .next_transition = &Plate_160x200_n1,
 
};
 
/* Первый переход, включить пневмоциллиндр и дождатся его подвода к заготовке.*/
 
struct Tech_transition 

Plate_160x200_n1 ={
	
 .transition_type = DEVICE_AND_SENSORS_TRANSITION,
 .transition_information = NULL,	 	
 
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
 .next_transition = &Plate_160x200_n2,
 
};


/*
 * Второй переход, 
 * включить клапан ваккума- захват новой заготовки,
 * убрать заслонку - чтоб выпала заготовка
 * выдержать предыдущию заготовку в штампе.
 */
 
 struct Tech_transition 

Plate_160x200_n2 ={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = NULL,	 	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin   ={VACUUM_VALVE,RELE_LIMITER},
	.value ={HIGH,HIGH},
	.N_pin = 2,
	},
 .transition_delay={
   .tv_sec = 5,
   .tv_nsec = 0,	 
  },	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | TECH_PROCESS_PAUSE, 
 .next_transition = &Plate_160x200_n3,
 
};
 
/*
 * Третий переход, выключить пневмоциллиндр,
 * и дождатся его возврата.
 */
 
 struct Tech_transition 

Plate_160x200_n3 ={
	
 .transition_type = DEVICE_AND_SENSORS_TRANSITION,
 .transition_information = NULL,	 	
 
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
 .next_transition = &Plate_160x200_n4,
 
};
 
/* 
 * Четвертый переход, 
 * закрыть заслонку,отключить ваккум,
 * выдержать 1 секунду.
 *
 */
struct Tech_transition 

Plate_160x200_n4 ={
	
 .transition_type = DEVICE_AND_PAUSE_TRANSITION,
 .transition_information = NULL,	 	
 .check_sensors = 0,		
 
 .device_on_off={
	.pin   ={VACUUM_VALVE,RELE_LIMITER},
	.value ={LOW,LOW},
	.N_pin = 2,
	},
 .transition_delay={
   .tv_sec = 1,
   .tv_nsec = 0,	 
  },	 
 .dependency = NULL,
 .N_dependency = 0,	 

 .move_to_next_transition = TECH_PROCESS_DEVICE_ON_OFF | TECH_PROCESS_PAUSE, 
 .next_transition = &Plate_160x200_n1,
};

// Операция штамповки
uint16_t tarelka160x200_on[]  ={200,200};
uint16_t tarelka160x200_off[] = {205,205};


/*****************************************************************************************
 *			Технологоческий процесс 											 		 * 
 *****************************************************************************************/

struct Tech_operation Stanok_All_operation[] = {

	{// тарелка 160х200		
	.thermoblock = &Stanok_termoblock,
	.temperatura_on = tarelka160x200_on,
	.temperatura_off = tarelka160x200_off,
	.N_temp = 2,	
	.first_transition = &Plate_160x200_init,
	.exit_transition = &Plate_160x200_n1,	
	},
	
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


struct Tech_process Stanok_process={
	
	.gpio = &Stanok_gpio,
	
	.operation= Stanok_All_operation,
	.N_operation = 1,
	.current_number_operation = 0,
	.current_operation = NULL,
	
	.current_transition = NULL,
	.move_to_next_transition = 0,
	
		
};


