
/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2021 mobizt
 *
*/
//GPS
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
static const int RXPin = 3, TXPin = 1;
static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

#include <TinyMPU6050.h>
#include "BLEDevice.h"
MPU6050 mpu (Wire);


#include <Arduino_JSON.h>

#include "SIM800L.h"

#define SIM800_RST_PIN     5
#define SIM800_PWKEY     4
#define SIM800_POWER_ON     23

// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

const char APN[] = "internet"; //"telemach.hr"
const char serialID[] = "blU4OAVa9XU7k6YNom2G";
const char CONTENT_TYPE[] = "application/json";

SIM800L* sim800l;

unsigned long dataMillis = 0;

//za ble
static BLEAddress *pServerAddress;
BLEScan* pBLEScan;

bool deviceFound = false;
char blAddress1[20] = "cc:de:fc:12:2e:01";
char blAddress2[20] = "";
char blAddress3[20] = "";
unsigned long entry;
bool blStatus= false; //ucitava s app bluetooth toggle
bool blDeviceNear=false;


//za senzor
float lastAvg=0; // (xArrayAvg+yArrayAvg+zArrayAvg)/3
float nowAvg=0;
int i;
float diffAvg=0;

//za gps
float latBuffer;
float lngBuffer;
String latBufferString;
String lngBufferString;
String dateBuffer;
String timeBuffer;
String locationIDString;
char locationID[20];
char lastLocationId[20];
float latBufferLast;
float lngBufferLast;
int countVehicleMoved=0;
int validCheckerGPS=0; //if all gps data is valid(date,time,location) then result is 4

int count = 0;
int state = 1;
int PushButton = 2;
int LedDiode =15;
int lowSense=1;
int midSense=3;
int highSense=6;
int delayAfterTrigger=0;
bool lowPowerMode = false;
String triggerSense;

float rawlowSense = (float(lowSense)/50.0); // data for calculating in if to comapre to diffAvg value
float rawmidSense = (float(midSense)/50.0);
float rawhighSense = (float(highSense)/50.0);


// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

unsigned long differenceMillis=0;

// long for storing millis for button hold
unsigned long currentMillisHoldButton=0;

//dijeli mills s 1000 kako bi se usporedio
float secondsPassed;

long delayAfterTriggerMilis = delayAfterTrigger*1000;           // interval at which to blink (milliseconds)


// url za update state
const char URLstate[] = "https://firestore.googleapis.com/v1beta1/projects/vehiclesecurityapp/databases/(default)/documents:commit?key=[AIzaSyBeaGfoqcAExFeKRvkgCOMK-UzguHa-yWE]";
// payload za update state 
const char PAYLOADstate1[] = "{\"writes\": {\"update\": {\"name\": \"projects/vehiclesecurityapp/databases/(default)/documents/users/";
//izmedu ide serialID "blU4OAVa9XU7k6YNom2G"
const char PAYLOADstate2[] = "\",\"fields\": {\"state\": {\"integerValue\":";
//izmedu ide state
const char PAYLOADstate3[] =  "}}}}}";
char PAYLOADstate[200];

// url-i za create new location node
const char URLlocationNode1[] = "https://firestore.googleapis.com/v1/projects/vehiclesecurityapp/databases/(default)/documents/users/";
//izmedu ide serialID "blU4OAVa9XU7k6YNom2G"
const char URLlocationNode2[] = "/locationNodes?documentId=";
//izmedu ide LocationId "202512441655"
const char URLlocationNode3[] = "&prettyPrint=false&fields=fields&key=[AIzaSyBeaGfoqcAExFeKRvkgCOMK-UzguHa-yWE]";
char URLlocationNode[270];
// payload za new location node
// const char PAYLOADlocationNode[] = "{\"fields\":{\"time\":{\"stringValue\":\"17:50\"}, \"date\":{\"stringValue\":\"25/02/2025\"},\"id\":{\"integerValue\":202512441655},\"triggerSense\":{\"stringValue\":\"Medium\"},\"geopoint\":{\"geoPointValue\":{\"latitude\":11.3245,\"longitude\":11.3245}}}}";
char PAYLOADlocationNode[270];
JSONVar myObjectNewLocation;

//url za update location node koordinate
const char URLupdateLocation[] = "https://firestore.googleapis.com/v1/projects/vehiclesecurityapp/databases/(default)/documents:commit?key=[AIzaSyBeaGfoqcAExFeKRvkgCOMK-UzguHa-yWE]";
// payload za update location node koordinate
//const char PAYLOADupdateLocation[] = "{\"writes\":[{\"updateMask\":{\"fieldPaths\":[\"geopoint\"]},\"update\":{\"name\":\"projects/vehiclesecurityapp/databases/(default)/documents/users/blU4OAVa9XU7k6YNom2G/locationNodes/202512441655\",\"fields\":{\"geopoint\":{\"geoPointValue\":{\"latitude\":23.344803170143031,\"longitude\":23.421023690991058}}}}}]}";
char PAYLOADupdateLocation[312];
JSONVar myObjectUpdateLocation;

