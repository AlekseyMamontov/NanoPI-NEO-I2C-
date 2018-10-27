
 /* To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 * File:   main.c
 * Author: Aleksey Mamontov
 * Created on 24 июня 2018 г., 23:05
 * 
 * * i2c -> mcp23017 ->ili9486
 * 
 * OLATA.0  lcd_d0
 * OLATA.1  lcd_d1
 * OLATA.2  lcd_d2
 * OLATA.3  lcd_d3
 * OLATA.4  lcd_d4
 * OLATA.5  lcd_d5
 * OLATA.6  lcd_d6
 * OLATA.7  lcd_d7
 * 
 *  
 * OLATB.0  WR 	-строб на передач данных в дисплей 1-0-1 
 * OLATB.1  CS 	-выбор чипа экрана 0 , выкл. 1
 * OLATB.2  RS 	-выбор комманда 0 или данные 1
 * OLATB.3  RST -сброс экрана 1-0-1
 * OLATB.4 	RD 	-строб на чтение из экрана данных
 * OlATB.5  free
 * OLATB.6  free
 * OLATB.6  free
 * 
 */

 
#include "tft_i2c.h"





struct TFT_i2c ili9486 = {
	.image_max_x = 479,
	.image_max_y = 319,
	.chip = {.i2_bus ="/dev/i2c-0",
			 .address = 0x22,
			 .msg[0] = {0}
			}
	};
	
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
	
	
struct TFT_screen_panel TFT_screen_1 = {
		.window = &window_full,
		.tft_model = &ili9486
	};



void disp();

//////-------------------------------------------------------------------



int main(int argc, char** argv) {

MCP23017 *chip_tft = &TFT_screen_1.tft_model->chip;





mcp23017_write_byte (chip_tft, IODIRA,0xff);
mcp23017_write_byte (chip_tft, IODIRB,0xff);


mcp23017_write_byte (chip_tft, OLATA, 0x00);
mcp23017_write_byte (chip_tft, OLATB, 0xff);

mcp23017_write_byte (chip_tft, IODIRA,0);
mcp23017_write_byte (chip_tft, IODIRB,0);


//mcp23017_write_byte (chip_tft, OLATB, 0b00011111); // сброс 
//mcp23017_write_byte (chip_tft, OLATB, 0b00010111);
//mcp23017_write_byte (chip_tft, OLATB, 0b00011111);
sleep(1);											// ЭТА Звдержка Обязательна или будут артефакты
mcp23017_write_byte (chip_tft, OLATB, 0b00011101); //включить экран CS = 0
mcp23017_write_byte (chip_tft, IOCON, 0b00100000);


if (mcp23017_open_i2c (chip_tft) < 0)  { perror("I2C erorr open() "); return -1;};
 
disp();

if ( mcp23017_close_i2c (chip_tft) < 0){ perror("I2C erorr close() "); return -1;};


mcp23017_write_byte (chip_tft, OLATB, 0b00011111);	//выключить экран CS = 1

printf ("test \n");


return (EXIT_SUCCESS);
}




void test(int n,int t){
	
uint8_t res = 0;	
	
for(int i=0;i<n;i++){
	
print_char_tft(&TFT_screen_1,res);
res++;
if(res >= t)res=0;

};	

};



void disp(){
	
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
struct TFT_windows window2={
		.image_x0 = 200,
		.image_y0 = 00,
		.image_x1 = 460,
		.image_y1 = 320,
		.cursor_x = 0,
		.cursor_y = 0,
		.color_font = color_WHITE,
		.color_background = color_BLACK,
		.font = console18pt		
	};

struct TFT_windows window3={
		.image_x0 = 20,
		.image_y0 = 80,
		.image_x1 = 200,
		.image_y1 = 200,
		.cursor_x = 0,
		.cursor_y = 0,
		.color_font = color_WHITE,
		.color_background = color_BLACK,
		.font = number20pt		
	};		
	
	
	
	
	
	
	
		
	
init_ili9486(&TFT_screen_1);
tft_setWindow(&TFT_screen_1,0,0,479,319);
write_tft_clear_fast(&TFT_screen_1,0x00);



TFT_screen_1.window = &window1; 
test(700,178);

TFT_screen_1.window = &window2;
test(200,182);

TFT_screen_1.window = &window3;
test(10,10);

printf("cursor_x = %d \n cursor_y = %d \n  x = %d \n",
    TFT_screen_1.window->cursor_x, 
    TFT_screen_1.window->cursor_y,
    TFT_screen_1.window->image_x0
    );




};
/*
printf("image_x = %d \n image_y = %d \n  font_size = %d \n chip = %s \n chip1 = %s \n chip.addr = %x \n image_max_x = %d \n",
    TFT_screen_1.image_x0, 
    TFT_screen_1.image_y0,
    TFT_screen_1.font_size[0],
    TFT_screen_1.tft_model->chip.i2_bus,
    chip_tft -> i2_bus,
    TFT_screen_1.tft_model->chip.address,
    TFT_screen_1.tft_model->image_max_x
    
    );
    
   
    
 printf("image_x = %d \n image_y = %d \n  font_size = %d \n chip = %s \n  chip.addr = %x \n image_max_x = %d \n",
    TFT_screen_1.image_x1, 
    TFT_screen_1.image_y1,
    TFT_screen_1.font_size[0],
    TFT_screen_1.tft_model->chip.i2_bus,
    TFT_screen_1.tft_model->chip.address,
    TFT_screen_1.tft_model->image_max_x
    
    );   
 */   


