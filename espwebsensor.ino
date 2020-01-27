// ESP8266 NodeMCU simple multi temperature sensor
// display temperature from multiple DS18B20 sensors
// 4.7 - 10 KOm resistor required between DQ and VDD lines
// Optional I2C OLED 0.91' 128х32 display support
// Simple webserver on 80 port to return json state of sensor
// NODEMcu must be permanently connected
// max 10 sensors

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// configuration file wifi ssid, password
// set SSID, PASSWORD in config.h constants
#include "config.h"

// display config
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define USEDISPLAY false // OLED DISPLAY ENABLE

#define VERSION "1.000"
#define PORT 80 // webserver port
#define SENSORDQPIN 13 // pin for temperature sensor DS18B20 (GPIO 13 / D7)
#define LEDPIN 2 // pin for builtin led (GPIO num)

#define g_infomessage_x 10
#define g_infomessage_y 10
#define g_infomessage_y_2 20

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     16 // Reset pin # (or -1 if sharing Arduino reset pin)
#ifdef USEDISPLAY
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(SENSORDQPIN);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address


// global structure with sensor information
struct SENSORINFO
{
  String ipaddress;
  float temperatures[10];  // array with sensor temperatures
  String sensoraddr[10]; // array with sensor identifications
  int numberOfDevices; // Number of temperature devices found  
  int rssi = 0; //signal strength
} sensorInfo;

ESP8266WebServer server(PORT);

void resetDisplay()
{
  if (USEDISPLAY)
  {
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);
  }
}

void connectWIFI() {

  const char* ssid = STASSID;
  const char* password = STAPSK;
  Serial.print(F("Connecting to WIFI network: "));
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  if (USEDISPLAY)
  {
    resetDisplay();
    display.setCursor(g_infomessage_x, g_infomessage_y);
    display.print(F("Connecting...."));
    display.setCursor(g_infomessage_x, g_infomessage_y_2);
    display.print(ssid);
    display.display();
  }

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  sensorInfo.ipaddress = WiFi.localIP().toString();

  if (USEDISPLAY)
  {
    resetDisplay();
    display.setCursor(g_infomessage_x, g_infomessage_y);
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);
    display.print(F("Connected"));
    display.display();

  }
  delay(2000);
  // set LED PIN
  //digitalWrite(LEDPIN, HIGH);
}

void initializeSensors() {
  Serial.println(F("Initializing temperature sensors"));
  
  sensors.begin();
  
  // Grab a count of devices on the wire
  sensorInfo.numberOfDevices = sensors.getDeviceCount();

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensorInfo.numberOfDevices, DEC);
  Serial.println(" devices.");

  for (int i = 0; i < sensorInfo.numberOfDevices; i++) {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i)) {
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(tempDeviceAddress);
      sensorInfo.sensoraddr[i]= deviceAddressToString(tempDeviceAddress);
      Serial.println();
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}

// function to print a device address
String deviceAddressToString(DeviceAddress deviceAddress) {
  String out;
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) out+="0";
      out+=String(deviceAddress[i], HEX);
  }
  return out;
}

void initializeDisplay() {

  if (USEDISPLAY)
  {
    delay(100);
    Serial.println(F("Initializing display"));
  
  /*  pinMode(2, OUTPUT);
    digitalWrite(2, LOW);   // turn D2 low to reset OLED
    delay(50);
    digitalWrite(2, HIGH);    // while OLED is running, must set D2 in high
  */
  
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
      Serial.println(F("SSD1306 allocation failed"));
    }
    display.display();
    delay(2000); // Pause for 2 seconds
  }
}

void initializeServer() {
  Serial.println(F("Initializing server"));

  if (USEDISPLAY)
  {
    display.clearDisplay();
    display.setCursor(g_infomessage_x, g_infomessage_y);
    display.print(F("Setting server..."));
    display.display();
  }

  if (MDNS.begin(HOSTNAME)) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

}

// display sensor data (json string)
void  handleRoot()
{
  //char outBuffer[100];

  //Serial.print(F("RSSI: "));
  //Serial.println(sensorInfo.rssi);

  String outBuffer="{  \"rssi\": \""+ String(sensorInfo.rssi) + "\", ";
  outBuffer+=" \"version\": \""+String(VERSION)+"\", ";
  outBuffer+="\"sensors\": {";

  for (int i = 0; i < sensorInfo.numberOfDevices; i++) {
    // Search the wire for address      
    if (i>0) outBuffer +=", ";
    outBuffer+="\"" + sensorInfo.sensoraddr[i] +"\": ";
    outBuffer+="\""+String(sensorInfo.temperatures[i])+"\" ";
    
  }
  outBuffer+="} ";
  outBuffer+="}";

  Serial.print(F("Webrequest: "));
  Serial.println(outBuffer);
  server.send(200, "application/json", outBuffer);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  server.send(404, "text/plain", message);
}

// refresh temperature sensors
void refreshSensors()
{
  //Serial.println(F("Reading sensors"));
  sensors.requestTemperatures();
  
  // Loop through each device, print out temperature data
  for (int i = 0; i < sensorInfo.numberOfDevices; i++) {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i)) {

      // Output the device ID
    //  Serial.print("Temperature for device: ");
//      Serial.println(i, DEC);

      // Print the data
      float tempC = sensors.getTempC(tempDeviceAddress);
  //    Serial.print("Temp C: ");
      //Serial.println(tempC);

      sensorInfo.temperatures[i]=tempC;      
    }
  }


  // TODO: temperature sensor reading
  //sensorInfo.temp1 = random(50, 688) / 10;
  //sensorInfo.temp2 = random(50, 688) / 10;

  sensorInfo.rssi = WiFi.RSSI();

  
  //Serial.print(F("RSSI: "));
  //Serial.println(sensorInfo.rssi);
}

// refresh display information
void refreshDisplay()
{
  if (USEDISPLAY)
  {
    //Serial.println(F("Refreshing display..."));
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // first temp
    char buffer[100];
    sprintf(buffer, "T1: %.1f", sensorInfo.temperatures[0]);
    display.setCursor(0, 0);
    display.print(buffer);

    if(sensorInfo.numberOfDevices>1)
    {
     // second temp
     sprintf(buffer, "T2: %.1f", sensorInfo.temperatures[1]);
     display.setCursor(70, 0);
     display.print(buffer);
    }

    String quality = "";
    if (sensorInfo.rssi > -80) quality = "|";
    if (sensorInfo.rssi > -70) quality = "||";
    if (sensorInfo.rssi > -60) quality = "|||";
    if (sensorInfo.rssi > -40) quality = "||||";
    if (sensorInfo.rssi > -30) quality = "|||||";

    // ip addr
    sprintf(buffer, "%s %s", sensorInfo.ipaddress.c_str(), quality.c_str());
    display.setCursor(0, 25);
    display.print(buffer);

    // Graphs
    display.display();
  }
}


void setup() {

  //pinMode(LEDPIN, OUTPUT);
  //digitalWrite(LEDPIN, LOW);
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(3000);
  Serial.println("WIFI Temperature smart sensor");
  delay(3000);


  // connect to wifi network (as a client)
  
  initializeDisplay();
  connectWIFI();
  initializeSensors();
  initializeServer();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  refreshSensors();
  refreshDisplay();
  server.handleClient();
  MDNS.update();
}
