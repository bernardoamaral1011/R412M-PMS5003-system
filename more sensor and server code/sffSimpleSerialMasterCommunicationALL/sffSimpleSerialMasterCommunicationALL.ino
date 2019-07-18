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
  delay(1000);
  
  MODEM_STREAM.println("AT+CFUN=1");
  while (MODEM_STREAM.available()) {     
    DEBUG_STREAM.write(MODEM_STREAM.read());
  }

  delay(9000);
  
  MODEM_STREAM.println("AT+CGDCONT=1,\"IP\",\"\"");
  while (MODEM_STREAM.available()) {     
    DEBUG_STREAM.write(MODEM_STREAM.read());
  }

  delay(1000);
  
  MODEM_STREAM.println("AT+CGATT=1");
 
  //TODO: set timeout for when there's no network to attach to
  while (MODEM_STREAM.available()) {     
    DEBUG_STREAM.write(MODEM_STREAM.read());
  }
 
  delay(10000);
  
  MODEM_STREAM.println("AT+USOCR=17,1");
  while (MODEM_STREAM.available()) {     
    DEBUG_STREAM.write(MODEM_STREAM.read());
  }

  delay(10000);
  float hey = 4.13;
  String h = String(hey);
  String secretToken = "Tu.cNszz1)~MqSy_ok&mhZZ#FLZz8%";
  float PM2_5_hour = 435.432;
  float PM10_hour = 3000.50;

  /*at+usost=0,"10.1.1.1",49161,5,"hello"*/
  MODEM_STREAM.println("AT+USOST=0,\"138.68.165.208\",5555,"+
    String(5) +",\"hello\"");
  while (MODEM_STREAM.available()) {     
    DEBUG_STREAM.write(MODEM_STREAM.read());
  }
}