//url za palit svijetla
const char URLifttt[] = "https://maker.ifttt.com/trigger/alarm/with/key/eIZZKwHpy9eX5IiIZbNIeXbXrCMJdIwG_8WRlnEOeRn";

//url za get
const char URLget1[] = "https://firestore.googleapis.com/v1/projects/vehiclesecurityapp/databases/(default)/documents/users/";
// const char URL[] = "https://firestore.googleapis.com/v1/projects/vehiclesecurityapp/databases/(default)/documents/users/blU4OAVa9XU7k6YNom2G/settings/settingsInfo?prettyPrint=false&fields=fields&key=[AIzaSyBeaGfoqcAExFeKRvkgCOMK-UzguHa-yWE]";
const char URLget2[] = "/settings/settingsInfo?prettyPrint=false&fields=fields&key=[AIzaSyBeaGfoqcAExFeKRvkgCOMK-UzguHa-yWE]";
char URLget[230];
JSONVar myObjectGET;

//char u koji se upisuje json sting od httpgeta
//char *dataRecivedChar;

//bool koji je true ako je http error
bool httperror=false;

//gathers serial id in url link for get; needs to be in setup
void putSerialGET(){
  strcpy(URLget, URLget1);
strcat ( URLget, serialID);
strcat ( URLget, URLget2);
URLget[230]=0;

  }

//reads GPS data until it's readable data
void readGPSData(){
  // This sketch displays information every time a new sentence is correctly encoded.
  validCheckerGPS=0;
  while (ss.available() > 0){
    if (gps.encode(ss.read())){
      
      displayInfo(); 
      Serial.print("\n");
      
    }
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
  }
  
  }
  
}

void checkIfVehicleIsMoved(){
  /*
             * 1.provjera mice li se GPS sa mjesta, isit princip kao i s vremenom,
             * funckija gdje ocitava i onda usporedujes s readom koji se poslao u http postu iznad
             * 
             *  Ako ima razlike onda salji patch state(3) i opet udi u jos jedan loop gdje se cita GPS svake sekunde i salje(patch na isti id) samo ako je svaki sljedeci drugaciji,
             *  u slucaju da nije  nemoj slati. Postojati ce counter koji ce brojati koliko se puta ponovilo isto ocitanje. Ako se prosli read stalno usporeduje s istim ocitanjem 30 puta, onda udi u lock.
             *  Takoder stavi: if(digitalRead(PushButton) == LOW){
                                state=1;}
             *  Pojednostavljeno: ako se 30 sekundi ne mice s mjesta udi u lock
             *  
             * 
             */
             countVehicleMoved=0;
             do{
              latBufferLast = latBuffer;
              lngBufferLast = lngBuffer;
              if(digitalRead(PushButton) == LOW){
                digitalWrite(LedDiode, HIGH);
               state=1;
               if(lowPowerMode==true){
                 state=5;
              }
              break;
            }
              delay(1000);
              do{
              readGPSData();
             }while(validCheckerGPS!=4);
              if(latBufferLast==latBuffer && lngBufferLast==lngBuffer){
                countVehicleMoved++;
                Serial.print(countVehicleMoved);
                Serial.print("\n");
              }
              else{
                HttpPostLocationUpdateLocationNode();
                Serial.println("Updated location");
                Serial.print("\n");
                countVehicleMoved--;
              }
              if(countVehicleMoved==30){
                state=0;
                if(lowPowerMode==true){
                 state=4;
              }
              }
             }while(countVehicleMoved<30);
}


class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice Device){
      //Serial.print("BLE Advertised Device found: ");
      //Serial.println(Device.toString().c_str());
      pServerAddress = new BLEAddress(Device.getAddress()); 
      bool known = false;
      bool Master = false;
        if ((strcmp(pServerAddress->toString().c_str(), blAddress1))== 0 || (strcmp(pServerAddress->toString().c_str(), blAddress2))== 0 || (strcmp(pServerAddress->toString().c_str(), blAddress3))== 0) 
          known = true;
      
      if (known) {
        //Serial.print("Device found: ");
        //Serial.println(Device.getRSSI());
        if (Device.getRSSI() > -40) {
          deviceFound = true;
        }
        else {
          deviceFound = false;
        }
        Device.getScan()->stop();
        delay(100);
      }
    }
};

bool Bluetooth() {
  Serial.println();
  Serial.println("BLE Scan restarted.....");
  BLEScanResults scanResults = pBLEScan->start(5);
  if (deviceFound) {
    Serial.println("Unlocked");
    return true;
    delay(500);
  }
  else{
    Serial.println("Locked");
    return false;
    delay(500);
  }
}

