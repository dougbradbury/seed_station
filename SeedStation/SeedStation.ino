/********************************************************
 * PID RelayOutput Example
 * Same as basic example, except that this time, the output
 * is going to a digital pin which (we presume) is controlling
 * a relay.  the pid is designed to Output an analog value,
 * but the relay can only be On/Off.
 *
 *   to connect them together we use "time proportioning
 * control"  it's essentially a really slow version of PWM.
 * first we decide on a window size (5000mS say.) we then
 * set the pid to adjust its output between 0 and that window
 * size.  lastly, we add some logic that translates the PID
 * output into "Relay On Time" with the remainder of the
 * window being "Relay Off Time"
 ********************************************************/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>
#define HeatMatRelay 6
#define OneWireBus 3

/**************************************
*  PID
**************************************/
double Setpoint, Input, Output;
PID myPID(&Input, &Output, &Setpoint,2,5,1, DIRECT);
int WindowSize = 5000;
unsigned long windowStartTime;

/**************************************
*  TEMP SENSOR
**************************************/
OneWire oneWire(OneWireBus);
DallasTemperature sensors(&oneWire);


/**************************************
*  RELAY CONTROL
**************************************/
void setRelayState();

// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html
DeviceAddress seedThermometer = { 0x28, 0x94, 0xE2, 0xDF, 0x02, 0x00, 0x00, 0xFE };
void setup()
{
  windowStartTime = millis();
  Setpoint = 100;
  myPID.SetOutputLimits(0, WindowSize);
  myPID.SetMode(AUTOMATIC);

  sensors.begin();
  sensors.setResolution(seedThermometer , 10);
}

void loop()
{
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(seedThermometer);
  if (tempC == -127.00) {
    Serial.print("Error getting temperature");
  } else {
    Input = tempC;
  }

  myPID.Compute();
  setRelayState();
}

void setRelayState()
{
  if(millis() - windowStartTime>WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  if(Output < millis() - windowStartTime) digitalWrite(HeatMatRelay,HIGH);
  else digitalWrite(HeatMatRelay,LOW);

}
