// This example sends data to multiple variables to 
// Ubidots through HTTP protocol.

/****************************************
 * Include Libraries
 ****************************************/

#include <Ubidots.h>

/****************************************
 * Define Instances and Constants
 ****************************************/

const char * TOKEN = "BBFF-gSjxQK0S9ujN0Ol0MA1zbtQHDyGTpK";
const char * SSID = "UBIWIFI";
const char * PASSWORD = "clave123456789ubi";

Ubidots ubidots(TOKEN, UBI_HTTP);

/****************************************
 * Auxiliar Functions
 ****************************************/

//Put here your auxiliar functions


/****************************************
 * Main Functions
 ****************************************/

void setup() {
  Serial.begin(115200);
  ubidots.setDebug(true);  // Uncomment this line for printing debug messages
  ubidots.init(SSID, PASSWORD);

}


void loop() {
  float value1 = analogRead(A0);
  float value2 = analogRead(A1);
  float value3 = analogRead(A2);
  ubidots.add("Variable_Name_One", value1);  // Change for your variable name
  ubidots.add("Variable_Name_Two", value2);
  ubidots.add("Variable_Name_Three", value3);

  bool bufferSent = false;
  bufferSent = ubidots.send();  // Will send data to a device label that matches the device Id

  if(bufferSent){
    // Do something if values were sent properly
    Serial.println("Values sent by the device");
  }

  delay(10000);

}