void blInit(){
  
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(false);
  Serial.println("Done");
  }

  void HttpPostLocationUpdateLocationNodeJSONparser(){
  char updateName[120];
  strcpy(updateName, "projects/vehiclesecurityapp/databases/(default)/documents/users/");
  strcat(updateName, serialID);
  strcat(updateName, "/locationNodes/");
  strcat(updateName, lastLocationId);
  updateName[114]=0;
  Serial.println(updateName);
  
  myObjectUpdateLocation["writes"]["updateMask"]["fieldPaths"][0] = "geopoint";
  myObjectUpdateLocation["writes"]["updateMask"]["fieldPaths"][1] = "triggerSense";
  myObjectUpdateLocation["writes"]["updateMask"]["fieldPaths"][2] = "date";
  myObjectUpdateLocation["writes"]["updateMask"]["fieldPaths"][3] = "time";
  myObjectUpdateLocation["writes"]["update"]["name"] = updateName;
  myObjectUpdateLocation["writes"]["update"]["fields"]["geopoint"]["geoPointValue"]["latitude"] = latBuffer;
  myObjectUpdateLocation["writes"]["update"]["fields"]["geopoint"]["geoPointValue"]["longitude"] = lngBuffer;
  myObjectUpdateLocation["writes"]["update"]["fields"]["time"]["stringValue"] = timeBuffer;
  myObjectUpdateLocation["writes"]["update"]["fields"]["date"]["stringValue"] = dateBuffer;
  myObjectUpdateLocation["writes"]["update"]["fields"]["triggerSense"]["stringValue"] = "GPS";
  String buffermyObjectUpdateLocation = JSON.stringify(myObjectUpdateLocation);
  int buffermyObjectUpdateLocation_len = buffermyObjectUpdateLocation.length() + 1; 
  buffermyObjectUpdateLocation.toCharArray(PAYLOADupdateLocation, buffermyObjectUpdateLocation_len);
}

void HttpPostStateJSONparser(){
  PAYLOADstate[0] = 0;
  strcpy(PAYLOADstate, PAYLOADstate1);
  strcat(PAYLOADstate, serialID);
  strcat(PAYLOADstate, PAYLOADstate2);
  String stringState = String(state);
  char charBuf[2];
  stringState.toCharArray(charBuf, 2);
  strcat(PAYLOADstate, charBuf);
  strcat(PAYLOADstate, PAYLOADstate3);
  PAYLOADstate[176] = 0;
  Serial.println(URLstate);
  Serial.println(PAYLOADstate);
}

void postHttpState(){

  HttpPostStateJSONparser();

  
  
//izlazi iz petlje ako nije rc 701 ili 705
  do{

  
  // Establish GPRS connectivity (5 trials) 
  bool connected = false;
  for(uint8_t i = 0; i < 5 && !connected; i++) {
    delay(1000);
    connected = sim800l->connectGPRS();
  }
  


  // Check if connected, if not reset the module and setup the config again
  if(connected) {
    Serial.println(F("GPRS connected !"));
    httperror=false;
  } else {
    Serial.println(F("GPRS not connected !"));
    Serial.println(F("Reset the module."));
    sim800l->reset();
    setupModule();
  }
  
  Serial.println(F("Start HTTP POST..."));
  


  // Do HTTP POST communication with 10s for the timeout (read and write)
  uint16_t rc = sim800l->doPost(URLstate, CONTENT_TYPE, PAYLOADstate, 10000, 10000);
   if(rc == 200) {
    // Success, output the data received on the serial
    Serial.print(F("HTTP POST successful ("));
    Serial.print(sim800l->getDataSizeReceived());
    Serial.println(F(" bytes)"));
    Serial.print(F("Received : "));
    
    //Serial.println(sim800l->getDataReceived());
  } else {
    // Failed...
    Serial.print(F("HTTP POST error "));
    Serial.println(rc);
    
      sim800l->reset();
    setupModule();
    httperror=true;
  }
  
  if(!httperror){
  // Close GPRS connectivity (5 trials)
  bool disconnected = sim800l->disconnectGPRS();
  for(uint8_t i = 0; i < 5 && !connected; i++) {
    delay(1000);
    disconnected = sim800l->disconnectGPRS();
  }
  
  if(disconnected) {
     Serial.println(F("GPRS disconnected !"));
  
   } else {
     Serial.println(F("GPRS still connected !"));
     connected=true;
   }
  }
  else{
    connected=true;
    }

  
  }while(httperror);
  }
  
void HttpPostNewLocationNodeURL(){
  URLlocationNode[0]=0;
  strcpy(URLlocationNode, URLlocationNode1);
  strcat(URLlocationNode, serialID);
  strcat(URLlocationNode, URLlocationNode2);
  strcat(URLlocationNode, locationID);
  strcat(URLlocationNode, URLlocationNode3);
  URLlocationNode[270]= 0;
  Serial.println(URLlocationNode);
  }
  
