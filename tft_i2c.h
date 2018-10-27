
 /* To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 * File:   main.c
 * Author: Aleksey Mamontov
 * Created on 24/ 10/2018 г., 23:05
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
#include "mcp23017.h"
#include "20pt.h"
#include "console8pt.c"
#include "console18pt.c"

#define color_BLACK		0x0000
#define color_WHITE		0xFFFF
#define color_RED		0xF800
#define color_GREEN		0x0400
#define color_BLUE		0x001F
#define color_SILVER	0xC618
#define color_GRAY		0x8410
#define color_MAROON	0x8000
#define color_YELLOW	0xFFE0
#define color_OLIVE		0x8400
#define color_LIME		0x07E0
#define color_AQUA		0x07FF
#define color_TEAL		0x0410
#define color_NAVY		0x0010
#define color_FUCHSIA	0xF81F
#define color_PURPLE	0x8010


// 0b0-0-0-RD-RST-RS-CS-WR

#define comm_on 0b00011000  
#define comm_of 0b00011001
#define data_on 0b00011100
#define data_of	0b00011101


struct TFT_i2c {
 uint16_t image_max_x;
 uint16_t image_max_y;
uint8_t *init_display;
		 MCP23017 chip;
};

struct TFT_windows {
	uint16_t image_x0;
	uint16_t image_y0;
	uint16_t image_x1;
	uint16_t image_y1;
	uint16_t cursor_x;
	uint16_t cursor_y;
	uint16_t color_font;
	uint16_t color_background;
	uint8_t *font;	
};


struct TFT_screen_panel {
	struct TFT_windows *window;
	struct TFT_i2c *tft_model;
};


	
/////////////////////////////////////// TFT command ///////////////////////////////////////////////////

void write_tft_command (struct TFT_screen_panel *panel,uint8_t value){

	MCP23017 *chip = &(panel->tft_model-> chip);									 // buf[0]  - номер  регистра
	
	chip->buf[0] = OLATA;				
	chip->buf[1] = value;
	chip->buf[2] = comm_on;
	chip->buf[3] = value;
	chip->buf[4] = comm_of;																	// buf[1]  - что писать в этот регистр							
									
		chip->msg[0].addr = chip->address;							
		chip->msg[0].flags = 0;							
		chip->msg[0].len = 5;
		chip->msg[0].buf = chip->buf;							
		chip->rdwr.msgs = chip->msg;
		chip->rdwr.nmsgs = 1;																							
        

        int chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr) );
				if ( chip_write < 0 ) perror("I2C erorr rdwr");

 }
 
///////////////////////////////////////  TFT data ///////////////////////////////////////////////////

void write_tft_data (struct TFT_screen_panel *panel ,uint8_t value){

	
	
   MCP23017 *chip = &(panel->tft_model-> chip);									 
	
	chip->buf[0] = OLATA;				// buf[0]  - номер  регистра
	chip->buf[1] = value;				// buf[1]  - что писать в этот регистр
	chip->buf[2] = data_on;
	chip->buf[3] = value;
	chip->buf[4] = data_of;												
																		
																						
        chip->msg[0].addr = chip->address;							
		chip->msg[0].flags = 0;							
		chip->msg[0].len = 5;
		chip->msg[0].buf = chip->buf;							
		chip->rdwr.msgs = chip->msg;
		chip->rdwr.nmsgs = 1;																							
        

        int chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr) );
				if ( chip_write < 0 ) perror("I2C erorr rdwr");

 };

/////////////////////////////////// TFT setWindow /////////////////////////////////

int tft_setWindow (struct TFT_screen_panel *panel, 
									uint16_t x0,
									uint16_t y0,
									uint16_t x1,
									uint16_t y1){
										
	MCP23017 *chip = &(panel->tft_model-> chip);
										
 uint8_t  x0_msb = x0 >> 8;
 uint8_t  x0_lsb = x0 & 0xFF;
 uint8_t  x1_msb = x1 >> 8;
 uint8_t  x1_lsb = x1 & 0xFF;
 uint8_t  y0_msb = y0 >> 8;
 uint8_t  y0_lsb = y0 & 0xFF;
 uint8_t  y1_msb = y1 >> 8;
 uint8_t  y1_lsb = y1 & 0xFF;
 
  	
    uint8_t buf[] = {OLATA,			// buf[0]  - номер  регистра
										// buf[1]  - что писать в этот регистр
	 0x2A,comm_on,0x2A,comm_of,			// X
	 x0_msb,data_on,x0_msb,data_of,		// X0 msb
	 x0_lsb,data_on,x0_lsb,data_of,		// X0 lsb
	 x1_msb,data_on,x1_msb,data_of,		// X1 msb
	 x1_lsb,data_on,x1_lsb,data_of, 	// X1 lsb

	 0x2B,comm_on,0x2B,comm_of, 		// Y
	 y0_msb,data_on,y0_msb,data_of,		// Y0 msb
	 y0_lsb,data_on,y0_lsb,data_of,		// Y0 lsb
	 y1_msb,data_on,y1_msb,data_of, 	// Y1 msb
	 y1_lsb,data_on,y1_lsb,data_of,  	// Y1 lsb
	 
	 0x2C,comm_on,0x2C,comm_of, 		// start to fill
};
					
		chip->msg[0].addr = chip->address;							
		chip->msg[0].flags = 0;							
		chip->msg[0].len = 45;
		chip->msg[0].buf = buf;							
		chip->rdwr.msgs = chip->msg;
		chip->rdwr.nmsgs = 1;	
					
		int chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
		if ( chip_write < 0 ) perror("I2C erorr rdwr");
																											

return chip_write;
};


////////////////////////// TFT clear (fill) 480x320 -> byte lcd ///////////////////////////////

int write_tft_clear (struct TFT_screen_panel *panel ,uint16_t color){

	
	MCP23017 *chip = &(panel->tft_model-> chip);
	int chip_write;	
	 
	uint8_t  color_msb = color >> 8;
	uint8_t  color_lsb = color & 0xFF;

	  chip->buf[0] = OLATA;				// buf[0]  - номер  регистра
	  int len_byte = 1;					// buf[1]  - что писать в этот регистр
																					
	for(int i=0; i< 960; i++){  
										// 480x2
		chip->buf[len_byte]= color_msb; len_byte++; 
		chip->buf[len_byte]= data_on; 	len_byte++; 
		chip->buf[len_byte]= color_msb;	len_byte++;
		chip->buf[len_byte]= data_of;	len_byte++;
		
		chip->buf[len_byte]= color_lsb; len_byte++; 
		chip->buf[len_byte]= data_on; 	len_byte++; 
		chip->buf[len_byte]= color_lsb; len_byte++;
		chip->buf[len_byte]= data_of; 	len_byte++;			
	};
			chip->msg[0].addr = chip->address;							
			chip->msg[0].flags = 0;							
			chip->msg[0].len = len_byte;
			chip->msg[0].buf = chip->buf;							
			chip->rdwr.msgs = chip->msg;
			chip->rdwr.nmsgs = 1;				

				for(int i=0; i< 160; i++){                              			//320
					chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr) );
						if ( chip_write < 0 ) perror("I2C erorr rdwr");
				};

return chip_write;
};

////////////////////////// TFT clear 480x320 -> fast ///////////////////////////////
// Цвет может быть задан только 8 битами

int write_tft_clear_fast (struct TFT_screen_panel *panel,uint8_t color){


    tft_setWindow (panel,0,0,panel->tft_model->image_max_x,panel->tft_model->image_max_y);
    
	
	MCP23017 *chip = &(panel->tft_model-> chip);			
	int chip_write;
	int len_byte;
	
	chip->buf[0] = OLATA;				// задаем цвет заполнения
	chip->buf[1] = color;
	chip->buf[2] = data_on;
	chip->buf[3] = color;
	chip->buf[4] = data_of;
	
		chip->msg[0].addr = chip->address;							
		chip->msg[0].flags = 0;							
		chip->msg[0].len = 5;
		chip->msg[0].buf = chip->buf;							
		chip->rdwr.msgs = chip->msg;
		chip->rdwr.nmsgs = 1;	
					
		chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
		if ( chip_write < 0 ) perror("I2C erorr rdwr");
			
	
			chip->buf[0] = IOCON;					// переключится в режим 8 байт
			chip->buf[1] = 0b10100000;				// и пишем по кругу в регистр для строба WR
			chip->msg[0].len = 2;
	 
			chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
			if ( chip_write < 0 ) perror("I2C erorr rdwr");
	
					chip->buf[0] = OLATB_8bit;	 					
					len_byte = 1 ;					
																	// формируем буфер стробов \_/ 1-0-1						
					for(int i=0; i< 3840; i++){             		// 480x2 3840*2 4 строки  заполнения экрана 
						chip->buf[len_byte]= data_on; len_byte++; 	// строб на WR \_/
						chip->buf[len_byte]= data_of; len_byte++;
					};
					
					chip->msg[0].len = len_byte;	
																	//320  4х80 = 320																     	
					for(int i=0; i< 80; i++){               
						chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
							if ( chip_write < 0 ) perror("I2C erorr rdwr");
					};

			chip->buf[0] = IOCON_8bit;		// возращаемся в режим 16 bit
			chip->buf[1] = 0b00100000;
			chip->msg[0].len = 2;
	 
			chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
			if ( chip_write < 0 ) perror("I2C erorr rdwr");

return chip_write;
};
 


////////////////////////////////// Write char to window Screen Panel ////////////////////////////////////////////

int write_char_tft (struct TFT_screen_panel *panel,uint8_t symvol){
	
MCP23017 *chip = &(panel->tft_model-> chip); 

struct TFT_windows *window = panel->window;


chip->msg[0].addr = chip->address;							
chip->msg[0].flags = 0;
chip->msg[0].buf = chip->buf;							
chip->rdwr.msgs = chip->msg;
chip->rdwr.nmsgs = 1;		

        	
 uint8_t  color_msb = window->color_font >> 8;
 uint8_t  color_lsb = window->color_font & 0xFF;
 uint8_t  background_msb = window->color_background >> 8;
 uint8_t  background_lsb = window->color_background & 0xFF;	
 uint16_t  sizeXfont = window->font[0];
 uint16_t  colXbayt = (window->font[0])/8;
 uint16_t  sizeYfont = (window->font[1]);

	int  znak = (symvol*sizeYfont*colXbayt)+2;
	int len_byte = 1;
	chip->buf[0] = OLATA;

	
		for( uint8_t n=0; n < sizeYfont; n++){
					
			for (uint8_t l=0; l < colXbayt; l++){

				for (uint8_t i=0; i<8 ; i++){
				
					if (((window->font[znak])&(1<<(7-i)))!=0){
									
						chip->buf[len_byte]= color_msb; len_byte++; 
						chip->buf[len_byte]= data_on,	len_byte++; 
						chip->buf[len_byte]= color_msb; len_byte++;
						chip->buf[len_byte]= data_of;	len_byte++;
						
						chip->buf[len_byte]= color_lsb;	len_byte++; 
						chip->buf[len_byte]= data_on;	len_byte++;						
						chip->buf[len_byte]= color_lsb; len_byte++;
						chip->buf[len_byte]= data_of;	len_byte++;
						//printf ("1");
					} else {
						
						chip->buf[len_byte]= background_msb;len_byte++; 
						chip->buf[len_byte]= data_on;		len_byte++; 
						chip->buf[len_byte]= background_msb;len_byte++;
						chip->buf[len_byte]= data_of;		len_byte++;
						
						chip->buf[len_byte]= background_lsb;len_byte++; 
						chip->buf[len_byte]= data_on;		len_byte++;						
						chip->buf[len_byte]= background_lsb;len_byte++;
						chip->buf[len_byte]= data_of;		len_byte++;
						
						//printf ("0");
					};
								
				};
			znak ++;
			};


//printf("\n");
};


								
		chip->msg[0].len = len_byte;								
					
		int chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
		if ( chip_write < 0 ) perror("I2C erorr rdwr");

};
////////////////////////////// print char //////////////////////////////////////////////////

int print_char_tft (struct TFT_screen_panel *panel,uint8_t symvol){

struct TFT_windows *window = panel->window;
uint16_t x0,x1,y0,y1,res;
	
	 x0 = (window->font[0])*(window->cursor_x)+(window->image_x0);	
	 x1 = x0 + ((window->font[0])-1);
	  
		if(x1 > window->image_x1 ){	x0 = window->image_x0;
									x1 = x0 + ((window->font[0])-1);
									window->cursor_x = 0;
									window->cursor_y = window->cursor_y + 1;
								};
	 
	y0 = (window->font[1])*(window->cursor_y)+(window->image_y0);
	y1 = y0 + ((window->font[1])-1);
	
		if(y1 > window->image_y1 ){	y0 = window->image_y0;
									y1 = y0 + ((window->font[1])-1);
									window->cursor_y = 0;
								};
	
	
	tft_setWindow (panel,x0,y0,x1,y1);
	
	write_char_tft(panel, symvol);
	
	window->cursor_x = (window->cursor_x) + 1;

};


///////////////////////////// init ILI9486 //////////////////////////////////////////////////


int init_ili9486(struct TFT_screen_panel *panel){

MCP23017 *chip = &(panel->tft_model-> chip); 	

int chip_write;
	
 uint8_t buf[261] = {OLATA,	
	
0xb0,comm_on,0xb0,comm_of,	
0x00,comm_on,0x00,comm_of,
0x11,comm_on,0x11,comm_of,

0x3a,comm_on,0x3a,comm_of,
0x55,data_on,0x55,data_of,

0xC2,comm_on,0xC2,comm_of,
0x44,data_on,0x44,data_of,

0xC5,comm_on,0xC5,comm_of,
0x00,data_on,0x00,data_of,
0x00,data_on,0x00,data_of,
0x00,data_on,0x00,data_of,
0x00,data_on,0x00,data_of,

0xE0,comm_on,0xE0,comm_of,
0x0f,data_on,0x0f,data_of,
0x1f,data_on,0x1f,data_of,
0x1c,data_on,0x1c,data_of,
0x0c,data_on,0x0c,data_of,
0x0f,data_on,0x0f,data_of,
0x08,data_on,0x08,data_of,
0x48,data_on,0x48,data_of,
0x98,data_on,0x98,data_of,
0x37,data_on,0x37,data_of,
0x0a,data_on,0x0a,data_of,
0x13,data_on,0x13,data_of,
0x04,data_on,0x04,data_of,
0x11,data_on,0x11,data_of,
0x0d,data_on,0x0d,data_of,
0x00,data_on,0x00,data_of,

0xE1,comm_on,0xE1,comm_of,
0x0f,data_on,0x0f,data_of,
0x32,data_on,0x32,data_of,
0x2e,data_on,0x2e,data_of,
0x0b,data_on,0x0b,data_of,
0x0d,data_on,0x0d,data_of,
0x05,data_on,0x05,data_of,
0x47,data_on,0x47,data_of,
0x75,data_on,0x75,data_of,
0x37,data_on,0x37,data_of,
0x06,data_on,0x06,data_of,
0x10,data_on,0x10,data_of,
0x03,data_on,0x03,data_of,
0x24,data_on,0x24,data_of,
0x20,data_on,0x20,data_of,
0x00,data_on,0x00,data_of,


0xE2,comm_on,0xE2,comm_of,
0x0f,data_on,0x0f,data_of,
0x32,data_on,0x32,data_of,
0x2e,data_on,0x2e,data_of,
0x0b,data_on,0x0b,data_of,
0x0d,data_on,0x0d,data_of,
0x05,data_on,0x05,data_of,
0x47,data_on,0x47,data_of,
0x75,data_on,0x75,data_of,
0x37,data_on,0x37,data_of,
0x06,data_on,0x06,data_of,
0x10,data_on,0x10,data_of,
0x03,data_on,0x03,data_of,
0x24,data_on,0x24,data_of,
0x20,data_on,0x20,data_of,
0x00,data_on,0x00,data_of,

0x11,comm_on,0x11,comm_of,

0x20,comm_on,0x20,comm_of,

0x36,comm_on,0x36,comm_of,
0x28,data_on,0x28,data_of,

0x29,comm_on,0x29,comm_of,
};	
	
	
	struct i2c_msg msg [1] = {{chip->address, 0, 261, buf}};
	struct i2c_rdwr_ioctl_data rdwr = {msg,1};

        chip_write  = ioctl(chip->bus_open, I2C_RDWR, &rdwr );
				if ( chip_write < 0 ) perror("I2C erorr rdwr");
	
return chip_write;
 
};

