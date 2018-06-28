/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/*
 * File:   main.c
 * Author: aleksey
 *
 * Created on 17 июня 2018 г., 17:44
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

///////////////////// 16 bit mode  (IOCON.BANK = 0) ////////////////////
/* 
 регистры указывают навправление входа и выхода для 16 портов
 bit =1 - вход , bit = 0 выход, IODIRA - порты А0 - А7, ODIRB порты B0-В7
  */ 
#define IODIRA 0x00
#define IODIRB 0x01

/* регистры отвечают за инверсию входных данных,  для портов  GPIOA - GPIOB 
которые выставлены как вход (bit=1)  1 -> 0 или 0 ->1 :   (bit=0) 1->1 или 0->0
*/
#define IPOLA  0x02
#define IPOLB  0x03

/* 
 Выставив  bit=1 мы определяем что данный порт может быть внешним прерыванием //
 Прерывание происходит при смене напряжения.
*/
#define GPINTENA 0x04
#define GPINTENB 0x05

//В данных регистрах мы указываем при каком условии происходит прерывание
//например bit=0 - то прерывание произойдет при  появление 1 (+3.3 до +5V)  на данном порту
// или bit=1 - тогда когда на порту появится 0 (GND,VSS) произойдет прерывание
#define DEFVALA 0x06
#define DEFVALB 0x07

// Если в этом регистре мы выставляем bit = 1 , то  определения для прерываний 
// заданные в DEFVALA , DEFVAB игнорируются и прерывание происходит при условии
// смены уровней тоесть 0 -> 1 прерывание  1-> 0 прерывание.
#define INTCONA 0x08
#define INTCONB 0x09



/* Эти регистры управлюят работой микросхемы (дублируют друг друга -изменение в одном приводит в к изменению в другом).

bit 7  (BANK)  0 - 16 bit  представление регистров  
                     1 -   8 bit представление регистров   

bit6 (Mirror)   0 -  выходы микросхемы INTA и INTB отвечают каждая за свои 8 портов
                    1-   выходы дублируют друг друга
 
bit5 (SEQOP)  0 -  при записи данных -  счетчик адресс  регистров увеличивается  
                     1 -  при записи данных - адресс регистров не изменяется
                            8 битном режиме - в один регистр -> OLATA ->OLATA 
                            16 битном режиме - в два регистра последовательно
                               тоесть -> OLATA -> OLATB -> OLATA ->OLTAB 

bit4 (DISSLW)  0 -  помочь ведущему на линии опускать и поднимать ногу на линии
                      1 - не помогать ))
 
bit3                -  для микросхемы MCP23S17 (SPI) 
 
bit2 (ODR)      0  - ножки микросхемы INTA и INTB - цифровые   0 или 1
                      1 -  ножки микросхемы INTA и INTB - транщисторный выход (открытый коллетор)
                                ногу к резистору 10кОМ и к + (VDD),

bit1  (iNTPOL)  0 -  вслучаи прерывания активный сигнал 0, простой 1
                      1 -  вслучаи прерывания активный сигнал 1 , простой  0  
 */                           

#define IOCON 0x0A
#define IOCON2 0x0b

/*
 Подтягивать порты которые как входы к питанию через 100 кОм резистор
 1 - подтягивать , 0 - не подтягивать
 */
#define GPPUA 0x0c
#define GPPUB 0x0d
/*
 * Помогает определить на какой ножке произошло прерывание,  по умолчанию во всех
 * регистрах 0, произошло прерывание на порту в соответствующем разряде появится 1
 */
#define INTFA 0x0e
#define INTFB 0x0f
/*Регистр защелка, после прерывания происходит фиксация всех значений на портах на момент
 прерывания, 
 */
#define INTCAPA 0x10 
#define INTCAPB 0x11

/*
В этом регистре хранятся значения  на портах выставленных как входы 
 */
#define GPIOA 0x12
#define GPIOB 0x13
/*
 В этом регистре выставляются значения на портах которые выставлены как  выходы
  */
#define OLATA 0x14
#define OLATB 0x15

