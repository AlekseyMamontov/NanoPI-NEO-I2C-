
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

struct tft_i2c ili9486 = {
	.image_max_x = 480,
	.image_max_y = 320,
	.chip = {.i2_bus ="/dev/i2c-0",
			 .address = 0x22,
			 .msg[0] = {
						.addr = 0x22
						}
			}
	};
	
struct tft_screen_panel TFT_screen_1 = {
	.image_x0 = 0,
	.image_y0 = 0,
	.image_x1 = 480,
	.image_y1 = 320,
	.cursor_x = 0,
	.cursor_y = 0,
	.color_font = color_WHITE,
	.color_background = color_BLACK,
	.font_x = 8,
	.font_y = 11,
	.font_size = lucida8,
	.tft_model = &ili9486
	};



void disp();

//////-------------------------------------------------------------------



int main(int argc, char** argv) {

MCP23017 *chip_tft = &TFT_screen_1.tft_model->chip;


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


mcp23017_write_byte (chip_tft, IODIRA,0xff);
mcp23017_write_byte (chip_tft, IODIRB,0xff);


mcp23017_write_byte (chip_tft, OLATA, 0x00);
mcp23017_write_byte (chip_tft, OLATB, 0xff);

mcp23017_write_byte (chip_tft, IODIRA,0);
mcp23017_write_byte (chip_tft, IODIRB,0);


mcp23017_write_byte (chip_tft, OLATB, 0b00011111);
//sleep(1);
mcp23017_write_byte (chip_tft, OLATB, 0b00010111);
//sleep(1);
mcp23017_write_byte (chip_tft, OLATB, 0b00011111);
//sleep(1);
mcp23017_write_byte (chip_tft, OLATB, 0b00011101);
mcp23017_write_byte (chip_tft, IOCON, 0b00100000);


if (mcp23017_open_i2c (chip_tft) < 0)  { perror("I2C erorr open() "); return -1;};
 
//ili9486();
disp();

if ( mcp23017_close_i2c (chip_tft) < 0){ perror("I2C erorr close() "); return -1;};


mcp23017_write_byte (chip_tft, OLATB, 0b00011111);	

printf ("test \n");



//goto test;






return (EXIT_SUCCESS);}



void disp(){
	
init_ili9486(&TFT_screen_1);

TFT_setWindow(&TFT_screen_1,0,0,479,319);

write_tft_clear_fast(&TFT_screen_1,0x00);


int res = 0;
int16_t x,x1,y,y1,i,d;

y=0;
for (d=0; d < 29 ; d++){
y=d*11;	//24
y1=y+10;
for (i=0; i < 60; i++){
	x=i*8;
	x1=x+7;
	TFT_setWindow (&TFT_screen_1,x,y,x1,y1);
	write_char_tft(&TFT_screen_1,res);
	
	res++;
	
	if(res >=64)res=0;
	
 };
 
};

};



