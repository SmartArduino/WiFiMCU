
# **WiFiMCU** #
   WiFiMCU is developed based on EMW3165 module produced by [MXCHIP.INC](http://www.mxchip.com/). A Lua interpreter is builded inside with hardware support. A light weight file system and socket protocols can help to realize IoT development easily and quickly. Basically, you can load this on your device and then run Lua scripts on it with nothing more than a terminal connection. Enjoy it!

#####Cortex-M4 microcotroller<br/>
- STM32F411CE<br/>
- 100MHz,Cortex-M4 core<br/>
- 2M bytes of SPI flash and 512K bytes of on-chip flash<br/>
- 128K bytes of RAM<br/>
- Operation Temperature：-30℃ ~ +85℃<br/>

#####Multi-Interface<br/>
- 17 GPIO Pin<br/>
- 3 UARTs<br/>
- ADC(5)/SPI(1)/I2C(1)/USB(1)<br/>
- SWD debug interface<br/>
- 11 PWMs<br/>

#####Broadcom IEEE 802.11 b/g/n RF Chip<br/>
- Supports 802.11 b/g/n<br/>
- WEP,WPA/WPA2,PSK/Enterprise<br/>
- 16.5dBm@11b,14.5dBm@11g,13.5dBm@11n<br/>
- Receiver sensitivity：-87 dBm<br/>
- Station,Soft AP and Station+Soft AP<br/>
- CE, FCC suitable<br/>

#Overview
- Based on Lua 5.1.4 (package, string, table, math modules)<br/>
- Build-in modules: mcu,gpio, timer, wifi, net, file, pwm, uart, adc.<br/>
- Modules to be builded: spi, i2c, 1-wire, bit, mqtt...<br/>
- Integer version provided.<br/>

#GPIO table
<a id="gpio pin table"></a>
<table>
  <tr>
    <th scope="col">GPIO index</th><th scope="col">Alternative Function</th><th scope="col">Discription</th>
  </tr>
  <tr>
    <td>D0</td><td>GPIO/BOOT</td><td>WiFiMCU would enter into Bootloader Mode, if D0 goes to LOW</td>
  </tr>
  <tr>
    <td>D1</td><td>GPIO/PWM/ADC</td><td></td>
  </tr>
  <tr>
    <td>D2</td><td>GPIO</td><td></td>
  </tr>
  <tr>
    <td>D3</td><td>GPIO/PWM</td><td></td>
  </tr>
  <tr>
    <td>D4</td><td>GPIO</td><td></td>
  </tr>
  <tr>
    <td>D5</td><td>GPIO</td><td>SWD Flash Programming Pin: swclk</td>
  </tr>
  <tr>
    <td>D6</td><td>GPIO</td><td>SWD Flash Programming Pin: swdio</td>
  </tr>
  <tr>
    <td>D7</td><td>GPIO</td><td></td>
  </tr>
  <tr>
    <td>D8</td><td>GPIO/PWM</td><td>Uart1 rx pin: RX1</td>
  </tr>
  <tr>
    <td>D9</td><td>GPIO/PWM</td><td>Uart1 tx pin: TX1</td>
  </tr>
  <tr>
    <td>D10</td><td>GPIO/PWM</td><td>I2C interface: SCL</td>
  </tr>
  <tr>
    <td>D11</td><td>GPIO/PWM</td><td>I2C interface: SDA</td>
  </tr>
  <tr>
    <td>D12</td><td>GPIO/PWM</td><td></td>
  </tr>
  <tr>
    <td>D13</td><td>GPIO/PWM/ADC</td><td></td>
  </tr>
  <tr>
    <td>D14</td><td>GPIO/PWM</td><td></td>
  </tr>
  <tr>
    <td>D15</td><td>GPIO/PWM/ADC</td><td></td>
  </tr>
  <tr>
    <td>D16</td><td>GPIO/PWM/ADC</td><td></td>
  </tr>
  <tr>
    <td>D17</td><td>GPIO/ADC</td><td>A LED is connected on WiFiMCU board</td>
  </tr>
</table>

#Program demos
####Setup a AP

```lua
    cfg={ssid='Doit_3165',pwd=''}
    wifi.startap(cfg)
```
####WebServer

```lua
   skt = net.new(net.TCP,net.SERVER) 
   net.on(skt,"accept",function(clt,ip,port) 
   print("accept ip:"..ip.." port:"..port.." clt:"..clt) 
   net.send(clt,[[HTTP/1.1 200 OK
   Server: WiFiMCU
   Content-Type:text/html
   Content-Length: 28
   Connection: close
   
   
   <h1>Welcome to WiFiMCU!</h1>]])
   end)
   net.start(skt,80) 
```
####Connect Wireless Router

```lua
   print(wifi.sta.getip())  --check ip
   0.0.0.0
   cfg={ssid="Doit",pwd="123456789"} wifi.startsta(cfg) --sta mode connect
   print(wifi.sta.getip())  --check ip
   0.0.0.0
   print(wifi.sta.getip())  --check ip
   192.168.1.116 
```
####Connect Remote Server

```lua
   clt = net.new(net.TCP,net.CLIENT)
   net.on(clt,"dnsfound",function(clt,ip) print("dnsfound clt:"..clt.." ip:"..ip) end)
   net.on(clt,"connect",function(clt) print("connect:clt:"..clt) end)
   net.on(clt,"disconnect",function(clt) print("disconnect:clt:"..clt) end)
   net.on(clt,"receive",function(clt,data) print("receive:clt:"..clt.."data:"..data) end)
   net.start(clt,6579,"115.29.109.104")
```
####GPIO Operation

```lua
   gpio.mode(17,gpio.OUTPUT)
   gpio.toggle(17)
   gpio.write(17,gpio.HIGH)
   gpio.write(17,gpio.LOW)
   gpio.mode(17,gpio.INPUT)
   =gpio.read(17)
   1
   =gpio.read(17)
   0
```
####Timer

```lua
   function tmr_cb() print('tmr1 is called') end 
   tmr.start(1,1000,tmr_cb)
   tmr1 is called
   tmr1 is called
```
####File Operation

```lua
   file.open ("test.lua","w+")
   file.write("this is a test")
   file.close()
   file.open ("test.lua","r")
   data=file.read()
   print(data)
   this is a test
   file.close()
```
####Auto start

```lua
   file.open ("init.lua","w+")
   file.write("print('Hello WiFiMCU!')")
   file.close()
   mcu.reboot()
```

###Reference
[EMW3165 Introduction(Chinese)](https://github.com/SmartArduino/WiFiMCU/tree/master/Document)<br/>
[WiFiMCU SCH](https://github.com/SmartArduino/WiFiMCU/tree/master/Document)<br/>
[WiFiMCU Refecence](https://github.com/SmartArduino/WiFiMCU/tree/master/Document)<br/>
[WiFiMCU Tutorial](https://github.com/SmartArduino/WiFiMCU/tree/master/Document)<br/>

###Resource
Home site:www.wifimcu.com<br/>
discussion:www.emw3165.com<br/>
http://bbs.smartarduino.com<br/>
http://bbs.doit.am<br/>

###The IDE tool for wifimcu can be found here
https://github.com/SmartArduino/WiFiMCU-STUDIO

####Acknowledgements
Thanks to [eLua project](https://github.com/elua/elua),[NodeMCU project](https://github.com/nodemcu/nodemcu-firmware),[spiffs file system](https://github.com/pellepl/spiffs)<br/>

####More information please go to
www.wifimcu.com<br/>

####[Doctors of Intelligence & Technology](www.doit.am)
[WiFiMCU Dev Kit](http://www.smartarduino.com)

####Version log
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
