
 /* 
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
 * 
 *  для себя
 * 
  * отображать температуру вкл.выкл динамически
  * вывод картинки .bmp
  * простая векторная графика
  *   
 * 
 * 
 * 
 * 
 rgb888 to rgb565
 (uint8_t r, uint8_t g, uint8_t b) {
 return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); 
 } 
 * 
 * 
 * 
 */
#include "font/20pt.h"
#include "font/console8pt.c"
#include "font/console18pt.c"

// Иницилизация экрана на ili9486 -> схема подключения 8 бит.
//#include "tft_i2c/ili9486.h"
//#include "tft_i2c/r61581.h"



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
#define reset_on   0b00000000
#define reset_of   0b00011001

#define STOP_INIT  0xFF
#define PAUSE_INIT 0xFE 

#define INIT_ILI9486 init_ili9486
#include "init_ili9486.h"
#define INIT_R61581  init_r61581
#include "init_r61581.h"




typedef struct {
	
char 		*i2_bus;
uint16_t 	address;
int      	chip_open;
int       	bus_open;
 
uint8_t 	registr[0x16];
uint8_t 	buf[8192];

pthread_mutex_t lock_registr;

struct i2c_msg msg [MAX_msg];
struct i2c_rdwr_ioctl_data rdwr;

}MCP23017_TFT;


struct TFT_i2c {
		MCP23017_TFT chip;
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
	

	pthread_mutex_t lock_command;
	uint8_t buffer_command[256];
	void 	*buffer_data[256];
	uint8_t position_buffer;
	uint8_t num_commands;
	
	uint8_t command;
	void   *data;
	//void   *atrrib_command;
	//uint8_t status_command;
		
	struct itimerspec time;
	
	pthread_t tft_thread;
		
	uint8_t buf[8192];
};

void init_tft_R61581(struct TFT_screen_panel *panel);



//--------------------------------------------------------------------//
// Блок отвечает за отображение статической и динамической информации //
//-------------------------------------------------------------------//
#define MAX_change_block 7  // максимальное количество изменяемых 
							// значений в блоке
struct TFT_info_block{
	uint8_t lock_block;
	uint8_t *init_block;    // массив статического отображения информации

	uint8_t *visible_block[MAX_change_block]; // изменения в блоке, n > 0 зайти в блок и изменить информацию на экране.
											  // n>2..255 зизменить с задержкой n-циклов 
														
	uint8_t *change_block[MAX_change_block];  // параметры (курсор. шрифт) где  находится на экране изменияемый текст 
	uint8_t *change_text[MAX_change_block];   // указатель на блок где находится отображаемый текст
	uint8_t *new_change_txt[MAX_change_block];// новый текст - текст, показание температуры и др датчиков
	uint8_t lock_change;                      // тtf_fast  выставляет номер блока который выводит на экран на данный момент.
											  // 0 - его здесь нет ))

	uint8_t N_change_block; 		// количество изменяемых параметров.
	uint8_t N_fast_change_block	; 	// количество быстро изменяемых значений в этом блоке
								    // берется с начала списка n- штук на вывод
	uint8_t N_change_char_text[MAX_change_block]; // количество символ в строке
	
								    
	uint8_t pin[MAX_change_block]; // пин для  индикатора, счетчика и т.п. за котором он следит   				    
	uint8_t pin_previous_values[MAX_change_block]; // предыдущие значение этого пина						    

	uint16_t indikator_on_color[MAX_change_block];							    	
	uint16_t indikator_off_color[MAX_change_block];

   void (*Change_InfoBlock[MAX_change_block])(struct TFT_screen_panel *panel, int, int ); // обработка данных в этом блоке
   uint8_t (*get_pin)(uint8_t);
}; 



