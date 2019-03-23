# NanoPI-NEO-I2C
Сборка gcc -o example stanok.c -lrt

Nano Pi Neo согласно даташита чип H3  может работать по шине
i2c 400 kHz, 
в реальности в Linux может работать 

до 1.7Mhz c MCP23017
до 3.4Mhz c MCP23018 (open drain)

Для этого в дереве устройств надо выставить частоту ))

<code>
	
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
	clock-frequency = <1700000>;
	}; // 1.7 Mhz для MCP23017
	
	или clock-frequency = <3400000>; - for MCP23018
	
	  это работает , it's work !! ))
</code>

Работа с шиной I2C, а в частности с микросхемой MCP23017 до 1.7 Mhz
MCP23017.h

Пример экран TFT(480x320) 3.5 inch ili9486, интерфейс I2C (через MCP23017), можно подключить нескольких экранов
к одной плате Nano PI Neo.

Работа на частоте от 400 kHZ до 1,7 MHz через MCP23017.

<img src="/Datasheet/examplePanel_i2c_3_5_ili9486_small.jpg" width=400 >
<img src="/tft_i2c/Schematic_MCP23017-ili9486_Sheet-1_20190113144251.png" width=400 >


Подключение к MCP23017 нескольких чипов температуры MAX6675 можно до 6 штук (
температура считывается со всех одновременно.
Принципе количество можно увеличить до 14 штук одновременно - но зачем ))

Схема подключения - желательно через оптопары H11L1 или ADUM1201  

<img src="/Datasheet/example_i2c_MAX6675_4pcs_small.jpg" width=400 >
<img src="/thermo_i2c/Schematic_optoisilator_Sheet-1_20190113143345.png" width=400 >

Интересный чип TTP229-LSF  подключатся 16 сенсорных кнопок (даташит в папке), 
работает по i2C - даташите написано 400 kHz - в реальности работает и 1 Mhz,

...............

Из-за особенностей внутренней реализации MCP23017 и MCP23018 (или бага в последней), они ведут себя по разному при записи в один регистр по кругу (режим bit5 SEQOP = 1) , если писать напрямую в защелки регистр OLATA(B), то mcp23017 ведет себя адекватно и ожидаемо, пишет по кругу в данные регистры, но mcp23018 - сходит с ума )), записав первую пару значений, перескакивает на следующую пару регистров и пишет все остальные данные в эти регистры, а так как это последния пара регистров OLATA(B) - то все остальные данные попадают в первыую пару регистров IODIRA(B) - отвечающая за выставление направление портов - и в этоге становится весело и этот баг нигде не описан.          

*Поэтому наверно не стоит писать напрямую в регистры  OLATA(B) по кругу, для этого лучше работать c GPIOA(B), в итоге получается примерно адекватная реакция от MCP23018  - он исходя из логики описанной работы , записывает первую пару значений GPIOA(B), и затем все остальное в OLATA(B). примерно как то так))
....................
