
 /* To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 * File:   main.c
 * Author: Aleksey Mamontov
 * Created on 24 июня 2018 г., 23:05
 */

////////////// 8bit mode (IOCON.BANK = 1) ////////////////////////////

#define IODIRA_8bit 0x00
#define IODIRB_8bit 0x10
#define IPOLA_8bit  0x01
#define IPOLB_8bit  0x11
#define GPINTENA_8bit 0x02
#define GPINTENB_8bit 0x12
#define DEFVALA_8bit 0x03
#define DEFVALB_8bit 0x13
#define INTCONA_8bit 0x04
#define INTCONB_8bit 0x14
#define IOCON_8bit 0x05
#define IOCON2_8bit 0x15
#define GPPUA_8bit 0x06
#define GPPUB_8bit 0x016
#define INTFA_8bit 0x07
#define INTFB_8bit 0x017
#define INTCAPA_8bit 0x08
#define INTCAPB_8bit 0x18
#define GPIOA_8bit 0x09
#define GPIOB_8bit 0x19
#define OLATA_8bit 0x0a
#define OLATB_8bit 0x1a


///////////////////// 16 bit mode  (IOCON.BANK = 0) ////////////////////

#define IODIRA 0x00 //  регистры указывают навправление входа и выхода для 16 портов
#define IODIRB 0x01 //  bit =1 - вход , bit = 0 выход, IODIRA - порты А0 - А7, ODIRB порты B0-В7

#define IPOLA  0x02 //  регистры отвечают за инверсию входных данных,  для портов  GPIOA - GPIOB
#define IPOLB  0x03 //  которые выставлены как вход (bit=1)  1 -> 0 или 0 ->1 :   (bit=0) 1->1 или 0->0

#define GPINTENA 0x04 //    Выставив  bit=1 мы определяем что данный порт может быть внешним прерыванием
#define GPINTENB 0x05 //    Прерывание происходит при смене напряжения.

#define DEFVALA 0x06 // В данных регистрах мы указываем при каком условии происходит прерывание
#define DEFVALB 0x07 // например bit=0 - то прерывание произойдет при  появление 1 (+3.3 до +5V)  на данном порту
                                    //  или bit=1 - тогда когда на порту появится 0 (GND,VSS) произойдет прерывание

#define INTCONA 0x08 // Если в этом регистре мы выставляем bit = 1 , то  определения для прерываний
#define INTCONB 0x09 // заданные в DEFVALA , DEFVAB игнорируются и прерывание происходит при условии
                                    // смены уровней тоесть 0 -> 1 прерывание  1-> 0 прерывание.

#define GPPUA 0x0c  // Подтягивать порты которые как входы к питанию через 100 кОм резистор
#define GPPUB 0x0d  // 1 - подтягивать , 0 - не подтягивать

#define INTFA 0x0e // Помогает определить на какой ножке произошло прерывание,  по умолчанию во всем
#define INTFB 0x0f  // регистре 0, произошло прерывание на порту в соответствующем разряде  появится 1

#define INTCAPA 0x10 // Регистр защелка, после прерывания происходит фиксация всех значений на портах на момент
#define INTCAPB 0x11 // прерывания,

#define GPIOA 0x12 // В этом регистре хранятся значения  на портах выставленных как входы
#define GPIOB 0x13 // c помощью регистров IODIRA , IODIRB

#define OLATA 0x14 // В этом регистре выставляются значения на портах выбранными как выходы
#define OLATB 0x15 // c помощью регистров IODIRA , IODIRB

