
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
#include "font/20pt.h"
#include "font/console8pt.c"
#include "font/console18pt.c"


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



#define SET_background 0x04
#define SET_font 0x03
#define SET_cursorX 0x01
#define SET_cursorY 0x02
#define SET_font8pt 0x06
#define SET_font18pt 0x05
#define SET_fontNumber 0x07




// 0b0-0-0-RD-RST-RS-CS-WR

#define comm_on 0b00011000  
#define comm_of 0b00011001
#define data_on 0b00011100
#define data_of	0b00011101


struct TFT_i2c {
		MCP23017 chip;
 uint16_t image_max_x;
 uint16_t image_max_y;
uint8_t *init_display;
	
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


#define MAX_info_block 20 // максимальное количество информационных блоков



struct TFT_screen_panel {
	
	struct TFT_windows *window;
	struct TFT_i2c *tft_model; 
	
	struct TFT_info_block *info_block[MAX_info_block];
	uint8_t N_info_block;
	
	
		
	uint8_t buf[8192];
};

//--------------------------------------------------------------------//
// Блок отвечает за отображение статической и динамической информации //
//-------------------------------------------------------------------//
#define MAX_change_block 7  // максимальное количество изменяемых 
							// значений в блоке
struct TFT_info_block{
	
	uint8_t *init_block;    // массив статического отображения информации

	uint8_t *change_block[MAX_change_block];  // параметры (курсор. шрифт) где  находится на экране изменияемый текст 
	uint8_t *change_text[MAX_change_block];   // указатель на блок где находится отображаемый текст
	uint8_t *new_change_txt[MAX_change_block];// новый текст - текст, показание температуры и др датчиков

	uint8_t N_change_block; 		// количество изменяемых параметров.
	uint8_t N_fast_change_block	; 	// количество быстро изменяемых значений в этом блоке
								    // берется с начала списка n- штук на вывод
	uint8_t N_change_char_text[MAX_change_block]; // количество символ в строке
	
								    
	struct MCP23017_PIN *pin_indikator[MAX_change_block]; // пин для индикатора, за котором он следит))   				    
	uint8_t pin_previous_values[MAX_change_block]; // предыдущие значение этого пина						    

	uint16_t indikator_on_color[MAX_change_block];							    	
	uint16_t indikator_off_color[MAX_change_block];

   void (*Change_InfoBlock[MAX_change_block])(struct TFT_screen_panel *panel, int, int ); // обработка данных в этом блоке

}; 






//////////////////////////////////   ioctl ////////////////////////////////////////////////////////////


int tft_ioctl(MCP23017 *chip){
	
	if(mcp23017_open_i2c(chip) < 0) return -1;
	
	if (ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr)) <0) perror("I2C erorr rdwr");
	
	return mcp23017_close_i2c(chip);
	};

	
/////////////////////////////////////// TFT command ///////////////////////////////////////////////////

void write_tft_command (struct TFT_screen_panel *panel,uint8_t value){

	MCP23017 *chip = &(panel->tft_model-> chip);									 // buf[0]  - номер  регистра
	
	panel->buf[0] = OLATA;				
	panel->buf[1] = value;
	panel->buf[2] = comm_on;
	panel->buf[3] = value;
	panel->buf[4] = comm_of;																	// buf[1]  - что писать в этот регистр							
									
		chip->msg[0].addr = chip->address;							
		chip->msg[0].flags = 0;							
		chip->msg[0].len = 5;
		chip->msg[0].buf = panel->buf;							
		chip->rdwr.msgs = chip->msg;
		chip->rdwr.nmsgs = 1;																							
        

        int chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr) );
				if ( chip_write < 0 ) perror("I2C erorr rdwr");

 }
 
///////////////////////////////////////  TFT data ///////////////////////////////////////////////////

void write_tft_data (struct TFT_screen_panel *panel ,uint8_t value){

	
	
   MCP23017 *chip = &(panel->tft_model-> chip);									 
	
	panel->buf[0] = OLATA;				// buf[0]  - номер  регистра
	panel->buf[1] = value;				// buf[1]  - что писать в этот регистр
	panel->buf[2] = data_on;
	panel->buf[3] = value;
	panel->buf[4] = data_of;												
																		
																						
        chip->msg[0].addr = chip->address;							
		chip->msg[0].flags = 0;							
		chip->msg[0].len = 5;
		chip->msg[0].buf = panel->buf;							
		chip->rdwr.msgs = chip->msg;
		chip->rdwr.nmsgs = 1;																							
        

        int chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr) );
				if ( chip_write < 0 ) perror("I2C erorr rdwr");

 };


