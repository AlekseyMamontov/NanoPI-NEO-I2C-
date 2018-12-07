
 /* 
 * Author: Aleksey Mamontov
 * mamontov1@gmail.com
 
 * 
 */

// Работа c tft экраном по схеме: i2c (400-1700 кГц) ->mcp23017->ili9486 -8bit 
#include "tft_i2c/tft_i2c.h"

// Иницилизация экрана на ili9486 -> схема подключения 8 бит.
#include "tft_i2c/ili9486.h"

// Термодатчик до 6шт. по i2c 
#include "thermo_i2c/i2c-max6675.h"

// Здесь хранится все данные для построения экрана и выводимый текст 
#include "stanok-txt.h"


/*
+--------------------------------------------------------------------+
| Иницилизация портов на микросхемах MCP23017 на плате 				 |
|	*chip, bit, modeInOut, default_value,on,ipol},					 |
|  modeInOut - in -1 ; out - 0										 |
+--------------------------------------------------------------------+*/

#define pin_IN 1
#define pin_OUT 0
#define pin_HIGH 1
#define pin_LOW 0
#define pin_pullUP 1
#define pin_no_pullUP 0


// Опеределяем микросхемы на плате и строим структуру GPIO
// выставляем направление пинов и значения на порту.
// активный сигнал (0 или 1) и т.п. 


MCP23017 chip1 = {"/dev/i2c-0",0x20};
MCP23017 chip2 = {"/dev/i2c-0",0x24};

struct MCP23017_PIN mcp_gpio[] ={
	
	// block Triac, Relay,Contaktor
	
	//pin0
	{.chip= &chip1,
	 .bit = 2,
	 .modeInOut = pin_OUT, 
	 .default_value = pin_LOW,
	 .on =  pin_HIGH, 
	 .ipol = pin_LOW
	 },
	
	{&chip1,3,pin_OUT,pin_LOW,pin_HIGH,pin_LOW}, //pin1
	{&chip1,4,pin_OUT,pin_LOW,pin_HIGH,pin_LOW}, //pin2
	{&chip1,5,pin_OUT,pin_LOW,pin_HIGH,pin_LOW}, //pin3
	{&chip1,6,pin_OUT,pin_LOW,pin_HIGH,pin_LOW}, //pin4
	{&chip1,7,pin_OUT,pin_LOW,pin_HIGH,pin_LOW}, //pin5
	
	// block sensor, limit switch
	
	{&chip1,8, pin_IN,pin_no_pullUP,pin_HIGH,pin_LOW}, //pin6
	{&chip1,9, pin_IN,pin_no_pullUP,pin_HIGH,pin_LOW}, //pin7
	{&chip1,10,pin_IN,pin_no_pullUP,pin_HIGH,pin_LOW}, //pin8
	{&chip1,11,pin_IN,pin_no_pullUP,pin_HIGH,pin_LOW}, //pin9
	{&chip1,12,pin_IN,pin_no_pullUP,pin_HIGH,pin_LOW}, //pin10
	{&chip1,13,pin_IN,pin_no_pullUP,pin_HIGH,pin_LOW}, //pin11
	{&chip1,14,pin_IN,pin_no_pullUP,pin_HIGH,pin_LOW}, //pin12
	{&chip1,15,pin_IN,pin_no_pullUP,pin_HIGH,pin_LOW}, //pin13

	};	
	
	
// Пример  определить имя в программе для пина.
	
struct MCP23017_PIN *Lampa = &mcp_gpio[0];


	

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


  
/*
+---------------------------------------------------------------+
| 		Адрес mcp23017 на шине к которой подключен	 			|
| 		экран на чипе ili9486(81) + разрешение экрана 			|
+---------------------------------------------------------------+*/

struct TFT_i2c ili9486 = {
	.chip = {.i2_bus ="/dev/i2c-0",	// cm.MC23017.h
			 .address = 0x22,
			 .msg[0] = {0}
			},
	.image_max_x = 480,
	.image_max_y = 320,
	.init_display = init_ili9486, //cm. ili9486.h
	};
		
	
/*
+------------------------------------------------------+
| Подключаем термопары к mcp23017, схема подключения   |
| термопары Clk 0,CS-1, MiSO 2-7 бит (до 6шт,) 		   |
+------------------------------------------------------+*/
		
	
struct Thermosensors MAX6675={
	
	.chip = &chip2,					//cm.i2c-max6675
	.check_thermocouple = {0},
	.N_thermal_sensors = 2,
	.chip_registr = OLATA,
	.t_correct = {0,0},
	};
			

						
// Структура блока термостатат

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

};

/*
+---------------------------------------------------------------------+
|				Определяем Термостат для матрицы					  |																  |
| text - stanok-txt.h 												  |
+---------------------------------------------------------------------+
*/

struct Thermostat TEN_matrix = {
		
	.temperatura_data = &MAX6675.thermo_data[0], //Канал 1 температуры 

	.temperatura_on = 200,
	.temperatura_off = 207,
	
	.pin_on = {&mcp_gpio[0]},
	.N_pin_on = 1,
	
	.pin_off = {&mcp_gpio[0]},
	.N_pin_off = 1,
	
