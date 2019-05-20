#include <Arduino.h>

// variables to check data inputs from sensor
int highInput= 0;
int lowInput = 0;

// variables to check if inputs are valid and sensor working accordingly
uint16_t dataCheck = 0;
uint16_t fDataCheck = 0;

// sensor variables
uint16_t PM1_CF1;
uint16_t PM2_5_CF1;
uint16_t PM10_CF1;
uint16_t PM1_Amb;
uint16_t PM2_5_Amb;
uint16_t PM10_Amb;
uint8_t errorCode;

void setup() {

  pinMode(SARA_ENABLE, OUTPUT);  
  digitalWrite(SARA_ENABLE, HIGH);
  
  pinMode(SARA_R4XX_TOGGLE, OUTPUT);
  digitalWrite(SARA_R4XX_TOGGLE, LOW);

    SerialUSB.begin(9600);
    while(!SerialUSB) {}
    
    Serial.begin(9600);
    while(!Serial) {}
    SerialUSB.println("Serial ports ready!");

   
}

void loop() {
    // This is for excel use only
   
    // This is where everything happens
    readData();

    // Some delay for safety purposes
    delay(100);
}

void readData() {
    // Wait for availability (should add a delay?)
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
          SerialUSB.print(PM1_CF1);
          SerialUSB.print(",");
          break;
        case 2:
          PM2_5_CF1 = lowInput + (highInput<<8);
          SerialUSB.print(PM2_5_CF1);
          SerialUSB.print(",");
          break;
        case 3:
          PM10_CF1 = lowInput + (highInput<<8);
          SerialUSB.print(PM10_CF1);
          SerialUSB.print(",");
          break;
        case 4:
          PM1_Amb = lowInput + (highInput<<8);
          SerialUSB.print(PM1_Amb);
          SerialUSB.print(",");
          break;
        case 5:
          PM2_5_Amb = lowInput + (highInput<<8);
          SerialUSB.print(PM2_5_Amb);
          SerialUSB.print(",");
          break;
        case 6:
          PM10_Amb = lowInput + (highInput<<8);
          SerialUSB.println(PM10_Amb);
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
    // Here you can do things with the data you got
    // e.g.: send to the receivers in BC68
    
    // First check for errors
    if(dataCheck != fDataCheck){
      SerialUSB.print("Error! Error code:");
      SerialUSB.print(errorCode);
      SerialUSB.print(';'); 
      SerialUSB.print(dataCheck); 
      SerialUSB.print(';'); 
      SerialUSB.print(fDataCheck);  
      SerialUSB.print('\n');           
    }

    // Secondly send the data if everything's fine
    // TODO: find out how to send the data to the BC68
    
    delay(700);  // higher delay will get you checksum errors      
    return;
}