//////////////////////////  dynamic time window for font, sprite  //////////////////////////////////

int tft_dynamicWindow (struct TFT_screen_panel *panel, 
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
 
  	
    uint8_t buf[] = {OLATA,				// buf[0]  - номер  регистра
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
/////////////////////////////////// TFT setWindow /////////////////////////////////////////////////

int tft_setWindow(struct TFT_screen_panel *panel,
									uint16_t x0,
									uint16_t y0,
									uint16_t x1,
									uint16_t y1){
		panel->window->image_x0	= x0;
		panel->window->image_x1	= x1;							
		panel->window->image_y0	= y0;								
		panel->window->image_y1	= y1;							

return tft_dynamicWindow(panel,x0,y0,x1,y1);
};

///////////////////////  Actual active visible Window /////////////////////////////////

int tft_activeWindow(struct TFT_screen_panel *panel){
	
return tft_dynamicWindow (panel,
				   panel->window->image_x0,
				   panel->window->image_y0,
				   panel->window->image_x1,
				   panel->window->image_y1);
};


////////////////////////// TFT clear window  ///////////////////////////////////////////

int tft_clear_window (struct TFT_screen_panel *panel){

	struct TFT_windows *window = panel->window;
	MCP23017 *chip = &(panel->tft_model-> chip);
	int chip_write;	
	
	tft_activeWindow (panel);
	
	uint32_t size_byte = (window->image_x1 - window->image_x0)*(window->image_y1 - window->image_y0); 

	uint8_t  color_msb = window->color_background >> 8;
	uint8_t  color_lsb = window->color_background & 0xFF;

	  panel->buf[0] = OLATA;				// buf[0]  - номер  регистра
	  int len_byte = 1;						// buf[1]  - что писать в этот регистр
	  uint16_t size_col = 1;					
	  uint16_t ostatok = 0;					
						
	if (size_byte > 960){ size_col = size_byte/960;
				   		  ostatok = size_byte%960;
						  size_byte =960;
						  }; 																					
																					
	for(int i=0; i< size_byte; i++){  
														// X*Y
		panel->buf[len_byte]= color_msb; len_byte++; 
		panel->buf[len_byte]= data_on; 	len_byte++; 
		panel->buf[len_byte]= color_msb;	len_byte++;
		panel->buf[len_byte]= data_of;	len_byte++;
		
		panel->buf[len_byte]= color_lsb; len_byte++; 
		panel->buf[len_byte]= data_on; 	len_byte++; 
		panel->buf[len_byte]= color_lsb; len_byte++;
		panel->buf[len_byte]= data_of; 	len_byte++;			
	};
			chip->msg[0].addr = chip->address;							
			chip->msg[0].flags = 0;							
			chip->msg[0].len = len_byte;
			chip->msg[0].buf = panel->buf;							
			chip->rdwr.msgs = chip->msg;
			chip->rdwr.nmsgs = 1;				
			

				for(int i=0; i< size_col; i++){                              			
					   chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr) );
						if ( chip_write < 0 ) perror("I2C erorr rdwr");
				};
				
				if(ostatok > 0){ 
								chip->msg[0].len = ostatok*8;
					            chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr) );
									if ( chip_write < 0 ) perror("I2C erorr rdwr");
								};
return chip_write;
};

////////////////////////// TFT full clear 480x320 -> fast ///////////////////////////////
// Цвет может быть задан только 8 битами

