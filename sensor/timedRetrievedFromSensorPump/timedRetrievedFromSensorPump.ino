/*
  timedRetrievalFromSensor (PMS5003)

  Data retrieval from sensor according to the basis of aggregation 
  of air quality data according to the legislation in force. From 
  15 to 15 minutes, instant measures are taken and an average 
  of those is made. After 60 minutes, an average of those periodic 
  measurements within that hour is made and sent through NB-IoT to
  the server, and stored in a MongoDB database.

  The circuit:
  * SFF pins 12(TX) and 13(RX) connected to RX and TX of the sensor, 
  respectively.
  * SFF VCC and GND connected to IN+ and IN- of a 5V DC-DC converter,
  and Vout and GND of converter connect to Vin and GND of the sensor.

  Created 26 June 2019
  By Bernardo Amaral
  Modified 18 July 2019
  By Bernardo Amaral

*/
#include <Arduino.h>

// debug mode - uncomment this line to turn on debug mode
//#define DEBUG

#ifdef DEBUG
 #define DEBUG_PRINTLN(x)  SerialUSB.println(x)
 #define DEBUG_PRINT(x)  SerialUSB.print(x)
#else
 #define DEBUG_PRINTLN(x)
 #define DEBUG_PRINT(x)
#endif

#define SENSORLOCATION "av.liberdade"
#define vacuumPump 3 //arduino vacuumPump pin
#define hour 120 // 120
#define quarter 15 // 15
#define smallQuarter 1 // 1

// timer variables
unsigned long minuteStart, minuteCtr;
unsigned long quarterStart, quarterCtr;
unsigned long hourStart, hourCtr,twoHourStart, twoHourCtr;
const unsigned long minute = 60000;  // 1 minute in ms
const unsigned long serverOverhead = 1500; // server hourly overhead

// sensor variables
int highInput= 0; // check data inputs from sensor
int lowInput = 0;
uint16_t dataCheck = 0; // inputs are valid and sensor working accordingly
uint16_t fDataCheck = 0;
uint16_t PM1_CF1;   // values
uint16_t PM2_5_CF1;
uint16_t PM10_CF1;
uint16_t PM1_Amb;
uint16_t PM2_5_Amb;
uint16_t PM10_Amb;
uint8_t errorCode;

// avg calculation variables
float PM10_minute, PM2_5_minute;
float PM10_hour, PM2_5_hour;

//loop variables
int i = 0; 
int j = 0;

// server token
String sens_id = "2";
String secretToken = "Tu.cNszz1)~MqSy_ok&mhZZ#FLZz8%";
String errorToken = "b!PKnFniJ~jbj*yG)j`qyo,vL!2uK{";

void setup() {
  // Turn the power to the SARA module on.
  // we have a powerswitch on board to switch the power to the SARA on/of when needed
  // in most applications we keep the power on all the time  
  pinMode(SARA_ENABLE, OUTPUT);  
  digitalWrite(SARA_ENABLE, HIGH);
  
  // Turn the nb-iot module on
  // The R4XX module has an on/off pin. You can toggle this pin or keep it low to
  // switch on the module
  pinMode(SARA_R4XX_TOGGLE, OUTPUT);
  digitalWrite(SARA_R4XX_TOGGLE, LOW);
  
#ifdef DEBUG
  SerialUSB.begin(9600);
  while(!SerialUSB) {}
#endif

  // connected to sensor
  Serial.begin(9600);
  while(!Serial) {}
  
  // modem serial port
  Serial1.begin(115200);
  while(!Serial1) {}
  
  DEBUG_PRINTLN("Serial ports ready!");

#ifndef DEBUG
  delay(30000); // sensor warm up time
#endif

  // Vacuum pump setup
  pinMode(vacuumPump, OUTPUT);
  digitalWrite(vacuumPump, HIGH);
  setupConnection();

}


void setupConnection() {
  
  Serial1.println("AT");
  while (Serial1.available()) {
    Serial1.read();
  }

  delay(500);
  
  Serial1.println("AT+CFUN=1");
  while (Serial1.available()) {
    Serial1.read();
  }

  delay(500);
  
  Serial1.println("AT+CGDCONT=1,\"IP\",\"\"");
  while (Serial1.available()) {
    Serial1.read();
  }

  delay(500);
 
  Serial1.println("AT+CGATT=1"); 
  if (Serial1.available()) {
    Serial1.read();
  }

  delay(500);
}