//pretvara varijable u json payload
void HttpPostNewLocationNodeJSONparser(){
  
  myObjectNewLocation["fields"]["time"]["stringValue"] = timeBuffer;
  myObjectNewLocation["fields"]["date"]["stringValue"] = dateBuffer;
  myObjectNewLocation["fields"]["id"]["stringValue"] = locationID;
  myObjectNewLocation["fields"]["triggerSense"]["stringValue"] = triggerSense;
  myObjectNewLocation["fields"]["geopoint"]["geoPointValue"]["latitude"] = latBuffer;
  myObjectNewLocation["fields"]["geopoint"]["geoPointValue"]["longitude"] = lngBuffer;
  String buffermyObjectNewLocation = JSON.stringify(myObjectNewLocation);
  int buffermyObjectNewLocation_len = buffermyObjectNewLocation.length() + 1; 
  buffermyObjectNewLocation.toCharArray(PAYLOADlocationNode, buffermyObjectNewLocation_len);
  Serial.println(PAYLOADlocationNode);
}


void  HttpPostNewLocationNode(){
  
  HttpPostNewLocationNodeURL();
  HttpPostNewLocationNodeJSONparser();
  
  //izlazi iz petlje ako nije rc 701 ili 705
  do{
  
  //sim800l->setPowerMode(NORMAL);
  // Establish GPRS connectivity (5 trials)
  bool connected = false;
  for(uint8_t i = 0; i < 5 && !connected; i++) {
    delay(1000);
    connected = sim800l->connectGPRS();
  }
    
  // Check if connected, if not reset the module and setup the config again
  if(connected) {
    Serial.println(F("GPRS connected !"));
    httperror=false;
  } else {
    Serial.println(F("GPRS not connected !"));
    Serial.println(F("Reset the module."));
    sim800l->reset();
    setupModule();
    
  }
  Serial.println(URLlocationNode);
  Serial.println(PAYLOADlocationNode);
  Serial.println(F("Start HTTP POST..."));
  

  
  // Do HTTP POST communication with 10s for the timeout (read and write)
  uint16_t rc = sim800l->doPost(URLlocationNode, CONTENT_TYPE, PAYLOADlocationNode, 10000, 10000);
   if(rc == 200) {
    // Success, output the data received on the serial
    Serial.print(F("HTTP POST successful ("));
    Serial.print(sim800l->getDataSizeReceived());
    Serial.println(F(" bytes)"));
    Serial.print(F("Received : "));
    
    //Serial.println(sim800l->getDataReceived());
  } else {
    // Failed...
    Serial.print(F("HTTP POST error "));
    Serial.println(rc);
    
      sim800l->reset();
    setupModule();
    httperror=true;
  }
  
  if(!httperror){
  // Close GPRS connectivity (5 trials)
  bool disconnected = sim800l->disconnectGPRS();
  for(uint8_t i = 0; i < 5 && !connected; i++) {
    delay(1000);
    disconnected = sim800l->disconnectGPRS();
  }
  
  if(disconnected) {
     Serial.println(F("GPRS disconnected !"));
   } else {
     Serial.println(F("GPRS still connected !"));
     connected=true;
   }
  }
  else{
    connected=true;
    }

  }while(httperror);

}