#define IOCON 0x0A     // Управляющие регистры дублируют друг друга.
#define IOCON2 0x0B  //
/*
bit 7  (BANK)  0 - 16 bit  представление регистров
                       1 -   8 bit представление регистров

bit 6 (Mirror)  0 -  выходы микросхемы INTA и INTB отвечают каждая за свои 8 портов
                       1-   выходы дублируют друг друга

bit5 (SEQOP)  0 -  при записи данных -  счетчик адресс  регистров увеличивается
                        1 -  при записи данных - адресс регистров не изменяется
                        //8 битном режиме// - в один регистр
                        //16 битном режиме// - в два регистра последовательно
                                                             тоесть -> OLATA -> OLATB -> OLATA ->OLTAB

bit4 (DISSLW)  0 -  помочь ведущему на линии опускать и поднимать ногу на линии
                       1 - не помогать ))

bit3                -  для микросхемы MCP23S17 (SPI)

bit2 (ODR)      0  - ножки микросхемы INTA и INTB - цифровые   0 или 1
                      1 -  ножки микросхемы INTA и INTB - транзисторный выход (открытый коллетор)
                                ногу к резистору 10кОМ и к + (VDD),

bit1  (iNTPOL)  0 -  вслучаи прерывания активный сигнал 0, простой 1
                      1 -  вслучаи прерывания активный сигнал 1 , простой  0
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/i2c-dev.h>  // struct i2c_msg
#include <sys/ioctl.h>
#include <linux/i2c.h> // struct i2c_rdwr_ioctl_data

#define MAX_msg 10

typedef struct {
char 		*i2_bus;
__int16_t 	address;
 int      	chip_open;
 int       	bus_open;
__int8_t 	registr[0x16];
__int8_t 	buf[8192];
__int8_t 	lock_chip;
struct i2c_msg msg [MAX_msg];
struct i2c_rdwr_ioctl_data rdwr;

}MCP23017;

// example
//MCP23017 chip1 = {"/dev/i2c-0",0x20};
//MCP23017 chip2 = {"/dev/i2c-0",0x24};



//-------------------------------------  open i2C bus------------------------------------------------------//

int mcp23017_open_i2c(MCP23017 *chip){

        chip->bus_open = open( chip->i2_bus , O_RDWR);

        return chip->bus_open; };

//-------------------------------------  close i2c bus   ---------------------------------------------------//

int mcp23017_close_i2c (MCP23017 *chip){

    int chip_close = close(chip->bus_open);
        if (chip_close <0) perror("mcp23017 error close() I2C  ");

    return chip_close;};

//-------------------------------------- check open chip for  read , write --------------------------------//

int mcp23017_open_read_write (MCP23017 *chip){

    if ( mcp23017_open_i2c (chip) < 0) {perror("mcp23017 error  open() I2C; ");
                                                            return chip->bus_open;}            // шины нет, выходим с ошибкой

        chip->chip_open = ioctl( chip->bus_open, I2C_SLAVE,chip->address);
            if ( chip->chip_open < 0 ) {perror("Устройство на шине I2C не найдено: ");
                                                    mcp23017_close_i2c (chip);
                                                    };

    return chip->chip_open;                                                              // устройство есть на шине, выходим
};


//------------------------------------------ write bytes 8bit------------------------------------------//

__int8_t mcp23017_write_byte (MCP23017 *chip , __int8_t registr, __int8_t value){

 static __int8_t buf[2];

buf[0] = registr;                                           // buf[0]  - номер  регистра
buf[1] = value;                                             // buf[1]  - что писать в этот регистр

 if (mcp23017_open_i2c (chip) < 0)  return chip->bus_open;

        struct i2c_msg msg [1] = {
                                                {chip->address, 0, 2, buf }                   //выставить адрес и записать
                                                };
        struct i2c_rdwr_ioctl_data rdwr = {msg, 1 };

            int chip_write  = ioctl(chip->bus_open, I2C_RDWR, &rdwr );
            if ( chip_write < 0 ) perror("I2C erorr rdwr");


  if ( mcp23017_close_i2c (chip) < 0){ perror("I2C erorr close() "); return -1;};

chip->registr[registr] = value;

 return  chip_write;}


///----------------------------------------read bytes 8bit ------------------------------------------------//

__int8_t mcp23017_read_byte (MCP23017 *chip, __int8_t registr){

   if (mcp23017_open_i2c (chip) < 0)  return chip->bus_open;



    static __int8_t buf[1];
    buf[0]= registr;                                                 //выставить регистр который читаем

    struct i2c_msg msg [2] = {
                               {chip->address, 0, 1, buf },          //выставить адрес (методом записи)
                               {chip->address, I2C_M_RD, 1, buf }    //прочитать данные в буфер
                                            };

   struct i2c_rdwr_ioctl_data rdwr = {msg, 2 };                     //поместить массив  блоков (не больше 32 байт в блоке)
                                                                    // структуру (на все дается 8192 байт)

        if ( ioctl(chip->bus_open, I2C_RDWR, &rdwr ) < 0 )
                                    { perror("I2C erorr rdwr");}
                                    else {chip->registr[registr] = buf[0];}  //  при ок сохранить даннные в массив

  if ( mcp23017_close_i2c (chip) < 0){ perror("I2C erorr close() ");
                                                           return -1;}


return chip->registr[registr];};

//--------------------------------- read bit -----------------------------------------------------





//--------------------------------- set bit -------------------------------------------------------



//-----------------------------------------------------------------------------------------------------





