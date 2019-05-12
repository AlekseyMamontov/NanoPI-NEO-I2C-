
 /* 
 * Author: Aleksey Mamontov
 * mamontov1@gmail.com
 
 */
 
#include "mcp23017.h"

/*
+--------------------------------------------------------------------+
| Иницилизация портов на Cтанке						  				 |
| соместное использование портов c разных микросхем например		 |
* в пользовательском пространстве   						         |
! MCP23017-18 напрямую 												 |
! UAPI_GPIO               											 |
| 		 															 |
+--------------------------------------------------------------------+*/

struct nFUNCTIONS {
	
	int (*set_nPIN)(uint8_t, uint8_t);	
	int (*get_nPIN)(uint8_t);	
	int (*direction_nPIN)(uint8_t,uint8_t);	
	int (*init_nPIN)(uint8_t);	

	uint8_t (*get_nPIN_async)(uint8_t);  // быстрое получение значения    
};


struct nPIN {
	
	void   *chip;
	  
	uint8_t direction;// 1 - in, 0 -out  IODIRA
	uint8_t value;

	uint8_t bit;	  			 // n-bit 0...15 for chip;
	uint8_t on;		 			 // default 1, 
	uint8_t ipol;    			 // in port, invert 0 - no, 1- yes
	uint8_t default_value;

	struct nFUNCTIONS *pin_functions;
}; 

#define MAX_PIN 64

struct nGPIO{

	struct nPIN *pin[MAX_PIN];
	
	int (*set)(uint8_t , uint8_t );	
	int (*get)(uint8_t);
	uint8_t (*get_async)(uint8_t);	
	int (*direction)(uint8_t,uint8_t);	
	struct nPIN *((*info_pin)(uint8_t));
	int (*init)(uint8_t );	  
	
	
	
};


//  Определеяем структуру GPIO станка  

extern struct nGPIO Stanok_gpio;

int Stanok_gpio_set_pin (uint8_t pin, uint8_t value){Stanok_gpio.pin[pin]->pin_functions->set_nPIN(pin,value);};
int Stanok_gpio_get_pin (uint8_t pin){ Stanok_gpio.pin[pin]->pin_functions->get_nPIN(pin);};
uint8_t Stanok_gpio_get_async (uint8_t pin){Stanok_gpio.pin[pin]->pin_functions->get_nPIN_async(pin);};
int Stanok_gpio_direction_pin (uint8_t pin, uint8_t value){Stanok_gpio.pin[pin]->pin_functions->direction_nPIN(pin,value);};
int Stanok_gpio_init_pin (uint8_t pin){ Stanok_gpio.pin[pin]->pin_functions->init_nPIN(pin);};
struct nPIN *Stanok_gpio_info_pin(uint8_t pin){return Stanok_gpio.pin[pin];};

/*
struct nGPIO Stanok_gpio ={
* 
    .pin {
		....
		....
      },
	.set = Stanok_gpio_set_pin,
	.get = Stanok_gpio_get_pin,
	.get_async = Stanok_gpio_get_async,
	.direction = Stanok_gpio_direction_pin,
	.init = Stanok_gpio_init_pin,
	.info_pin =  Stanok_gpio_info_pin,
	
};
*/


//////// Управление оборудованием ////////////////

extern int Stanok_enabled_device (uint8_t pin){	
	return Stanok_gpio.set(pin, Stanok_gpio.pin[pin]->on);
};

extern int Stanok_disabled_device (uint8_t pin){	
	return Stanok_gpio.set(pin,!(Stanok_gpio.pin[pin]->on));
};

extern void Stanok_init_device (){
	for (uint8_t i=0; Stanok_gpio.pin[i] != NULL; i++){Stanok_gpio.init(i);};
};



////////  микросхемы I2C mcp 230xx (17-18) ///////  

int mcp_set_pin(uint8_t pin ,uint8_t value){
	Stanok_gpio.pin[pin]->value = value;	
	return mcp23017_registr_set_bit (Stanok_gpio.pin[pin]->chip,GPIOA,Stanok_gpio.pin[pin]->bit,Stanok_gpio.pin[pin]->value);
};

int mcp_get_pin (uint8_t pin){
	return Stanok_gpio.pin[pin]->value = mcp23017_registr_read_bit(Stanok_gpio.pin[pin]->chip,GPIOA,Stanok_gpio.pin[pin]->bit);
};

int mcp_direction (uint8_t pin, uint8_t value){	
	Stanok_gpio.pin[pin]->direction = value;
	return mcp23017_registr_set_bit (Stanok_gpio.pin[pin]->chip,GPPUA,Stanok_gpio.pin[pin]->bit,Stanok_gpio.pin[pin]->direction);
};

int mcp_init(uint8_t pin){	

    mcp23017_set_mode(Stanok_gpio.pin[pin]->chip,Stanok_gpio.pin[pin]->bit,Stanok_gpio.pin[pin]->direction); //0-out 1-in
	
	if(Stanok_gpio.pin[pin]->direction&1){
						 mcp23017_set_pullUP(Stanok_gpio.pin[pin]->chip,Stanok_gpio.pin[pin]->bit,Stanok_gpio.pin[pin]->default_value);
						 mcp23017_set_IPOL(Stanok_gpio.pin[pin]->chip,Stanok_gpio.pin[pin]->bit,Stanok_gpio.pin[pin]->ipol);
						 return Stanok_gpio.pin[pin]->value = mcp23017_read_pin_bit(Stanok_gpio.pin[pin]->chip,Stanok_gpio.pin[pin]->bit); 
						 };
						 
	Stanok_gpio.pin[pin]->value = Stanok_gpio.pin[pin]->default_value;					 	

	return mcp23017_set_pin_bit (Stanok_gpio.pin[pin]->chip,Stanok_gpio.pin[pin]->bit,Stanok_gpio.pin[pin]->value);
};

uint8_t mcp_get_async (uint8_t pin){
	
	MCP23017 *chip;
	chip = Stanok_gpio.pin[pin]->chip;
	
	
	uint8_t bit = Stanok_gpio.pin[pin]->bit;
	uint8_t status_bit = 0b00000001 << (bit&0x7);// создаем маску бита

	// example 0...7 GPIOA  8...15 GPIOB		
	uint8_t registr = GPIOA | ((bit&0x08)>>3); 
	
	// Информацию мы получаем из регистров которые лежат в структуре сhip _mcp23017
	// при обмене данных с микросхемой, запись или чтении - эти регистры обновляются
    
	uint8_t per = chip->registr[registr];
	return Stanok_gpio.pin[pin]->value = (per&status_bit) >> (bit&0x7);
};



struct nFUNCTIONS mcp230xx = {
	
	.set_nPIN = mcp_set_pin,
	.get_nPIN = mcp_get_pin,
	.direction_nPIN = mcp_direction,
	.init_nPIN = mcp_init,
	.get_nPIN_async = mcp_get_async,
	};

	
////////// UAPI gpio Linux ////////////////	
	

// добавить позже	









