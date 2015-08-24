
# **WiFiMCU** #
WiFiMCU is developed based on EMW3165 module produced by Mxchip.inc. A Lua interpreter is builded inside with hardware support. A light weight file system and socket protocols can help to realize IoT development easily and quickly. Basically, you can load this on your device and then run Lua scripts on it with nothing more than a terminal connection. Enjoy it!

####MCortex-M4 microcotroller
STM32F411CE
100MHz,Cortex-M4 core
2M bytes of SPI flash and 512K bytes of on-chip flash
128K bytes of RAM
Multi-Interface
17 GPIO Pin
3 UARTs
ADC(5)/SPI(1)/I2C(1)/USB(1)
SWD debug interface
11 PWM
Broadcom IEEE 802.11 b/g/n RF Chip
Supports 802.11 b/g/n
WEP,WPA/WPA2,PSK/Enterprise
16.5dBm@11b,14.5dBm@11g,13.5dBm@11n
Receiver sensitivity：-87 dBm
Station,Soft AP and Station+Soft AP
CE,  FCC  suitable
Operation Temperature：-30℃ ~ +85℃


The IDE tool for wifimcu can be found here:
https://github.com/SmartArduino/WiFiMCU-STUDIO

####Based on EMW3165, Lua 5.1.4, eLua, NodeMCU
Thanks to [eLua project](https://github.com/elua/elua),[NodeMCU project](https://github.com/nodemcu/nodemcu-firmware),[spiffs file system](https://github.com/pellepl/spiffs)<br/>

####More information please go to: www.wifimcu.com<br/>

Doctors of Intelligence & Technology (www.doit.am)(br/)
support@doit.am(br/)
[EMW3165](http://www.smartarduino.com)(br/)

####version log
v0.9.4@2015-8-24<br/>
repair wifi module bugs<br/>
v0.9.3@2015-8-18<br/>
change the bootloader ymodem.c<br/>
v0.9.2@2015-8-15<br/>
add uart/pwm/adc modules change<br/>
change net module<br/>
change the logo<br/>
v0.9.1@2015-7-26<br/>
initial publish<br/>