int write_tft_clear_fast (struct TFT_screen_panel *panel,uint8_t color){


    tft_setWindow (panel,0,0,panel->tft_model->image_max_x,panel->tft_model->image_max_y);
    
	
	MCP23017 *chip = &(panel->tft_model-> chip);			
	int chip_write;
	int len_byte;
	
	panel->buf[0] = OLATA;				// задаем цвет заполнения
	panel->buf[1] = color;
	panel->buf[2] = data_on;
	panel->buf[3] = color;
	panel->buf[4] = data_of;
	
		chip->msg[0].addr = chip->address;							
		chip->msg[0].flags = 0;							
		chip->msg[0].len = 5;
		chip->msg[0].buf = panel->buf;							
		chip->rdwr.msgs = chip->msg;
		chip->rdwr.nmsgs = 1;	
					
		chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
		if ( chip_write < 0 ) perror("I2C erorr rdwr");
			
	
			panel->buf[0] = IOCON;					// переключится в режим 8 байт
			panel->buf[1] = 0b10100000;				// и пишем по кругу в регистр для строба WR
			chip->msg[0].len = 2;
	 
			chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
			if ( chip_write < 0 ) perror("I2C erorr rdwr");
	
					panel->buf[0] = OLATB_8bit;	 					
					len_byte = 1 ;					
																	// формируем буфер стробов \_/ 1-0-1						
					for(int i=0; i< 3840; i++){             		// 480x2 3840*2 4 строки  заполнения экрана 
						panel->buf[len_byte]= data_on; len_byte++; 	// строб на WR \_/
						panel->buf[len_byte]= data_of; len_byte++;
					};
					
					chip->msg[0].len = len_byte;	
																	//320  4х80 = 320																     	
					for(int i=0; i< 80; i++){               
						chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
							if ( chip_write < 0 ) perror("I2C erorr rdwr");
					};

			panel->buf[0] = IOCON_8bit;		// возращаемся в режим 16 bit
			panel->buf[1] = 0b00100000;
			chip->msg[0].len = 2;
	 
			chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
			if ( chip_write < 0 ) perror("I2C erorr rdwr");

return chip_write;
};
 


///////////////////////// Write char to window Screen Panel //////////////////////////////