void  HttpGetAllSettingsInfo(){

  putSerialGET();

    //izlazi iz petlje ako nije rc 701 ili 705
  do{

  //sim800l->setPowerMode(NORMAL);
  // Establish GPRS connectivity (5 trials)
  bool connected = false;
  for (uint8_t i = 0; i < 5 && !connected; i++) {
    delay(1000);
    connected = sim800l->connectGPRS();
  }


  // Check if connected, if not reset the module and setup the config again
  if (connected) {
    Serial.println(F("GPRS connected !"));
    httperror=false;
  } else {
    Serial.println(F("GPRS not connected !"));
    Serial.println(F("Reset the module."));
    sim800l->reset();
    setupModule();
  }

  Serial.println(F("Start HTTP GET..."));

  // Do HTTP GET communication with 10s for the timeout (read)
  uint16_t rc = sim800l->doGet(URLget, 10000);
  if (rc == 200) {
    // Success, output the data received on the serial
    Serial.print(F("HTTP GET successful ("));
    Serial.print(sim800l->getDataSizeReceived());
    Serial.println(F(" bytes)"));
    Serial.print(F("Received : "));
    Serial.println("\n");;

    
    Serial.println(sim800l->getDataReceived());
    
    myObjectGET = JSON.parse(sim800l->getDataReceived());
    
    /*
    const char* blAddress1Buffer = (const char*) myObjectGET["fields"]["blAddress1"]["stringValue"];
    const char* blAddress2Buffer = (const char*) myObjectGET["fields"]["blAddress2"]["stringValue"];
    const char* blAddress3Buffer = (const char*) myObjectGET["fields"]["blAddress3"]["stringValue"];
    */
    
    strncpy(blAddress1, (const char*) myObjectGET["fields"]["blAddress1"]["stringValue"], 20);
    blAddress1[18] = 0; //buffer is overrun and so it needs to be shorten-because strncpy function has error
    strncpy(blAddress2, (const char*) myObjectGET["fields"]["blAddress2"]["stringValue"], 20);
    blAddress2[18] = 0; //buffer is overrun and so it needs to be shorten-because strncpy function has error
    strncpy(blAddress3, (const char*) myObjectGET["fields"]["blAddress3"]["stringValue"], 20);
    blAddress3[18] = 0; //buffer is overrun and so it needs to be shorten-because strncpy function has error
    blStatus = (bool) myObjectGET["fields"]["blStatus"]["booleanValue"];
    lowSense = atoi((const char*) myObjectGET["fields"]["lowSense"]["integerValue"]);
    midSense = atoi((const char*) myObjectGET["fields"]["midSense"]["integerValue"]);
    highSense = atoi((const char*) myObjectGET["fields"]["highSense"]["integerValue"]);
    lowPowerMode = (bool) myObjectGET["fields"]["lowPowerMode"]["booleanValue"];
    delayAfterTrigger = atoi((const char*) myObjectGET["fields"]["delay"]["integerValue"]);

      Serial.println("bl1");
      Serial.println(blAddress1);
      Serial.println("bl2");
      Serial.println(blAddress2);
      Serial.println("bl3");
      Serial.println(blAddress3);
      Serial.println("blStatus");
      if(blStatus){
      Serial.println("true");
      }
      else{
      Serial.println("false");
      }
      Serial.println("lowSense");
      Serial.println(lowSense); 
      Serial.println("midSense");
      Serial.println(midSense);
      Serial.println("highSense");
      Serial.println(highSense);
      
      Serial.println("delayAfterTrigger");
      Serial.println(delayAfterTrigger);
    
      Serial.println("lowPowerMode");
      if(lowPowerMode){
      Serial.println("true");
      }
      else{
      Serial.println("false");
      }

      //nakon geta stavi iz varijabli low,mid,high u varijablu s kojom ce se usporedivati s diffavg
       rawlowSense = (float(lowSense)/50.0); // data for calculating in if to comapre to diffAvg value
       rawmidSense = (float(midSense)/50.0);
       rawhighSense = (float(highSense)/50.0);
      
    
  } else {
    // Failed...
    Serial.print(F("HTTP GET error "));
    Serial.println(rc);
    
      sim800l->reset();
    setupModule();
    httperror=true;
  }



  if(!httperror){
  // Close GPRS connectivity (5 trials)
  bool disconnected = sim800l->disconnectGPRS();
  for(uint8_t i = 0; i < 5 && !connected; i++) {
    delay(1000);
    disconnected = sim800l->disconnectGPRS();
  }
  
  if(disconnected) {
     Serial.println(F("GPRS disconnected !"));
   } else {
     Serial.println(F("GPRS still connected !"));
     connected=true;
   }
  }
  else{
    connected=true;
    }
/*
  // Go into low power mode
  bool lowPowerMode = sim800l->setPowerMode(MINIMUM);
  if (lowPowerMode) {
    Serial.println(F("Module in low power mode"));
  } else {
    Serial.println(F("Failed to switch module to low power mode"));
  }
  */
  
  }while(httperror);
}

void HttpPostLocationUpdateLocationNode(){

  HttpPostLocationUpdateLocationNodeJSONparser();

    //izlazi iz petlje ako nije rc 701 ili 705
  do{

  // Establish GPRS connectivity (5 trials)
  bool connected = false;
  for(uint8_t i = 0; i < 5 && !connected; i++) {
    delay(1000);
    connected = sim800l->connectGPRS();
  }


  // Check if connected, if not reset the module and setup the config again
  if(connected) {
    Serial.println(F("GPRS connected !"));
    httperror=false;
  } else {
    Serial.println(F("GPRS not connected !"));
    Serial.println(F("Reset the module."));
    sim800l->reset();
    setupModule();
  }
  
  Serial.println(F("Start HTTP POST..."));
  


  // Do HTTP POST communication with 10s for the timeout (read and write)
  uint16_t rc = sim800l->doPost(URLupdateLocation, CONTENT_TYPE, PAYLOADupdateLocation, 10000, 10000);
   if(rc == 200) {
    // Success, output the data received on the serial
    Serial.print(F("HTTP POST successful ("));
    Serial.print(sim800l->getDataSizeReceived());
    Serial.println(F(" bytes)"));
    Serial.print(F("Received : "));
    
    //Serial.println(sim800l->getDataReceived());
  } else {
    // Failed...
    Serial.print(F("HTTP POST error "));
    Serial.println(rc);
   
      sim800l->reset();
    setupModule();
    httperror=true;
  }

  if(!httperror){
  // Close GPRS connectivity (5 trials)
  bool disconnected = sim800l->disconnectGPRS();
  for(uint8_t i = 0; i < 5 && !connected; i++) {
    delay(1000);
    disconnected = sim800l->disconnectGPRS();
  }
  
  if(disconnected) {
     Serial.println(F("GPRS disconnected !"));
   } else {
     Serial.println(F("GPRS still connected !"));
     connected=true;
   }
  }
  else{
    connected=true;
    }
  }while(httperror);

}