void loop() {
  
  twoHourStart = millis();
  twoHourCtr = twoHourStart;
  digitalWrite(vacuumPump, LOW);
  
  // Every 2 Hours, execute the following loop for 2 Hours
  while(twoHourCtr - twoHourStart < (minute * hour)){
    
    // reset measurement values
    PM10_hour=0.0;
    PM2_5_hour = 0.0; 
    j = 0;
    
    // hourly period
    hourStart = millis();
    hourCtr = hourStart;
    DEBUG_PRINTLN("Starting loop");
  
    //fifteen minute averages, according to APA, Qualar
    while(hourCtr - hourStart < (minute * quarter) - (serverOverhead * j)) {
      // TODO: kill hourly time deviation
      DEBUG_PRINTLN("Restarting hourly loop");
      
      i = 0;
      PM10_minute= 0.0;
      PM2_5_minute = 0.0;
      // take measures for one minute    
      minuteStart = millis();
      minuteCtr = minuteStart;
      
      while(minuteCtr - minuteStart < minute * smallQuarter) {
        // measurement function
        readData();
        // Validating test
        if (!(PM10_minute + 750 < PM10_Amb || PM2_5_minute + 750 < PM2_5_Amb)){
          PM10_minute = PM10_minute + PM10_Amb;
          PM2_5_minute = PM2_5_minute + PM2_5_Amb;
          i = i + 1; 
        }
        minuteCtr = millis();
      }
  
      // calculate average for the above minute
      PM10_hour = PM10_hour + (PM10_minute / i);
      PM2_5_hour = PM2_5_hour + (PM2_5_minute / i);
      j = j + 1;
  
      DEBUG_PRINTLN("This minute averages for PM2.5 and PM10: ");
      DEBUG_PRINT(PM2_5_minute / i);
      DEBUG_PRINT("\t");
      DEBUG_PRINTLN(PM10_minute / i);
      DEBUG_PRINTLN("This quarter averages for PM2.5 and PM10: ");
      DEBUG_PRINT(PM2_5_hour / j);
      DEBUG_PRINT("\t");
      DEBUG_PRINTLN(PM10_hour / j);
  
      // refresh hour counter
      hourCtr = millis();
    }
  
    // calculate quarter average
    PM10_hour = PM10_hour / j;
    PM2_5_hour = PM2_5_hour / j;
    
    // quarter period is over - send data to server
    DEBUG_PRINTLN("Sending data to server");
    sendToServer(0);

    // refresh twoHour counter
    twoHourCtr = millis();
  }
  
  digitalWrite(vacuumPump, HIGH);
  
  delay(7200000);
}


void readData() {

  while(Serial.available() < 32){
    delay(100);
  }
  
  // Requirements from Spreadsheet - first bits sent by the sensor every loop
  if (Serial.read() != 0x42) return;
  if (Serial.read() != 0x4D) return;
  if (Serial.read() != 0x00) return;
  if (Serial.read() != 0x1c) return;

  // the data sent by the sensor is checked at the end of every loop
  dataCheck = 0x42 + 0x4D + 0x00 + 0x1c;

  for(int i = 1; i < 15; i++){
    
    highInput = Serial.read();
    lowInput = Serial.read();
    dataCheck += highInput + lowInput;
    
    switch(i){
      case 1:
        PM1_CF1 = lowInput + (highInput<<8);
        break;
      case 2:
        PM2_5_CF1 = lowInput + (highInput<<8);
        break;
      case 3:
        PM10_CF1 = lowInput + (highInput<<8);
        break;
      case 4:
        PM1_Amb = lowInput + (highInput<<8);
        break;
      case 5:
        PM2_5_Amb = lowInput + (highInput<<8);
        DEBUG_PRINT("PM2.5: ");
        DEBUG_PRINT(PM2_5_Amb);
        DEBUG_PRINT(",");
        break;
      case 6:
        PM10_Amb = lowInput + (highInput<<8);
        DEBUG_PRINT(" PM10: ");
        DEBUG_PRINTLN(PM10_Amb);
        break;
      case 13:
        errorCode = lowInput;
        break;
      case 14:
        dataCheck = dataCheck - (highInput + lowInput);
        fDataCheck = lowInput + (highInput<<8);
        break;
      default:
        break;
    }
  }
  
  // First check for errors
  if(dataCheck != fDataCheck){ 
    // send error message to server
    sendToServer(1);

    DEBUG_PRINTLN("Error! Error code:");
    DEBUG_PRINT(errorCode);
    DEBUG_PRINT(';');
    DEBUG_PRINT(dataCheck);
    DEBUG_PRINT(';');
    DEBUG_PRINT(fDataCheck);
    DEBUG_PRINT('\n');

    //TODO: check if pump is on?
  }
  
  delay(700);  // higher delay will get you checksum errors
  return;
}


void sendToServer(int error) {
  
  // first check if modem is still attached to network
  Serial1.println("AT+CGATT=1");
  while (Serial1.available()){
    Serial1.read();  // TODO: if error
  }

  // open a socket, send udp packet and close socket
  delay(500);
 
  Serial1.println("AT+USOCR=17,1");
  while (Serial1.available()) {
    Serial1.read(); 
  }
 
  delay(500);
  String location = SENSORLOCATION;
  
  /*ursula: "146.193.48.22\",49161,"*/
  /*docean: "138.68.165.208\",5555,"*/
  
  if(error) {
    Serial1.println("AT+USOST=0,\"146.193.48.22\",49161,"+
    String(errorToken.length() + location.length() + 1) +
    ",\""+ errorToken + " " + location +
    "\"");
  } else {
    Serial1.println("AT+USOST=0,\"146.193.48.22\",49161,"+
    String(secretToken.length() + location.length() + String(PM10_hour).length() + String(PM2_5_hour).length() + sens_id.length()+ 4) +
    ",\""+ secretToken + " " + String(PM10_hour) + " " + String(PM2_5_hour) + " " + location + " " + sens_id +"\"");
  }
  
  while (Serial1.available()) {
    Serial1.read(); 
  }

  delay(500);
  
  Serial1.println("AT+USOCL=0");
  
  while (Serial1.available()) {
    Serial1.read();
  }

  return;
}
