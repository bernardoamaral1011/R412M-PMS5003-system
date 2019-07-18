#include <Arduino.h>

#define R4XX // Uncomment when you use the ublox R4XX module

#define DEBUG_STREAM SerialUSB
#define MODEM_STREAM Serial1

#if defined(R4XX)
unsigned long baud = 115200;  //start at 115200 allow the USB port to change the Baudrate
#else 
unsigned long baud = 9600;  //start at 9600 allow the USB port to change the Baudrate
#endif

void setup() 
{
  // Turn the power to the SARA module on.
  // we have a powerswitch on board to switch the power to the SARA on/of when needed
  // in most applications we keep the power on all the time  
  pinMode(SARA_ENABLE, OUTPUT);  
  digitalWrite(SARA_ENABLE, HIGH);

#ifdef R4XX
  // Turn the nb-iot module on
  // The R4XX module has an on/off pin. You can toggle this pin or keep it low to
  // switch on the module
  pinMode(SARA_R4XX_TOGGLE, OUTPUT);
  digitalWrite(SARA_R4XX_TOGGLE, LOW);
#endif
  
  // Start communication
  DEBUG_STREAM.begin(baud);
  MODEM_STREAM.begin(baud);
}

// Forward every message to the other serial
void loop() 
{
  delay(5000);
  DEBUG_STREAM.write("hello");
  MODEM_STREAM.println("at");
  while (!MODEM_STREAM.available()){
  
    MODEM_STREAM.println("at");
  }
  MODEM_STREAM.println("at");
  while (MODEM_STREAM.available())
  {     
    DEBUG_STREAM.write(MODEM_STREAM.read());
  }
  delay(5000);
  // check if the USB virtual serial wants a new baud rate
  // This will be used by the UEUpdater to flash new software
  if (DEBUG_STREAM.baud() != baud) {
    baud = DEBUG_STREAM.baud();
    MODEM_STREAM.begin(baud);
  }
}