void afterTriggerFunction(String triggerSenseString){

  triggerSense=triggerSenseString;
  Serial.println(triggerSense);
   if(delayAfterTrigger==0){
    state=2;
    postHttpState();
    digitalWrite(LedDiode, LOW);
   }
   
    previousMillis=millis(); //16000
    do{
            unsigned long currentMillis = millis();
            differenceMillis = currentMillis - previousMillis;
            
            if(delayAfterTrigger!=0){
            if(differenceMillis >= (delayAfterTrigger*1000)){

            state=2;
            postHttpState();
            digitalWrite(LedDiode, LOW);
            /*
             * 1.stavi da ucita GPS- date, time i geopoint
             * 2.funkcija koja pretvara date i time u id
             * 3.triggerSense se zapisuje u varijablu
             * 4.stavi http post za novi locationNode, ukljucujuci sve ovo iznad 
             * 
             */
             
             
             do{
              readGPSData();
             }while(validCheckerGPS!=4);
             
             latBufferLast = latBuffer;
             lngBufferLast = lngBuffer;
             strncpy(lastLocationId, locationID, 20);
             lastLocationId[15] = 0; //buffer is overrun and so it needs to be shorten-because strncpy function has error
             HttpPostNewLocationNode();
            
            } 
            
            Serial.print("\n Difference:");
            Serial.print(differenceMillis);

                        
            if(digitalRead(PushButton) == LOW){
               state=1;
               digitalWrite(LedDiode, HIGH);
               if(lowPowerMode==true){
                 state=5;
              }
            }

            }
            else{
            /*
             * 1.stavi da ucita GPS- date, time i geopoint
             * 2.funkcija koja pretvara date i time u id
             * 3.triggerSense se zapisuje u varijablu
             * 4.stavi http post za novi locationNode, ukljucujuci sve ovo iznad 
             * 
             */
             
             
             do{
              readGPSData();
             }while(validCheckerGPS!=4);
             latBufferLast = latBuffer;
             lngBufferLast = lngBuffer;
             strncpy(lastLocationId, locationID, 20);
             lastLocationId[15] = 0; //buffer is overrun and so it needs to be shorten-because strncpy function has error
             HttpPostNewLocationNode();      
            }
    }
    while(state==0);
     if(delayAfterTrigger!=0 && state!=2){
    postHttpState();
   }
    if(state==5){
      state=1;
      }
    if(state!=1){
     previousMillis=millis(); //16000
     
        do{
            
             unsigned long currentMillis = millis();
            differenceMillis = currentMillis - previousMillis;
            if(differenceMillis >= 10000){
              state=0;
               if(lowPowerMode==true){
                 state=4;
              }              
            }

                        Serial.print("\n Difference:");
            Serial.print(differenceMillis);

                        
            if(digitalRead(PushButton) == LOW){
               state=1;
               digitalWrite(LedDiode, HIGH);
               if(lowPowerMode==true){
                 state=5;
              }               
            }


            /*
             * 1.provjera mice li se GPS sa mjesta, isit princip kao i s vremenom,
             * funckija gdje ocitava i onda usporedujes s readom koji se poslao u http postu iznad
             * 
             *  Ako ima razlike onda salji patch state(3) i opet udi u jos jedan loop gdje se cita GPS svake sekunde i salje(patch na isti id) samo ako je svaki sljedeci drugaciji,
             *  u slucaju da nije  nemoj slati. Postojati ce counter koji ce brojati koliko se puta ponovilo isto ocitanje. Ako se prosli read stalno usporeduje s istim ocitanjem 30 puta, onda udi u lock.
             *  Takoder stavi: if(digitalRead(PushButton) == LOW){
                                state=1;}
             *  Pojednostavljeno: ako se 30 sekundi ne mice s mjesta udi u lock
             *  
             * 
             */
             do{
              readGPSData();
             }while(validCheckerGPS!=4);
             if((abs(latBufferLast - latBuffer) > 0.000001) || (abs(latBufferLast - latBuffer) > 0.000001)){
              state=3;
              postHttpState();
              checkIfVehicleIsMoved();
             }
            
    }
    while(state==2); //ako je 0 ili 1 izadi a ako je 2 cekaj 10s
      postHttpState();
      if(state==4){
       state=0;
      }
      else if(state==5){
        state=1;
        }
      
      
     
     }
}