//////////////////////////////////   open ////////////////////////////////////////////////////////////
int tft_open_mcp_i2c(MCP23017_TFT *chip){	

	chip->bus_open = open( chip->i2_bus , O_RDWR);
	if(chip->bus_open < 0) perror("mcp23017_TFT error open() I2C;");			
		
return chip->bus_open; };
//////////////////////////////////   close ////////////////////////////////////////////////////////////
int tft_close_mcp_i2c (MCP23017_TFT *chip){

    int chip_close = close(chip->bus_open);    
    if (chip_close <0) perror("mcp23017_TFT error close() I2C");		
		
return chip_close;};
//////////////////////////////////   ioctl ////////////////////////////////////////////////////////////
int tft_ioctl(MCP23017_TFT *chip){
	
	if(tft_open_mcp_i2c(chip) < 0) return -1;
	
	if (ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr)) <0) perror("I2C erorr rdwr");
	
	return tft_close_mcp_i2c(chip);
	};
//////////////////////////////////   write byte to mcp23017 ////////////////////////////////////////////////////////////

int tft_write_byte_to_mcp (MCP23017_TFT *chip , uint8_t registr, uint8_t value){

//pthread_mutex_lock (&chip->lock_registr);
	
	uint8_t buf[2] = {registr,value};
	struct i2c_msg msg [1] = {{chip->address, 0, 2, buf }};  //выставить адрес и записать
	struct i2c_rdwr_ioctl_data rdwr = {msg, 1 };	
	chip->registr[registr] = value;
	int  error = ioctl(chip->bus_open, I2C_RDWR,&rdwr);
	if ( error < 0 ) perror("I2C mcp23017_TFT tft_write_byte erorr rdwr");

//pthread_mutex_unlock (&chip->lock_registr);	

	return  error;
};
	
/////////////////////////////////////// TFT command ///////////////////////////////////////////////////