/*////////////// 8bit mode (IOCON.BANK = 1) ////////////////////////////

#define IODIRA 0x00
#define IODIRB 0x10
#define IPOLA  0x01
#define IPOLB  0x11
#define GPINTENA 0x02
#define GPINTENB 0x12
#define DEFVALA 0x03
#define DEFVALB 0x13
#define INTCONA 0x04
#define INTCONB 0x14
#define IOCON 0x05
//#define IOCON 0x15
#define GPPUA 0x06
#define GPPUB 0x016
#define INTFA 0x07
#define INTFB 0x017
#define INTCAPA 0x08 
#define INTCAPB 0x18
#define GPIOA 0x09
#define GPIOB 0x19
#define OLATA 0x0a
#define OLATB 0x1a
 
 */
#define TRUE  1
#define FALSE 0


typedef struct {
     char *i2_bus;
  __int8_t address;
  __int8_t registr[0x16];
 
}MCP23017;


MCP23017 chip1 = {"/dev/i2c-0",0x20};
MCP23017 chip2 = {"/dev/i2c-1",0x21};



int MCP23017_write_byte (MCP23017 *chip , __int8_t registr, __int8_t value){
  
 static __int8_t buf[2] = {0};
 static __int8_t res = TRUE ; 
  buf[0] = registr;
  buf[1] = value;
 
    int chip_open = open( chip->i2_bus , O_RDWR);  
    if (chip_open< 0) {perror(" Шина I2C не включена "); res = FALSE;return res;}
  
    if ( ioctl(chip_open, I2C_SLAVE, chip->address ) < 0) {
         perror("Устройство на шине I2C не найдено"); res = FALSE; goto exit;}
 
    if (write(chip_open, buf, 2) != 2) {
        perror("не удалось записать данные в микросхему");  res = FALSE; goto exit;}    // Выставляем в буфере buf[0] номер  регистра ,  buf[1]  - что писать в этот регистр
   
chip->registr[registr] = value;
   
exit: close(chip_open);

printf("\n chip  %s", chip ->i2_bus);
printf("\n address  %01x", chip->address);
printf("\n registr  %hhd", registr);
printf("\n value  %01x", value); 

return res;}


int MCP23017_read_byte (MCP23017 *chip , __int8_t registr){
  
 static __int8_t buf[2] = {0,0};
  buf[0] = registr;
 static __int8_t res = TRUE; 

 
    int chip_open = open( chip->i2_bus , O_RDWR);  
     if (chip_open< 0) {perror(" Шина I2C не включена "); res = FALSE;return res;}
  
    if ( ioctl(chip_open, I2C_SLAVE, chip->address ) < 0) {
         perror("Устройство на шине I2C не найдено"); res = FALSE; goto exit;}
 
    if (write(chip_open, buf, 1) != 1) {
        perror("не удалось выставить регистр в микросхеме");  res = FALSE; goto exit;}   // Выставляем в буфере buf[0] номер  регистра 
    
    if (read(chip_open, buf, 1) != 1) {
        perror("не удалось  прочитать данные из микросхемы"); res = FALSE; goto exit;}
    
   
chip->registr[registr] = buf[0];
   
exit: close(chip_open);

printf("\n chip  %s", chip ->i2_bus);
printf("\n address  %01x", chip->address);
printf("\n registr  %hhd", registr);
printf("\n value  %01x",  buf[0]); 

return res;}




int main(int argc, char** argv) {
 
    //тест
    
    MCP23017_write_byte (&chip1, IOCON, 0);
    MCP23017_write_byte (&chip1, IODIRA, 0);
    MCP23017_write_byte (&chip1, OLATA, 0xff);
    
    MCP23017_read_byte (&chip1,OLATA);
    MCP23017_read_byte (&chip1,IODIRB);
    
    
    
   

    
for(__int8_t i=0; i< 0x16; i++){
    
    printf("\n regist[%hhd] = %x   <------> regist[%hhd] = %x",i, chip1.registr[i],i, chip2.registr[i]);
    
};


sleep(1);


for (__int8_t  i=0; i<255; i++){


MCP23017_write_byte (&chip1, OLATA, i);

sleep(1);

};




return (EXIT_SUCCESS);
}

