# nodemcu-simplewifisensor
Very simple and easy to assemble NodeMCU (ESP8266) WIFI temperature sensor. It can support one or several DS18B20 (or compatible Dallas 1-Wire sensor) temperature sensors and provide JSON via local webserver. Can be used in simple home automation/monitoring projects.


# Features
- Multiple DS18B20 sensors support (up to 10), automatic detection. 
- Optional support for onboard 0.91' 128x32 OLED display 
- Connects to existing WIFI network (ssid, password need to be configured)
- Local HTTP server. Listen to 80 port by default. Answer with JSON packet

# Hardware requirenments
- NodeMCU board (any compatible with Arduino IDE)
- One or several DS18B20 or compatible Dallas temperature sensor. 4.7 KOhm resistor between DQ and VDD lines required
- Existing WIFI network to work with


# Software requirenments
- Arduino IDE (tested on 1.8x)
- ESP8266 Arduino IDE stack

# Arduino Libraries
- DallasTemperature by MilesBurton, ... (https://github.com/milesburton/Arduino-Temperature-Control-Library)
- One Wire by Jim Studt,... (https://www.pjrc.com/teensy/td_libs_OneWire.html)
- AdaFruit SSD 1306

# Webserver
Example request: http://esp8266.local/
Example answer: {  "rssi": "-72",  "version": "1.000", "sensors": {"124aba839a234441": "31.25" , "38a78af391849992": "21.81" } }

rssi - WIFI signal quality (less than -70 is ok)
sensors - hash array where key is sensor unique id and value is measured temperature in celcius


