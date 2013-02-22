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
#define OneWireBus 3
OneWire oneWire(OneWireBus);
DallasTemperature sensors(&oneWire);
DeviceAddress seedThermometer = { 0x28, 0x5C, 0x8D, 0x3F, 0x03, 0x00, 0x00, 0x5C };

/**************************************
*  RELAY CONTROL
**************************************/
#define HeatMatRelay 6
void setRelayState();

/**************************************
*  SCHDEULE
**************************************/
// union value {
//   int i;
//   float f;
//   bool b;
// }
//
// struct event {
//   int hour;
//   int minute;
//   int seconds;
//   *(void function(value *))
//   value param;
// }
// schedule = [
//  [0, setTemp, 77]
//  [1, setTemp, 65]
// ]
//
//
void setup()
{
  Serial.begin(9600);

  windowStartTime = millis();
  Setpoint = 77.0f;
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
    Input = DallasTemperature::toFahrenheit(tempC);
    Serial.print(Input);
    Serial.print("\n");
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
  if(Output < millis() - windowStartTime) {
    digitalWrite(HeatMatRelay,HIGH);
    Serial.print("High");
  }
  else {
    digitalWrite(HeatMatRelay,LOW);
    Serial.print("Low");
  }
}
