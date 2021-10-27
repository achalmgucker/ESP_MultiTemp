#include <Arduino.h>
#include "network.h"

// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "mcp23009.h"

#include "temps.h"

#define SCREEN_WIDTH 128 // OLED display width, in pi xels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 0
#define TEMPERATURE_PRECISION 11

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
SingleTemp Temps[6];

// Assign address manually. The addresses below will need to be changed
// to valid device addresses on your bus. Device address can be retrieved
// by using either oneWire.search(deviceAddress) or individually via
// sensors.getAddress(deviceAddress, index)
// DeviceAddress insideThermometer = { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 };
// DeviceAddress outsideThermometer   = { 0x28, 0x3F, 0x1C, 0x31, 0x2, 0x0, 0x0, 0x2 };

void printResolution(DeviceAddress deviceAddress);
void printAddress(DeviceAddress deviceAddress);
void activatePorts(unsigned int mask, bool state);


MCP23009IoAbstraction *expand;

Network myNet;

void setup(void)
{
  // start serial port
  Serial.begin(115200);
  Serial.println("Greenhouse WL");

  myNet.Setup();

 // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  delay(50);
  digitalWrite(2, 1);
  delay(100);

  expand = new MCP23009IoAbstraction(0x20, Mcp23xInterruptMode::NOT_ENABLED, -1, &Wire);
  for (int i=0; i<7; i++)
    expand->pinDirection(i, INPUT_PULLUP);
  expand->pinDirection(7, OUTPUT);
  
  expand->writePort(0, 0xFF);
  expand->runLoop();
  delay(50);

// Start up the library
  sensors.begin();

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  // Search for devices on the bus and assign based on an index. Ideally,
  // you would do this to initially discover addresses on the bus and then
  // use those addresses and manually assign them (see above) once you know
  // the devices on your bus (and assuming they don't change).
  //
  // method 1: by index
  // if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  // if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1");

  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices,
  // or you have already retrieved all of them. It might be a good idea to
  // check the CRC to make sure you didn't get garbage. The order is
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  for (int port = 0; port < 6; port++)
  {
    activatePorts(1 << port, false);
    oneWire.reset_search();
    // assigns the first address found
    bool b = oneWire.search(Temps[port].addr);
    Temps[port].active = b;
    Temps[port].port = port;
    Temps[port].temp = -127.0F;
    // set the resolution to 9 bit per device
    if (b)
      sensors.setResolution(Temps[port].addr, TEMPERATURE_PRECISION);
  }
  activatePorts(0, true);

  // show the addresses we found on the bus
  for (int port = 0; port < 6; port++)
  {
    Serial.print("Device ");
    Serial.print(port, 2);
    Serial.print(" address: ");
    if (Temps[port].active)
      printAddress(Temps[port].addr);
    else
      Serial.println("----");

  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(1000); // Pause for 1 second

  // Clear the buffer
  display.clearDisplay();
  display.display();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  display.setRotation(1);

}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}

// main function to print information about a device
void ShowTemp(int pos, DeviceAddress deviceAddress)
{
  // rot=0
  // int x = 32+(pos & 0x01)*32;
  // int y = 8*(pos / 2)+20;
  // rot=1
  int x = 20; //+(pos & 0x01)*32;
  int y = 9*(pos / 1)+32;
  display.fillRect(x, y, 48, 8, 0);
  display.setCursor(x, y);     // Start at top-left corner

  float tempC = sensors.getTempC(deviceAddress);
  display.print(pos+1);
  display.print(' ');
  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    display.println(F("----"));
  }
  else
  {
    display.print(tempC, 1);
  }
  display.display();
}

void ShowTemp(int pos, SingleTemp sglTmp)
{
  // rot=0
  // int x = 32+(pos & 0x01)*32;
  // int y = 8*(pos / 2)+20;
  // rot=1
  int x = 20; //+(pos & 0x01)*32;
  int y = 9*(pos / 1)+32;
  display.fillRect(x, y, 48, 8, 0);
  display.setCursor(x, y);     // Start at top-left corner

  float tempC = sglTmp.temp;
  display.print(pos+1);
  display.print(' ');
  if(!sglTmp.active || tempC == DEVICE_DISCONNECTED_C) 
  {
    display.println(F("----"));
  }
  else
  {
    display.print(tempC, 1);
  }
  display.display();
}

unsigned long previousTime = millis();

const unsigned long interval = 1000;
bool state = false;

void activatePorts(unsigned int mask, bool state)
{
  // active bit = activated port
  for (int i=0; i<7; i++)
  {
    expand->pinDirection(i, (mask & 0x01) ? INPUT_PULLUP : OUTPUT);
    mask >>= 1;
  }
  expand->writePort(0, state ? 0 : 0x80);
  expand->runLoop();
}

/*
   Main function, calls the temperatures in a loop.
*/
void loop(void)
{
  myNet.Loop();

  unsigned long diff = millis() - previousTime;
  if(diff > interval) {
    previousTime += diff;
    state = !state;
  }

  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  activatePorts(0x7F, state);
  sensors.requestTemperatures();
  delay(20);

  activatePorts(0, state);

  for (int port = 0; port < 6; port++)
  {
    SingleTemp *p = &Temps[port];
    if (p->active)
    {
      activatePorts(1 << port, state);
      p->temp = sensors.getTempC(p->addr);
    }
  }

  for (int port = 0; port < 6; port++)
  {
    ShowTemp(port, Temps[port]);
  }
}