int write_char_tft (struct TFT_screen_panel *panel,uint8_t symvol){
	
	MCP23017 *chip = &(panel->tft_model-> chip); 
	struct TFT_windows *window = panel->window;


	chip->msg[0].addr = chip->address;							
	chip->msg[0].flags = 0;
	chip->msg[0].buf = panel->buf;							
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
	panel->buf[0] = OLATA;

	
		for( uint8_t n=0; n < sizeYfont; n++){
					
			for (uint8_t l=0; l < colXbayt; l++){

				for (uint8_t i=0; i<8 ; i++){
				
					if (((window->font[znak])&(1<<(7-i)))!=0){
									
						panel->buf[len_byte]= color_msb; len_byte++; 
						panel->buf[len_byte]= data_on,	len_byte++; 
						panel->buf[len_byte]= color_msb; len_byte++;
						panel->buf[len_byte]= data_of;	len_byte++;
						
						panel->buf[len_byte]= color_lsb;	len_byte++; 
						panel->buf[len_byte]= data_on;	len_byte++;						
						panel->buf[len_byte]= color_lsb; len_byte++;
						panel->buf[len_byte]= data_of;	len_byte++;
						//printf ("1");
					} else {
						
						panel->buf[len_byte]= background_msb;len_byte++; 
						panel->buf[len_byte]= data_on;		len_byte++; 
						panel->buf[len_byte]= background_msb;len_byte++;
						panel->buf[len_byte]= data_of;		len_byte++;
						
						panel->buf[len_byte]= background_lsb;len_byte++; 
						panel->buf[len_byte]= data_on;		len_byte++;						
						panel->buf[len_byte]= background_lsb;len_byte++;
						panel->buf[len_byte]= data_of;		len_byte++;
						
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
////////////////////////////// print char ////////////////////////////////////////////////

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
	if(symvol >= 0x20) {

	symvol = symvol - 0x20;	
	tft_dynamicWindow(panel,x0,y0,x1,y1);
	write_char_tft(panel, symvol);
	
	};	

	window->cursor_x = (window->cursor_x) + 1;

};
///////////////////////////// init MCP23017 and TFT /////////////////////////////////////


int init_TFT_panel(struct TFT_screen_panel *panel){

MCP23017 *chip = &(panel->tft_model->chip); 

panel->buf[0] = OLATA; //261

for(int i=0;i< 261;i++){
	
	panel->buf[i+1] = panel->tft_model->init_display[i];
}
	
chip->msg[0].addr = chip->address;							
chip->msg[0].flags = 0;
chip->msg[0].buf = panel->buf;							
chip->rdwr.msgs = chip->msg;
chip->rdwr.nmsgs = 1;
	
		chip->msg[0].len = 261;								
					
		int chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
		if ( chip_write < 0 ) perror("I2C erorr rdwr");
		
	
return chip_write;
 
};

/////////////////////////////////// print script ///////////////////////////////////////

// Принимает массив (arr8bit) для вывода текста  и т.п. опционально -
// расширить возможности функции можно до бессконечности - самое главное
// не увлекатся )),  окончание массива end [00].

void tft_printf(struct TFT_screen_panel *panel, uint8_t *massiv){
	
	int i=0;
	
	while (massiv[i]){
	
	 if (massiv[i] >= 0x20){
		
		print_char_tft(panel,massiv[i]);
		
		}else { switch (massiv[i]){
			
					case SET_cursorX: //set cursor X
						i++;
						panel->window->cursor_x = massiv[i]; 
						break;
					case SET_cursorY:
						i++;
						panel->window->cursor_y = massiv[i];
						break;
					
					case SET_font:
						i++;
						panel->window->color_font = massiv[i];
						panel->window->color_font = panel->window->color_font << 8;
						i++;
						panel->window->color_font = panel->window->color_font | massiv[i];
						break;
					case SET_background:
						i++;
						panel->window->color_background = massiv[i];
						panel->window->color_background = panel->window->color_background << 8;
						i++;
						panel->window->color_background = panel->window->color_background | massiv[i];
					break;
					
					case SET_font18pt:
						panel->window->font = console18pt;
					break;
						
					case SET_font8pt:
						panel->window->font = console8pt;
					break;
					
					case SET_fontNumber:
					panel->window->font = number20pt;
					//printf("Массив %d \n", i);
					break;
					
					case 0x0D :
						panel->window->cursor_x = 0;
						panel->window->cursor_y = panel->window->cursor_y +1 ;
					break;						
				};
			};
		i++;
	};


};

///////////////////////////////////////////////////////////////////////////////////////
//  Построить начальный рабочий экран  											    ///
///////////////////////////////////////////////////////////////////////////////////////

int build_info_block_to_display(struct TFT_screen_panel *panel){
		
	for (int i=0;i < panel->N_info_block; i++){
		
			tft_printf(panel, panel->info_block[i]->init_block);		
	};
	

};
/////////////////////////////////////////////////////////////////////////////////////////
// 	Обновить все параметры на экране												   // 
/////////////////////////////////////////////////////////////////////////////////////////

void update_change_full_block_to_display(struct TFT_screen_panel *panel){
	
	
	for (int i=0;i < panel->N_info_block; i++){

	   if(panel->info_block[i]->N_change_block == 0) continue;
			
		for(int s=0; s < panel->info_block[i]->N_change_block; s++){				
			tft_printf(panel, panel->info_block[i]->change_block[s]);	
			tft_printf(panel, panel->info_block[i]->change_text[s]);
		};	
	};
		
};

///////////////////////////////////////////////////////////////////////////////////////////////
// В зависимости от значения пина фон текста подсвечивается или наоборот.


void tft_fast_change_led (struct TFT_screen_panel *panel, int n_block,int n_fast_block){
	
		
	uint8_t bit = panel->info_block[n_block]->pin_indikator[n_fast_block]->bit;
	uint8_t status_bit = 0b00000001 << (bit&0x7);// создаем маску бита
	
  
// Исходя из того как выставлен порт на чтение или запись опрашиваем разные регистры.
//IODIRA (1 - in ->GPIOA , 0 -out ->OLATA)

	uint8_t registr;

	if(panel->info_block[n_block]->pin_indikator[n_fast_block]->modeInOut){
				registr = GPIOA;
		} else{ registr = OLATA;};
		
// example 0...7 GPIOA  8...15 GPIOB		
	registr = registr | ((bit&0x08)>>3); 

// Информацию мы получаем из регистров которые лежат в структуре сhip _mcp23017
// при обмене данных с микросхемой, запись или чтении - эти регистры обновляются
    
	uint8_t per = panel->info_block[n_block]->pin_indikator[n_fast_block]->chip->registr[registr];
	per = (per&status_bit) >> (bit&0x7);
	
	
// Если ничего не поменялось в регистрах выходим
	
	if(per == panel->info_block[n_block]->pin_previous_values[n_fast_block]) return;

	
	panel->info_block[n_block]->pin_previous_values[n_fast_block] = per;
	uint16_t per1 = panel->window->color_background;
	
// Сравнить значение бита с битом отмеченным как включающий и вывести фон под текстом исходя из этого 
	
	if(per == panel->info_block[n_block]->pin_indikator[n_fast_block]->on) {
		    panel->window->color_background = panel->info_block[n_block]->indikator_on_color[n_fast_block];
	}else{  panel->window->color_background = panel->info_block[n_block]->indikator_off_color[n_fast_block];}
	
	tft_printf(panel, panel->info_block[n_block]->change_block[n_fast_block]);
	tft_printf(panel,panel->info_block[n_block]->change_text[n_fast_block]);				
	
	panel->window->color_background = per1;				
	
	};



/////////////////////////////////////////////////////////////////////////////////////////////////

// Обновить число на экране от сенсоров, датчиков  
// используется в структуре info_block -> Change_InfoBlock ()							        

	
void tft_fast_change_number (struct TFT_screen_panel *panel, int n_block,int n_fast_block){
	
uint8_t per,per1;
	
 tft_printf(panel, panel->info_block[n_block]->change_block[n_fast_block]); // номер блока и элемент в блоке.

// Перебираем строку, если цифра на экране и полученные цифра от датчика совпадает, символ не выводим	
		
	for(int r=0; r< panel->info_block[n_block]->N_change_char_text[n_fast_block]; r++){
		
		per  = panel->info_block[n_block]->new_change_txt[n_fast_block][r];
		per1 = panel->info_block[n_block]->change_text[n_fast_block][r];
		panel->info_block[n_block]->change_text[n_fast_block][r] = per;
			
			if(per == per1) per = 0x1f; // символ на экран не выводим, но позицию сдвигаем				
			print_char_tft (panel,per);
	};
};	


///////////////////////////////////////////////////////////////////////////////////////
//  Обновить параметры на экранe, которые мониторятся в реальном времени)))			//
//////////////////////////////////////////////////////////////////////////////////////

void fast_change_block_to_display (struct TFT_screen_panel *panel){
		
// Перебираем блоки на панели

	  for (int i=0;i < panel->N_info_block; i++){			

// Перебираем элементы в выбранном блоке, исходя из того что они должны обновлятся постоянно
// данные элементы должны быть в начале списка

	   if(panel->info_block[i]->N_fast_change_block == 0) continue; // нету переходим к другому блоку
	   
		for(int s=0; s < panel->info_block[i]->N_fast_change_block; s++){	// N_fast - указывает сколько их в блоке

// Вызываем подпрограмму для обработки элемента в блоке, каждому элементу своя подпрограмма
// ну или просто дублируем, если не сделать  Segmentation fault - гарантирован

			panel->info_block[i] -> Change_InfoBlock[s](panel,i,s); 

			}; 	
	  };	
};
	


/////////////////////////////////////////////////////////////////////////////////////////////////	
// Данный блок содержит программы для обслуживания меню, на основе связанных списков   		  // 
// Menu_Line - информация об одной строке 													 //
// TFT_menu  - хранит всю информация об меню,списки,подпрограммы обслуживания и т.д         //																			
/////////////////////////////////////////////////////////////////////////////////////////////


#define MAX_line_menu 4 // максимальное количество отображаемых меню на экране.

struct TFT_menu;

struct Menu_line{
	
	uint8_t *text;
	
	struct Menu_line *prev;
	struct Menu_line *next;
	struct Menu_line *submenu;
	struct Menu_line *exit;
	void (*activation_menu_line)(struct TFT_menu *);
	 
};

struct TFT_menu{
	
	struct Menu_line *selected_line;
	struct TFT_info_block info_block;
	
	uint8_t max_menu_line ; 
	uint8_t n_selected_menu_line;
	uint8_t *visible_selection_on[MAX_line_menu]; // переделать
	uint8_t *visible_selection_off[MAX_line_menu];
	
	
	
	void (*tft_menu_up)(struct TFT_menu *menu);
	void (*tft_menu_down)(struct TFT_menu *menu);
	void (*tft_menu_enter)(struct TFT_menu *menu);
	void (*tft_menu_change)(struct TFT_menu *menu); 
	void (*tft_clean_change_text)(struct TFT_menu *menu);
};







///////////////Перестройка текста N_строк меню для отображения////////////////

void tft_menu_change(struct TFT_menu *menu){
	
 struct Menu_line *n_line;
 n_line = menu->selected_line;

	for (int i =0;i< menu->info_block.N_change_block;i++){		
			menu->info_block.new_change_txt[i] = n_line->text;
			n_line = n_line->next;
	};
};
// Очистка массива change_text, для того чтоб построить выделение или убрать его.
// при очередном проходе tft_fast_change_number
// можно было бы чтоб очищалось пока не встретит 0 - было бы универсально
// но есть вероятность что все потрет, его не встретив))

void tft_clean_change_text (struct TFT_menu *menu){

uint8_t n = menu->n_selected_menu_line;

	for(int i=0;i < menu->info_block.N_change_char_text[n]; i++){
		menu->info_block.change_text[n][i] = 0x1f;
		};
};

//........ Cнять выделение со строки.........//
void tft_menu_deselect_line(struct TFT_menu *menu){

uint8_t n = menu->n_selected_menu_line;

	menu->tft_clean_change_text(menu);
	menu->info_block.change_block[n] = menu->visible_selection_off[n];
};
//...........Установить выделение строки....//

void tft_menu_line_selection(struct TFT_menu *menu,uint8_t n){

	menu->n_selected_menu_line = n;
	menu->tft_clean_change_text(menu);
	menu->info_block.change_block[n] = menu->visible_selection_on[n];
	
};



//////////////// Опускатся вниз по меню /////////////////////////////

void tft_menu_down(struct TFT_menu *menu){

if(menu->selected_line->next == NULL) return;	

uint8_t n = menu->n_selected_menu_line;

	if((n+1) < menu->info_block.N_change_block){
	 
	//убрать выделение текста
	tft_menu_deselect_line(menu);	
	
	//перейти на новую строку и  выделить текст
	menu->selected_line = menu->selected_line->next;
    n++;
	tft_menu_line_selection(menu,n);	
		
}else{
	// eсли выделение внизу окна то пройти дальше по списку, 
	// экран отобразит новый список меню и выделение будет на первой строке 
	tft_menu_deselect_line(menu);
	menu->selected_line = menu->selected_line->next;
	tft_menu_line_selection(menu,0);
	menu->tft_menu_change(menu);	
	};	
};

/////////////// Поднятся вверх по меню//////////////////////////////

void tft_menu_up(struct TFT_menu *menu){

if(menu->selected_line->prev == NULL) return;
	
uint8_t n = menu->n_selected_menu_line;
	
 if(n){ tft_menu_deselect_line(menu);			
		menu->selected_line = menu->selected_line->prev;  
		n--;
		tft_menu_line_selection(menu,n);	
}else{
		uint8_t n_size=menu->info_block.N_change_block;
				
		tft_menu_deselect_line(menu);

		for (uint8_t i=0; i < n_size; i++){
			menu->selected_line = menu->selected_line->prev;
			if(menu->selected_line->prev == NULL) break;
			};
		menu->tft_menu_change(menu);

		for (uint8_t i=0; i < n_size-1; i++){menu->selected_line = menu->selected_line->next;};
		tft_menu_line_selection(menu,n_size-1);
	};
};


/////////////////// Вход в подменю //////////////////////////////// 

void tft_menu_submenu(struct TFT_menu *menu){
	
  if(menu->selected_line->submenu == NULL) return;

	uint8_t n = menu->n_selected_menu_line;
	
	tft_menu_deselect_line(menu);	
	menu->selected_line = menu->selected_line->submenu;
	tft_menu_line_selection(menu,0);
	menu->tft_menu_change(menu);
};
	
////////////////// Выход из подменю ///////////////////////////////
	
void tft_menu_exit(struct TFT_menu *menu){
	
  if(menu->selected_line->exit == NULL) return;

	uint8_t n = menu->n_selected_menu_line;

	tft_menu_deselect_line(menu);	
	menu->selected_line = menu->selected_line->exit;
	tft_menu_line_selection(menu,0);
	menu->tft_menu_change(menu);
};
	
////// времмено ничего не активируется null функция//////////////// 

void tft_menu_no_activation(struct TFT_menu *menu){};
	
////////////////// Активировать функцию линии//////////////////////

void tft_menu_enter(struct TFT_menu *menu){
	
	menu->selected_line->activation_menu_line(menu);

};