	.info_block = {
			.init_block = TEN_matrix_init,	
      
			.change_block = {TEN_matrix_temperature,
							 TEN_matrix_on,
							 TEN_matrix_off},							 
			.change_text = {TEN_matrix_temperature_txt,
							TEN_matrix_on_txt,
							TEN_matrix_off_txt},
			
			.sensors_txt = {MAX6675.thermo_char[0]}, // чтение температуры из канала 1
						
			.N_change_block = 3,
			.N_fast_change_block = 1,

			.N_change_char_text = {5,5,5},

			.Change_InfoBlock = {tft_fast_change_number}, // тест 
			
			},
};

/*
+-------------------------------------------------------------------------+
|				Определяем Термостат для пуансона						  |
+-------------------------------------------------------------------------+
*/
	
struct Thermostat TEN_punch = {
		
	.temperatura_data = &MAX6675.thermo_data[1],	
	
	.temperatura_on = 200,
	.temperatura_off = 207,
	
	.pin_on = {&mcp_gpio[1]},
	.N_pin_on = 1,
	
	.pin_off = {&mcp_gpio[1]},
	.N_pin_off = 1,
	
	.info_block = {
		
			.init_block = TEN_punch_init,
				     
			.change_block = {TEN_punch_temperature,
							 TEN_punch_on,
							 TEN_punch_off},							 
			.change_text = {TEN_punch_temperature_txt,
							TEN_punch_on_txt,
							TEN_punch_off_txt},
			.sensors_txt = {MAX6675.thermo_char[1]}, // чтение температуры из канала 0
			
			.N_change_block = 3,
			.N_fast_change_block = 1,
			
			.N_change_char_text = {5,5,5},
			
			.Change_InfoBlock = {tft_fast_change_number},						
	},
};

/*
+-------------------------------------------------------------------------+
|				Создаем блок счетчика готовой продукции					  |
+-------------------------------------------------------------------------+
*/

struct ProductCounter{
	
	uint32_t counter_raw; 	//количество событий 

	uint32_t counter_event; // при такой кратности должно срабатывать событие 
							// например 1/10,1/25 и 1/1000
	struct MCP23017_PIN pin_counter;
		
	struct MCP23017_PIN *pin_on[5];  // массив содержит пины которые реагируют 
	uint8_t  N_pin_on;			     // на включение - при кратности ; 
	struct MCP23017_PIN *pin_off[5]; // массив содержит пины которые реагируют 
	uint8_t  N_pin_off;				 // на отключение

	struct TFT_info_block info_block;

	};


// Регистрируем счетчик
	
	struct ProductCounter Counter_product ={
		
		.counter_raw = 0,
		.counter_event = 100,
		
		.pin_counter = 0,
		.pin_on = {0},
		.N_pin_on = 1,
		.pin_off ={0},
		.N_pin_off = 1,
		
		.info_block = {
			
			.init_block = Counter_product_init,	
      		.change_block = {Counter_product_change},
			.change_text = {Counter_product_change_txt},
			
			.N_change_block = 1,
			.N_change_char_text = {14},
			},
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
	 .sensors_txt = {0}, // пины устройств которые мы контролируем на изменения
					
			.N_change_block = 4,
			.N_fast_change_block = 4,
			
			.N_change_char_text = {0},
			
			
	.pin_indikator = {&mcp_gpio[2],
				   &mcp_gpio[3],
				   &mcp_gpio[4],
				   &mcp_gpio[5]}, 	    
						    
	.indikator_on_color ={0x0400,0x0400,0x400,0x400},							    	
	.indikator_off_color = {0,0,0,0},		
			
	.Change_InfoBlock = {tft_fast_change_led,
						 tft_fast_change_led,
						 tft_fast_change_led,
						 tft_fast_change_led
						},	
						
							
							
	}
};

//Indikator Sensors 
//Строим структуру отображающую обьекты (порты на вход) 				   
					   
struct Indikator Indikators_sensors={
	
     .info_block = {
     
     .init_block = init_indikator_sensors,	
     
     .change_block = {  sensors_1_led,
						sensors_2_led,
						sensors_3_led,
						sensors_4_led
					  },
														 
	 .change_text = {sensors_1_led_txt,
					 sensors_2_led_txt,
					 sensors_3_led_txt,
					 sensors_4_led_txt},
			
	 .sensors_txt = {0}, // чтение температуры из канала 0


			.N_change_block = 4,
			.N_fast_change_block = 0,
			
			.N_change_char_text = {0},		
		
		}
	};


////////////////////////////////////////////////////////////////////////////
//                    menu block
////////////////////////////////////////////////////////////////////////////

struct TFT_menu{
	
	struct TFT_info_block info_block;
	};



struct TFT_menu Test_Menu={
	
	.info_block = {
		
		
			.init_block = TFT_menu_init,	
      
			.change_block = {0},							 
			.change_text = {0},
			.sensors_txt = {0}, // чтение температуры из канала 0


			.N_change_block = 0,
			.N_fast_change_block = 0,
			
			.N_change_char_text = {0},		
		
		}
	
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
			
			&Test_Menu.info_block,			
			},
		.N_info_block = 6,	
	};

///// Пример  рабочего окна, в каком можно выводить текст .....
///// подменяя указатель  на такое окно в TTF_screen_panel можно организовать 
///// несколько окон..

struct TFT_windows window1={
		.image_x0 = 0,
		.image_y0 = 0,
		.image_x1 = 200,
		.image_y1 = 320,
		.cursor_x = 0,
		.cursor_y = 0,
		.color_font = color_GREEN,
		.color_background = color_BLACK,
		.font = console8pt		
	};