void write_tft_command (struct TFT_screen_panel *panel,uint8_t value){

	MCP23017_TFT *chip = &(panel->tft_model-> chip);									 // buf[0]  - номер  регистра
	
	panel->buf[0] = GPIOA;				
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

	
	
   MCP23017_TFT *chip = &(panel->tft_model-> chip);									 
	
	panel->buf[0] = GPIOA;				// buf[0]  - номер  регистра
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
											
	MCP23017_TFT *chip = &(panel->tft_model-> chip);
										
 uint8_t  x0_msb = x0 >> 8;
 uint8_t  x0_lsb = x0 & 0xFF;
 uint8_t  x1_msb = x1 >> 8;
 uint8_t  x1_lsb = x1 & 0xFF;
 uint8_t  y0_msb = y0 >> 8;
 uint8_t  y0_lsb = y0 & 0xFF;
 uint8_t  y1_msb = y1 >> 8;
 uint8_t  y1_lsb = y1 & 0xFF;
 
  	
    uint8_t buf[] = {GPIOA,				// buf[0]  - номер  регистра
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
	MCP23017_TFT *chip = &(panel->tft_model-> chip);
	int chip_write;	
	
	tft_activeWindow (panel);
	
	uint32_t size_byte = (window->image_x1 - window->image_x0)*(window->image_y1 - window->image_y0); 

	uint8_t  color_msb = window->color_background >> 8;
	uint8_t  color_lsb = window->color_background & 0xFF;

	  panel->buf[0] = GPIOA;				// buf[0]  - номер  регистра
	  int len_byte = 1;						// -buf[1]  - что писать в этот регистр
	  uint16_t size_col = 1;					
	  uint16_t ostatok = 0;					
						
	if (size_byte > 960){ size_col = size_byte/960;
				   		  ostatok = size_byte%960;
						  size_byte =960;
						  }; 																					
																					
	for(int i=0; i< size_byte; i++){  
														// X*Y
		panel->buf[len_byte++]= color_msb; 
		panel->buf[len_byte++]= data_on; 
		panel->buf[len_byte++]= color_msb;
		panel->buf[len_byte++]= data_of;
		
		panel->buf[len_byte++]= color_lsb; 
		panel->buf[len_byte++]= data_on; 
		panel->buf[len_byte++]= color_lsb;
		panel->buf[len_byte++]= data_of;			
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
    
	
	MCP23017_TFT *chip = &(panel->tft_model-> chip);			
	int chip_write;
	int len_byte;
	
	panel->buf[0] = GPIOA;				// задаем цвет заполнения
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
	
					panel->buf[0] = GPIOB_8bit;	 					
					len_byte = 1 ;					
																	// формируем буфер стробов \_/ 1-0-1						
					for(int i=0; i< 3840; i++){             		// 480x2 3840*2 4 строки  заполнения экрана 
						panel->buf[len_byte++]= data_on;			// строб на WR \_/
						panel->buf[len_byte++]= data_of;
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
	
	MCP23017_TFT *chip = &(panel->tft_model-> chip); 
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
	panel->buf[0] = GPIOA;

	
		for( uint8_t n=0; n < sizeYfont; n++){
					
			for (uint8_t l=0; l < colXbayt; l++){

				for (uint8_t i=0; i<8 ; i++){
				
					if (((window->font[znak])&(1<<(7-i)))!=0){
									
						panel->buf[len_byte++]= color_msb;
						panel->buf[len_byte++]= data_on; 
						panel->buf[len_byte++]= color_msb;
						panel->buf[len_byte++]= data_of;
						
						panel->buf[len_byte++]= color_lsb;
						panel->buf[len_byte++]= data_on;					
						panel->buf[len_byte++]= color_lsb;
						panel->buf[len_byte++]= data_of;
						//printf ("1");
					} else {
						
						panel->buf[len_byte++]= background_msb;
						panel->buf[len_byte++]= data_on;
						panel->buf[len_byte++]= background_msb;
						panel->buf[len_byte++]= data_of;
						
						panel->buf[len_byte++]= background_lsb; 
						panel->buf[len_byte++]= data_on;						
						panel->buf[len_byte++]= background_lsb;
						panel->buf[len_byte++]= data_of;
						
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
	
int d = 0, i;
uint8_t data8_size = 0;
struct timespec pause;
uint8_t *init_buf = panel->tft_model->init_display;

MCP23017_TFT *chip = &(panel->tft_model->chip); 

	chip->msg[0].addr = chip->address;							
	chip->msg[0].flags = 0;
	chip->msg[0].buf = panel->buf;							
	chip->rdwr.msgs = chip->msg;
	chip->rdwr.nmsgs = 1;

	panel->buf[0] = GPIOA;
	
while(*(init_buf + d) != STOP_INIT ){
		
	switch (*(init_buf + d)){ 
				
		case PAUSE_INIT:
		d++;
		pause.tv_sec = 0;
	    pause.tv_nsec = *(init_buf + d) * 100000000;
		nanosleep(&pause,NULL);
		d++;					 		
		break;
		
		default :
				
		i = 1;				
		
		panel->buf[i++]   = *(init_buf + d);
		panel->buf[i++] = comm_on;
		panel->buf[i++] = *(init_buf + d++);
		panel->buf[i++] = comm_of;
				
		data8_size = *(init_buf + d++);
		
			for(uint8_t s=0 ; s < data8_size; s++ ){
						
				panel->buf[i++] = *(init_buf + d);
				panel->buf[i++] = data_on;
				panel->buf[i++] = *(init_buf + d++);
				panel->buf[i++] = data_of;			
				
			};
		
		chip->msg[0].len = i;
		
/*  test code		
		for( int t=0; t < i; t++){printf ("%0x",panel->buf[t]);}
		printf (" -> %d \n",i);
*/		
				
		int chip_write  = ioctl(chip->bus_open, I2C_RDWR, &(chip->rdwr));
		if ( chip_write < 0 ) perror("I2C erorr  function : init_TFT_panel");
						
		break;		
		
	};
		
  };
 
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

void build_info_block_to_display(struct TFT_screen_panel *panel){
		
	for (int i=0;i < panel->N_info_block; i++){
		
			tft_printf(panel, panel->info_block[i]->init_block);		
	};
	

};
/////////////////////////////////////////////////////////////////////////////////////////
// Просто обновить строку 															   //
/////////////////////////////////////////////////////////////////////////////////////////

void tft_print_change_block(struct TFT_screen_panel *panel, int n_block,int n_fast_block){
	
	panel->info_block[n_block]->lock_change = n_block+1;
	
	tft_printf(panel,panel->info_block[n_block]->change_block[n_fast_block]);
	tft_printf(panel,panel->info_block[n_block]->change_text[n_fast_block]);
	
	panel->info_block[n_block]->lock_change = 0;
		
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

///////////////////////////////////////////////////////////////////////////////////////////
// В зависимости от значения пина фон текста подсвечивается или наоборот.				//	
//////////////////////////////////////////////////////////////////////////////////////////

void tft_fast_change_led (struct TFT_screen_panel *panel, int n_block,int n_fast_block){
	
	uint8_t per = panel->info_block[n_block]->get_pin(panel->info_block[n_block]->pin[n_fast_block]);
	
	if(per == panel->info_block[n_block]->pin_previous_values[n_fast_block]) return;

	
	panel->info_block[n_block]->pin_previous_values[n_fast_block] = per;
	uint16_t per1 = panel->window->color_background;
	
// Сравнить значение бита с битом отмеченным как включающий и вывести фон под текстом исходя из этого 
	
	if(per == 1) { ///////////////исправить!!!!
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

	panel->info_block[n_block]->lock_change = n_block+1;
 
	tft_printf(panel, panel->info_block[n_block]->change_block[n_fast_block]); // номер блока и элемент в блоке.

// Перебираем строку, если цифра на экране и полученные цифра от датчика совпадает, символ не выводим	
		
	for(int r=0; r< panel->info_block[n_block]->N_change_char_text[n_fast_block]; r++){
		
		per  = panel->info_block[n_block]->new_change_txt[n_fast_block][r];
		per1 = panel->info_block[n_block]->change_text[n_fast_block][r];
		panel->info_block[n_block]->change_text[n_fast_block][r] = per;
			
			if(per == per1) per = 0x1f; // символ на экран не выводим, но позицию сдвигаем				
			print_char_tft (panel,per);
	};
	
	panel->info_block[n_block]->lock_change = 0;
};	


///////////////////////////////////////////////////////////////////////////////////////
//  Обновить параметры на экранe, которые мониторятся в реальном времени)))			//
//////////////////////////////////////////////////////////////////////////////////////

int fast_change_block_to_display (struct TFT_screen_panel *panel){
		
	int status_visible = 0;	
		
// Перебираем блоки на панели

	  for (int i=0;i < panel->N_info_block; i++){			

// Перебираем элементы в выбранном блоке, исходя из того что они должны обновлятся постоянно
// данные элементы должны быть в начале списка

	   if(panel->info_block[i]->N_fast_change_block == 0) continue; // нету таких, переходим к другому блоку
	   
		for(int s=0; s < panel->info_block[i]->N_fast_change_block; s++){	// N_fast - указывает сколько их в блоке

// Вызываем подпрограмму для обработки элемента в блоке, каждому элементу своя подпрограмма
// ну или просто дублируем, если не сделать  Segmentation fault - гарантирован
// panel->info_block[i] -> Change_InfoBlock[s](panel,i,s); 
// 0 - пропускаем ; 1 - вывести; 2...254 - задержка перед выводом ;255 - постоянно выводим		
		
			if(*panel->info_block[i]->visible_block[s] > 0) { 
			
					//status_visible += (*panel->info_block[i]->visible_block[s]);
			
				if(*panel->info_block[i]->visible_block[s] < 0xff) {
					
					status_visible ++;
					
					if(!(--(*panel->info_block[i]->visible_block[s]))) panel->info_block[i]->Change_InfoBlock[s](panel,i,s);
								
				}else {panel->info_block[i] -> Change_InfoBlock[s](panel,i,s);}
									
			};				
		}; 	
	 };	
	 
return status_visible;	 
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
	
	uint8_t number;
	 void 	 *data;
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

void tft_menu_changes(struct TFT_menu *menu){
	
 struct Menu_line *n_line;
 n_line = menu->selected_line;

	for (int i =0;i< menu->info_block.N_change_block;i++){				
			menu->info_block.change_text[i] = n_line->text;
			*menu->info_block.visible_block[i] = 1;	
			n_line = n_line->next;
	};
	
};
////////////////  Отобразить все блоки меню ///////////

void tft_menu_visible(struct TFT_menu *menu){
 for (int i =0;i< menu->info_block.N_change_block;i++){
	*menu->info_block.visible_block[i] = 1;	
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
	menu->info_block.change_block[n] = menu->visible_selection_off[n];
	*menu->info_block.visible_block[n] = 1;
};

//...........Установить выделение строки....//

void tft_menu_selection_line(struct TFT_menu *menu,uint8_t n,uint8_t t){
	
	menu->n_selected_menu_line = n;
	menu->info_block.change_block[n] = menu->visible_selection_on[n];
	*menu->info_block.visible_block[n] = t;
};

//////////////// Опускатся вниз по меню /////////////////////////////

void tft_menu_down(struct TFT_menu *menu){

	if(menu->selected_line->next == NULL) return;	

	uint8_t n = menu->n_selected_menu_line;
   
	// lock for menu
	if(*menu->info_block.visible_block[n]) return;
	
	if((n+1) < menu->info_block.N_change_block){	 
	//убрать выделение текста
	tft_menu_deselect_line (menu);
	//перейти на новую строку 
	menu->selected_line = menu->selected_line->next;	
	// и выделить текст
	tft_menu_selection_line (menu,++n,1);	
}else{
	tft_menu_deselect_line (menu);	
	menu->selected_line = menu->selected_line->next;	
	menu->tft_menu_change(menu);			
	tft_menu_selection_line (menu,0,1);
	};	
};

/////////////// Поднятся вверх по меню//////////////////////////////

void tft_menu_up(struct TFT_menu *menu){

	if(menu->selected_line->prev == NULL) return;	
	uint8_t n = menu->n_selected_menu_line;	

	if(*menu->info_block.visible_block[n]) return;
		
 if(n){	tft_menu_deselect_line (menu);
		menu->selected_line = menu->selected_line->prev;  
		tft_menu_selection_line (menu,--n,2);					
}else{
		uint8_t n_size=menu->info_block.N_change_block;
		tft_menu_deselect_line (menu);

		for (uint8_t i=0; i < n_size; i++){
			 menu->selected_line = menu->selected_line->prev;
			if(menu->selected_line->prev == NULL) break;
		};
					
		menu->tft_menu_change(menu);				
		for (uint8_t i=0; i < n_size-1; i++){menu->selected_line = menu->selected_line->next;};		
		tft_menu_selection_line (menu,n_size-1,1);			
	};	
	
};


/////////////////// Вход в подменю //////////////////////////////// 

void tft_menu_submenu(struct TFT_menu *menu){
	
  if(menu->selected_line->submenu == NULL) return;

	uint8_t n = menu->n_selected_menu_line;
   
	if(*menu->info_block.visible_block[n]) return;
		
    tft_menu_deselect_line (menu);
  	menu->selected_line = menu->selected_line->submenu;	
	menu->tft_menu_change(menu);
	tft_menu_selection_line (menu,0,1);
	
};
	
////////////////// Выход из подменю ///////////////////////////////
	
void tft_menu_exit(struct TFT_menu *menu){
	
	
  if(menu->selected_line->exit == NULL) return;

	uint8_t n = menu->n_selected_menu_line;
	
	if(*menu->info_block.visible_block[n]) return;
	
	tft_menu_deselect_line (menu);		
	menu->selected_line = menu->selected_line->exit;	
	menu->tft_menu_change(menu);	
	tft_menu_selection_line (menu,0,1);
	
};
	
////// времмено ничего не активируется null функция//////////////// 

void tft_menu_no_activation(struct TFT_menu *menu){};
	
////////////////// Активировать функцию линии//////////////////////

void tft_menu_enter(struct TFT_menu *menu){
	
	menu->selected_line->activation_menu_line(menu);

};


////////////////// Перевод целого числа(32 bit) в символьное представление to char////////////

uint32_t tft_data_to_char_table[11] ={
	0,
	1,
	10,
	100,
	1000,
	10000,
	100000,
	1000000,
	10000000,
	100000000,
	1000000000,
};

void tft_data_to_char(struct TFT_info_block *info_block, // структура для отоброжения инфо на tft
								uint8_t n_block, 		 // номер элемента в info_block
								uint32_t length,    	 // длина числа 
								uint32_t data_to_char){	 // число

 uint8_t i,numeral,symbol = 0x20; 
 uint8_t max_char_text = info_block->N_change_char_text[n_block];
 
 if(max_char_text < length && length == 0 ) 
		return perror("not correct data to char \n");
	
 uint8_t correct = max_char_text - length;	
 if(correct) for(i=0; i<correct;i++)
			 info_block->new_change_txt[n_block][i] = symbol;
				
 uint32_t del = tft_data_to_char_table[length];  ///del /=10;	
	
	for(i=0;i<length;i++){	
		
	   if(data_to_char >= del){numeral = data_to_char/del; 
							   data_to_char %=del;
							   symbol = 0x30;
		}else numeral = 0;									    
	
	if ((del /= 10) < 1) symbol = 0x30;
	
	info_block->new_change_txt[n_block][i+correct] = symbol + numeral;						
	};
	
*info_block->visible_block[n_block] = 1;	
};

/******************************************************************************************
 * 	Кольцевой буфер команд и данных 													  *								 * 
 ******************************************************************************************/
 
int tft_set_command(struct TFT_screen_panel *panel, uint8_t command, void *data){

   uint8_t res;
    
	if(pthread_mutex_lock(&panel->lock_command)) perror("error mutex lock buffer_command \n");
	
	 res = panel->position_buffer + (panel->num_commands++);
	 panel->buffer_command[res] = command;
	 panel->buffer_data[res] = data;

	if(pthread_mutex_unlock(&panel->lock_command)) perror("error mutex unlock buffer_command \n");	
	
	//printf("buffer position %d num_command %d  res %d  \n",panel->position_buffer,panel->num_commands,res);
};

/*________________________________________________________________________*/

int tft_get_command(struct TFT_screen_panel *panel){

uint8_t res = 0;

  if(panel->num_commands) { 
	
  	if(!pthread_mutex_trylock(&panel->lock_command)){
		
		panel->data = panel->buffer_data[panel->position_buffer];
		res = panel->buffer_command[panel->position_buffer++];
		panel->command = res;
		panel->num_commands--;	
	
	pthread_mutex_unlock(&panel->lock_command);};	
  };
return res;
};

/***********************************************************************
 * TFT панель - обновления экрана происходит в отдельном потоке		   *
 ***********************************************************************/

#define TFT_CYCLE 1
#define TFT_MENU_UP 2
#define TFT_MENU_DOWN 3
#define TFT_MENU_ENTER 4
#define TFT_FAST_CLEAR 10
#define TFT_CLEAR 11
#define TFT_BUILD_INFO_BLOCK 12
#define TFT_UPDATE_CHANGE_BLOCK 13
#define TFT_DISABLE 14
#define TFT_ENABLED 15
#define TFT_INIT 16
#define TFT_PAUSE 0

void pthread_tft_panel(void *n_panel){
	
 struct TFT_screen_panel *panel;
 panel  = (struct TFT_screen_panel *) n_panel;
 MCP23017_TFT *chip = &panel->tft_model->chip;
 struct TFT_menu *menu;
 //120 ms reset tft
 struct timespec pause;
 
 uint8_t command;
 void *data;
	
 while(panel->command){


	if(panel->num_commands){
			
	if(pthread_mutex_trylock(&panel->lock_command)){		
		command = TFT_CYCLE;
	}else{				
		panel->data = panel->buffer_data[panel->position_buffer];
		command = panel->buffer_command[(panel->position_buffer)++];
		(panel->num_commands)--;						
		pthread_mutex_unlock(&panel->lock_command);
	};
		 
	}else command = TFT_CYCLE;


	panel->command = command;
	
	switch (command){
		
		
			case TFT_CYCLE: //крутится в цикле перестраивая экран
            
            
			if(!(fast_change_block_to_display(panel))) 
				//sched_yield();
				// Заснуть на время если не запланированно никаких изменений на следущий проход
				// чтоб не нагружать проц постоянным бегом по кольцу ))

				nanosleep (&panel->time.it_value,NULL);  
	
			break;
			
			case TFT_MENU_UP: 			
			menu = (struct TFT_menu *) panel->data;
			menu->tft_menu_up(menu);									 
			break;
			
			case TFT_MENU_DOWN: 				
			menu = (struct TFT_menu *) panel->data;
			menu->tft_menu_down(menu);						
			break;
					  
			case TFT_MENU_ENTER: //крутится в цикле перестраивая экран			
			menu = (struct TFT_menu *) panel->data;
			menu->tft_menu_enter(menu);			
			
			break;		  
					  
						 					  
			case TFT_FAST_CLEAR: //		
			write_tft_clear_fast(panel,0x00);
			break;
			
			case TFT_CLEAR: //
			tft_clear_window(panel);
			break;
			
			case TFT_BUILD_INFO_BLOCK:
			build_info_block_to_display (panel);
			break;

			case TFT_UPDATE_CHANGE_BLOCK:
			update_change_full_block_to_display(panel);
			break;		
						
			case TFT_DISABLE:
			write_tft_command(panel,0x28);
			
			break;	
			
			case TFT_ENABLED:
			write_tft_command(panel,0x29);
			break;	
			
			case TFT_INIT:
			
			
			tft_write_byte_to_mcp(chip,0x05,0);
			tft_write_byte_to_mcp(chip,0x0A,0);
			
			// чип mcp - выставить дефолтные значения
			// при иницилизации выставить значения в этих регистрах. 
			chip->registr[IODIRA] = 0xFF;
			chip->registr[IODIRB] = 0xFF;
			chip->registr[OLATA]  = 0;
			chip->registr[OLATB]  = 0b00011111;
			
			for (int i=0 ; i <0x16 ; i++)
			tft_write_byte_to_mcp(chip,i,chip->registr[i]);
			
			tft_write_byte_to_mcp(chip,IODIRA,0);
			tft_write_byte_to_mcp(chip,IODIRB,0);

			tft_write_byte_to_mcp(chip, OLATB, 0b00010111); // reset
			tft_write_byte_to_mcp(chip, OLATB, 0b00011111);
						
			pause.tv_sec = 0;
			pause.tv_nsec = 150000000;
						
			nanosleep(&pause,NULL); //150ms если не сделать будут артефакты.
			
			// Включить чтоб байты писались по кругу (для экрана)
			tft_write_byte_to_mcp(chip,IOCON, 0b00100000);  
			init_TFT_panel(panel);
			
			break;				
						
			default: sched_yield();
	};	
};	
			
pthread_exit(NULL);	
};

/*----------------------------------------------------------------------*
 * Иницилизация и запуск экрана. 										*
 * ---------------------------------------------------------------------*/

void pthread_start_tft_panel(struct TFT_screen_panel *panel){
 
 if (tft_open_mcp_i2c (&panel->tft_model->chip) < 0){
	 perror("I2C erorr open() tft_open_mcp_i2c ");
	 exit(-1);
	 };
 
 int potok = pthread_create(&panel->tft_thread,NULL,(void*)pthread_tft_panel,(void *) panel);
 if (potok){printf("ERROR; return code from pthread_create() is %d\n", potok);exit(-1);};

};






/*
////////////// на удаление



/*
Обработка вывода на экран , экран находится в своем отдельном потоке 
считывает данные для отображения, не какие не меняет только читает.
наверно мютекс не нужен. 

void *tft_panel(void *n_panel){
	
 struct TFT_screen_panel *panel;
 
 panel  = (struct TFT_screen_panel *) n_panel;
	
while(1){

 fast_change_block_to_display (panel);
 
 };	
	
pthread_exit(NULL);	
};
*/
