# NanoPI-NEO-I2C
Nano Pi Neo согласно даташита чип H3  может работать по шине
i2c 400 kHz, в реальности в Linux может работать до 1.7Mhz c MCP23017

Для этого в дереве устройств надо выставить частоту ))


i2c@1c2ac00 {
			compatible = "allwinner,sun6i-a31-i2c";
			reg = <0x1c2ac00 0x400>;
			interrupts = <0x0 0x6 0x4>;
			clocks = <0x3 0x3b>;
			resets = <0x3 0x2e>;
			pinctrl-names = "default";
			pinctrl-0 = <0x17>;
			status = "okay";
			#address-cells = <0x1>;
			#size-cells = <0x0>;
			clock-frequency = <10000000>;
			};
			
			
			// 1 Mhz )) 
работа с шиной I2C в частности с микросхемой MCP23017  до 1.7 Mhz

TFT(480x320) 3.5 inch ili9486 or ili9481  for NanoPI NEO
Работа на частоте от 400 kHZ до 1,7 MHz через MCP23017.


интерфейс I2C (MCP23017) для нескольких экранов ili9486
на одной плате.

Подключение к MCP23017 нескольких чипов температуры MAX6675 можно до 6 штук (
температура считывается со всех одновременно.
Принципе количество можно увеличить до 14 штук одновременно - но зачем ))

Схема подключения - желательно через оптопары H11L1 или ADUM1201  



Интересный чип TTP229-LSF  подключатся 16 сенсорных кнопок (даташит в папке), 
работает по i2C - даташите написано 400 kHz - в реальности работает и 1 Mhz,

<img src="/Datasheet/examplePanel_i2c_3_5_ili9486_small.jpg" width=400 >

<img src="/Datasheet/example_i2c_MAX6675_4pcs_small.jpg width=400 >