void displayInfo()
{

  
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    latBuffer = gps.location.lat();
    lngBuffer = gps.location.lng();
    latBufferString = String(latBuffer, 6);
    lngBufferString = String(lngBuffer, 6);
    Serial.print(latBuffer, 6);
    Serial.print(F(","));
    Serial.print(lngBuffer, 6);
    validCheckerGPS++;
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date: "));
  if (gps.date.isValid())
  {
    if (gps.date.day() < 10){
      dateBuffer = "0" + String(gps.date.day())+ "/";
      }
      else{
    dateBuffer = String(gps.date.day())+ "/";
      }
        if (gps.date.month() < 10){
      dateBuffer = dateBuffer + "0" + String(gps.date.month())+ "/";
      }
      else{
    dateBuffer = dateBuffer + String(gps.date.month())+ "/";
      }
    dateBuffer = dateBuffer + String(gps.date.year());
    Serial.print(dateBuffer);
    validCheckerGPS++;

  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Time: "));
  if (gps.time.isValid())
  {
    
    if (gps.time.hour() < 10){
      timeBuffer = "0" + String(gps.time.hour()+2) + ":";
      }
      else{
    timeBuffer = String(gps.time.hour()) + ":";
      }
    if (gps.time.minute() < 10){
      timeBuffer = timeBuffer + "0" + String(gps.time.minute());
      }
      else{
    timeBuffer = timeBuffer +String(gps.time.minute());
      }
    Serial.print(timeBuffer);
    validCheckerGPS++;
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  ID: "));
  if (gps.date.isValid())
  {
    locationIDString = String(gps.date.year());
    if (gps.date.month() < 10){
      locationIDString = locationIDString + "0" + String(gps.date.month());
      }
      else{
    locationIDString = locationIDString + String(gps.date.month());
      }
    if (gps.date.day() < 10){
      locationIDString = locationIDString + "0" + String(gps.date.day());
      }
      else{
    locationIDString = locationIDString + String(gps.date.day());
      }
    if (gps.time.hour() < 10){
      locationIDString = locationIDString + "0" + String(gps.time.hour());
      }
      else{
    locationIDString = locationIDString + String(gps.time.hour());
      }
    if (gps.time.minute() < 10){
      locationIDString = locationIDString + "0" + String(gps.time.minute());
      }
      else{
    locationIDString = locationIDString + String(gps.time.minute());
      }
    if (gps.time.second() < 10){
      locationIDString = locationIDString + "0" + String(gps.time.second());
      }
      else{
    locationIDString = locationIDString + String(gps.time.second());
      }
      
      locationIDString.toCharArray(locationID, 20);
      
    Serial.print(locationID);
    validCheckerGPS++;
  }
  else
  {
    Serial.print(F("INVALID"));
  }
}

void setupModule() {
  // Wait until the module is ready to accept AT commands
  while (!sim800l->isReady()) {
    Serial.println(F("Problem to initialize AT command, retry in 1 sec"));
    delay(1000);
  }
  Serial.println(F("Setup Complete!"));

  // Wait for the GSM signal
  uint8_t signal = sim800l->getSignal();
  while (signal <= 0) {
    delay(1000);
    signal = sim800l->getSignal();
  }
  Serial.print(F("Signal OK (strenght: "));
  Serial.print(signal);
  Serial.println(F(")"));
  delay(1000);

  // Wait for operator network registration (national or roaming network)
  NetworkRegistration network = sim800l->getRegistrationStatus();
  while (network != REGISTERED_HOME && network != REGISTERED_ROAMING) {
    delay(1000);
    network = sim800l->getRegistrationStatus();
  }
  Serial.println(F("Network registration OK"));
  delay(1000);

  // Setup APN for GPRS configuration
  bool success = sim800l->setupGPRS(APN);
  while (!success) {
    success = sim800l->setupGPRS(APN);
    delay(5000);
  }
  Serial.println(F("GPRS config OK"));
}

void setup()
{

    Serial.begin(115200);

    pinMode(PushButton, INPUT_PULLUP);
    pinMode(LedDiode, OUTPUT);
    digitalWrite(LedDiode, LOW);
    
  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  // Set serial for AT commands (to SIM800 module)

#define SerialAT Serial1
 
  while (!Serial);

  // Initialize the hardware Serial1
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(1000);

  // Initialize SIM800L driver with an internal buffer of 200 bytes and a reception buffer of 512 bytes, debug disabled
  sim800l = new SIM800L((Stream *)&Serial1, SIM800_RST_PIN, 200, 512);

  // Equivalent line with the debug enabled on the Serial
  // sim800l = new SIM800L((Stream *)&Serial1, SIM800_RST_PIN, 200, 512, (Stream *)&Serial);
  
  // Setup module for GPRS communication
  setupModule();
  
    ss.begin(GPSBaud);
    
    
    
    blInit();
    HttpGetAllSettingsInfo();
    mpu.Initialize();
     Serial.println("=====================================");
  Serial.println("Starting calibration...");
  mpu.Calibrate();
  Serial.println("Calibration complete!");
  Serial.println("Offsets:");
  Serial.print("GyroX Offset = ");
  Serial.println(mpu.GetGyroXOffset());
  Serial.print("GyroY Offset = ");
  Serial.println(mpu.GetGyroYOffset());
  Serial.print("GyroZ Offset = ");
  Serial.println(mpu.GetGyroZOffset());

    // tako da se skine settings kad se upali uredaj 
    do{
    readGPSData();
  }while(validCheckerGPS!=4);
  Serial.print("GPS location found");
    
    digitalWrite(LedDiode, HIGH);
}

void loop()
{
previousMillis=millis();

if(!blStatus){
  if (digitalRead(PushButton) == LOW){
    
    if(state==1){
      
      // for btn hold
      while(digitalRead(PushButton) == LOW){
        currentMillisHoldButton = millis();
        differenceMillis = currentMillisHoldButton - previousMillis;   
        if(differenceMillis >= 4000){
          digitalWrite(LedDiode, LOW);
          HttpGetAllSettingsInfo();
          digitalWrite(LedDiode, HIGH);
        }
      }
      currentMillisHoldButton=0;
      
      state=0;
      digitalWrite(LedDiode, LOW);
      mpu.Execute();
      lastAvg=(mpu.GetAccX()+mpu.GetAccY()+mpu.GetAccZ())/3;
      }
      
    else if(state==0){
      state=1;
      digitalWrite(LedDiode, HIGH);
      delay(500);
    }
    if(lowPowerMode==false){
        postHttpState();
    }
    delay(2000);
  }
}
else{

  // every 2 seconds read ble
  if(previousMillis > currentMillisHoldButton){
    if(previousMillis<4294964295){
    currentMillisHoldButton=previousMillis+8000;
  blDeviceNear=Bluetooth();
    
    if(blDeviceNear && state==0){
      state=1;
      digitalWrite(LedDiode, HIGH);
      if(lowPowerMode==false){
        postHttpState();
      }
      delay(1500);
    }
    else if(!blDeviceNear && state==1){
      state=0;
      digitalWrite(LedDiode, LOW);
      if(lowPowerMode==false){
        postHttpState();
      }
      delay(1500);
      mpu.Execute();
      lastAvg=(mpu.GetAccX()+mpu.GetAccY()+mpu.GetAccZ())/3;
    }
    
    //delay(1000);
  }
  }
  
  // for btn hold
  else if((digitalRead(PushButton) == LOW) && state==1){
    while(digitalRead(PushButton) == HIGH){
        currentMillisHoldButton = millis();
        differenceMillis = currentMillisHoldButton - previousMillis;   
        if(differenceMillis >= 4000){
          digitalWrite(LedDiode, LOW);
          HttpGetAllSettingsInfo();
          digitalWrite(LedDiode, HIGH);
        }
      }
      currentMillisHoldButton=0;
  }
  
  }


if(state==0){
  mpu.Execute();
  delay(100);
  nowAvg=(mpu.GetAccX()+mpu.GetAccY()+mpu.GetAccZ())/3;
  diffAvg=abs(nowAvg-lastAvg);
  
  Serial.print("\nlast");
  Serial.print(lastAvg);
  Serial.print("\nnow");
  Serial.print(nowAvg);
  Serial.print("\ndiff");
  Serial.print(diffAvg);

     if(diffAvg>rawhighSense){  //trigger za high
      digitalWrite(LedDiode, HIGH);
      afterTriggerFunction("High");
      mpu.Execute();
      lastAvg=(mpu.GetAccX()+mpu.GetAccY()+mpu.GetAccZ())/3;
      
     }

     else if(diffAvg>rawmidSense){  //trigger za medium
       digitalWrite(LedDiode, HIGH);
       afterTriggerFunction("Medium");
       mpu.Execute();
      lastAvg=(mpu.GetAccX()+mpu.GetAccY()+mpu.GetAccZ())/3;
       
     }

    else if(diffAvg>rawlowSense){  //trigger za low
      digitalWrite(LedDiode, HIGH);
      afterTriggerFunction("Low");
      mpu.Execute();
      lastAvg=(mpu.GetAccX()+mpu.GetAccY()+mpu.GetAccZ())/3;
      
    }


    }


}
