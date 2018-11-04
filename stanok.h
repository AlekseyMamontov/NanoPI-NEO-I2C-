
 /* To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 * File:   main.c
 * Author: Aleksey Mamontov
 * Created on 24 июня 2018 г., 23:05
 
 * 
 */

 
#include "tft_i2c/tft_i2c.h"
#include "tft_i2c/ili9486.h"
#include "thermo_i2c/i2c-max6675.h"

////////////////////блок термостатат////////////////////////

struct Thermostat{
	
	uint16_t temperatura_raw;
	uint16_t temperatura_on;
	uint16_t temperatura_off;
	uint16_t gpio_pin_on_off[5];
	uint8_t  n_gpio;
	uint8_t  enable;
	uint8_t  set_gpio_on;
		
	uint8_t * name_block;
	char  char_temperatura_raw[4];
	char  char_temperatura_on[4];
	char  char_temperatura_0ff[4];
	struct TFT_windows *window_thermostat; 
	uint8_t *init_block_array;
};

/////////////////Cчетчик продукции, событие по количеству //////////////


struct ProductCounter{
	
	uint32_t counter_raw;
	uint32_t counter_event; 
	
	uint8_t  gpio_pin_activate_event;
	
	uint16_t gpio_pin_on_off[5];
	uint8_t  n_gpio;
	uint8_t  enable_counter;
	uint8_t  enable_counter_event;
	uint8_t  set_gpio_on;
	
	
	uint8_t  *char_counter_name;
	char  char_counter_raw[8];
	struct TFT_windows *window_counter;
	uint8_t *init_block_array;
	};


/////////////////////// Мини консоль для вывода данных о переходах и датчиках/////////////////////////////

struct ConsoleStatusWindow{
	
	uint8_t  *char_sonsole_name;
	struct TFT_windows *window_counter;
	uint8_t *init_block_array;
		
	};

//------------------------------------------------------//
// Иницилизация портов на микросхемах MCP23017 на плате //
//	*chip, bit, modeInOut, default_value,on,ipol},
//  modeInOut - in -1 ; out - 0
//	
//														//  
//------------------------------------------------------//

#define pin_IN 1
#define pin_OUT 0
#define pin_HIGH 1
#define pin_LOW 0
#define pin_pullUP 1
#define pin_no_pullUP 0

MCP23017 chip1 = {"/dev/i2c-0",0x20};
MCP23017 chip2 = {"/dev/i2c-0",0x24};

struct MCP23017_PIN mcp_gpio[] ={
	
	// block Triac, Relay,Contaktor
	
	{&chip1,2,pin_OUT, pin_LOW, pin_HIGH, pin_LOW},//pin0
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


//    Иницилизация устройств на плате  

//------------------------------------------------//
// Адрес mcp23017 на шине к которой подключен	  //
// экран на чипе ili9486(81) + разрешение экрана  //
//------------------------------------------------//

struct TFT_i2c ili9486 = {
	.chip = {.i2_bus ="/dev/i2c-0",
			 .address = 0x22,
			 .msg[0] = {0}
			},
	.image_max_x = 480,
	.image_max_y = 320,
	.init_display = init_ili9486,		
	};

//------------------------------------------------//
// Адрес mcp23017 на шине к которой подключен	  //
// термопары									  //
//------------------------------------------------//
		
	
struct MCP23017_thermosensor MAX6675 ={
	
	.chip = &chip2,
	.check_thermocouple = {0},
	.N_thermal_sensors = 2,
	.chip_registr = OLATA,
	.t_correct = {2,0},
	};
			
	

//// Хранит все данные о панели на которую выводится информация ////// 
	
struct TFT_screen_panel TFT_screen_1 = {
		.window = &window_full,
		.tft_model = &ili9486
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








///////// структура для сборки информационной таблицы ///////////////////////

uint8_t TFT_skelet[]= {SET_font18pt,   // font 18pt
						//SET_font,0x00,0x00,
						SET_cursorX,18,
						SET_cursorY,0,   // set X_cusror 18
						SET_background,0x04,0x00, // Set background  GREEN
						0x20,0xd2,0xe5,0xec,0xef,0xe5,0xf0,0xe0,0xf2,0xf3,0xf0,0xe0, //txt Температура
						
						SET_background,0x00,0x00, 
						SET_cursorX,23, //
						SET_cursorY,2,
						SET_fontNumber,
						0x20,0x20,0x20,0x20,0x30, // значение
						SET_font18pt,0xad,0xd1,
						
						SET_cursorX,18, //
						SET_cursorY,4,  //

						0x20,0xc2,0xea,0xeb,0x20,0x3a,0x20,0x32,0x30,0x37,0xad,0xd1,//Вкл:
						 
						SET_cursorX,18,
						SET_cursorY,5,
						
						0x20,0xc2,0xfb,0xea,0xeb,0x3a, //Выкл:
						0x20,0x32,0x31,0x30,0xad,0xd1, //значение
						
			
						SET_background,0x0F8,0x00, // Set background  RED
						SET_cursorX,18,
						SET_cursorY,7,
						
						0x20,0xd2,0xe5,0xec,0xef,0xe5,0xf0,0xe0,0xf2,0xf3,0xf0,0xe0, //txt Температура
						
						SET_background,0x00,0x00, // Set background BLACK
						
						SET_cursorX,23, //
						SET_cursorY,9,
						SET_fontNumber,
						0x20,0x20,0x20,0x20,0x30,
						SET_font18pt,0xad,0xd1,
	
						
						SET_cursorX,18,
						SET_cursorY,11,
						
						0x20,0xc2,0xea,0xeb,0x20,0x3a,0x20,0x32,0x30,0x37,0xad,0xd1,//Вкл:
						 
						SET_cursorX,18,
						SET_cursorY,12,
						
						0x20,0xc2,0xfb,0xea,0xeb,0x3a,0x20,0x32,0x31,0x30,0xad,0xd1, //Выкл:
						
	
						SET_cursorX,0,   // set X_cusror 
						SET_cursorY,0,
						SET_background,0x00,0x1f, // Set background
						  
						0x20,0x20,0x20,0x20,0x20,0xC8,0xe7,0xe4,0xe5,0xeb,0xe8,0xe9,0x20,0x20,0x20,0x20, //txt  Изделий
						SET_cursorX,0,   // set X_cusror 
						SET_cursorY,2,
						SET_background,0x00,0x0, // Set background
						  
						0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x30,0x20,0xd8,0xf2,0x2e, //txt  Изделий
	
						0x00};

uint8_t TFT_change[] ={ SET_background,0x00,0x00, 
						SET_cursorX,23, //
						SET_cursorY,2,
						SET_fontNumber,
						0x20,0x20,0x32,0x30,0x37,
						
						SET_cursorX,23, //
						SET_cursorY,9,
						SET_fontNumber,
						0x20,0x20,0x20,0x20,0x30,
						SET_font18pt,0xad,0xd1,
						0,0};
	

