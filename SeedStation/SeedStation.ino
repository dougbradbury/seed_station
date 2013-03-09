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
#include <PID_AutoTune_v0.h>

/**************************************
*  PID
**************************************/
double Setpoint, Input, Output;
double kp=2,ki=0.5,kd=2;
PID_ATune aTune(&Input, &Output);
PID myPID(&Input, &Output, &Setpoint, kp, ki, kd, DIRECT);
int WindowSize = 5000;
unsigned long windowStartTime;
void AutoTuneHelper(boolean start);
byte ATuneModeRemember=2;
boolean tuning = false;
void doPid();

/**************************************
*  TEMP SENSOR
**************************************/
#define OneWireBus 3
bool readTemp(double *);
OneWire oneWire(OneWireBus);
DallasTemperature sensors(&oneWire);
DeviceAddress seedThermometer = { 0x28, 0x5C, 0x8D, 0x3F, 0x03, 0x00, 0x00, 0x5C };

/**************************************
*  RELAY CONTROL
**************************************/
#define HeatMatRelay 5
void setRelayState();

void setup()
{
  pinMode(HeatMatRelay, OUTPUT);
  pinMode(OneWireBus, INPUT);
  digitalWrite(OneWireBus, HIGH);
  Serial.begin(9600);

  windowStartTime = millis();
  Setpoint = 72.0f;
  myPID.SetOutputLimits(0, WindowSize);
  myPID.SetMode(AUTOMATIC);

  sensors.begin();
  sensors.setResolution(seedThermometer , 10);
}

void loop()
{
  if (readTemp(&Input)) {
    doPid();
    //myPID.Compute();
    setRelayState();
  }
}

bool readTemp(double * temp)
{
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(seedThermometer);
  if (tempC == -127.00) {
    Serial.print("Error getting temperature");
    return false;
  }
  else {
    *temp = DallasTemperature::toFahrenheit(tempC);
    return true;
  }
}

void doPid()
{
  if(tuning)
  {
    byte val = (aTune.Runtime());
    if (val!=0)
    {
      tuning = false;
    }
    if(!tuning)
    { //we're done, set the tuning parameters
      kp = aTune.GetKp();
      ki = aTune.GetKi();
      kd = aTune.GetKd();
      Serial.print("Tuning Complete:\n");
      Serial.print("kp: ");
      Serial.print(kp);
      Serial.print("\nki: ");
      Serial.print(ki);
      Serial.print("\nkd: ");
      Serial.print(kd);
      Serial.print("\n");
      myPID.SetTunings(kp,ki,kd);
      AutoTuneHelper(false);
    }
  }
  else myPID.Compute();
}

void AutoTuneHelper(boolean start)
{
  if(start)
    ATuneModeRemember = myPID.GetMode();
  else
    myPID.SetMode(ATuneModeRemember);
}

void setRelayState()
{
  unsigned long now = millis();
  unsigned long timeInWindow = now - windowStartTime;
  if ( timeInWindow > WindowSize )
  {
    Serial.print(Output);
    Serial.print(" | ");
    Serial.print(Input);
    Serial.print("\n");
    windowStartTime += WindowSize;
    timeInWindow = now - windowStartTime;
  }

  if ( timeInWindow <= Output ) {
    digitalWrite(HeatMatRelay,HIGH);
  }
  else {
    digitalWrite(HeatMatRelay,LOW);
  }
}
