#include <ESP8266WiFi.h>
#include "FirebaseESP8266.h"
#include <AsciiMassagePacker.h>
#include <AsciiMassageParser.h>

AsciiMassageParser inbound;
AsciiMassagePacker outbound;

#define FIREBASE_HOST "phys-labs.firebaseio.com"
#define FIREBASE_AUTH "is9YhJowq75WQr9Ioc2M6X3714p4owRZZSXdPiYA"
#define WIFI_SSID "TP-Link_790C"
#define WIFI_PASSWORD "16956483"

//Define Firebase Data objects
FirebaseData firebaseData0;
FirebaseData firebaseData1;
FirebaseData firebaseData2;
FirebaseData firebaseData3;
FirebaseData firebaseDataIsReady;
FirebaseJson json2;

int frequency = 2000;
String path = "/status";
String userFirebase = "null";
String userNodeID = "currentUser";
String bpStateNodeID = "bpState";
String freqNodeID = "frequency";
String chartIdNodeID = "chartId";
String user = "null";
String chartId = "";
String chartPath = "";
String inString = "";
String msg = "";
boolean bp = false;
String inputString = "";         // строка, в которую будут записываться входящие данные
boolean stringComplete = false;  // заполнилась ли строка или нет

unsigned long period_time = (long)60000;
// переменная таймера, максимально большой целочисленный тип (он же uint32_t)
unsigned long my_timer;

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  //  Serial.println("isReady");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  
  Firebase.setBool(firebaseDataIsReady, path + "/" + "isReady", true);
  Firebase.beginStream(firebaseData1, path + "/" + userNodeID);
  Firebase.beginStream(firebaseData2, path + "/" + bpStateNodeID);
}

void loop(void) {
  Firebase.readStream(firebaseData1);
  firebaseData1.streamTimeout();
  Firebase.readStream(firebaseData2);
  firebaseData2.streamTimeout();

    if ( inbound.parseStream( &Serial ) ) {
      // parse completed massage elements here.
      if ( inbound.fullMatch ("v") ) {
        // Get the first long.
        long value = inbound.nextLong();
        String valueStr = String(value);
        json2.set("x", frequency);
        json2.set("y", valueStr);
  
        Firebase.pushJSON(firebaseData3, chartPath, json2);
        Firebase.setBool(firebaseData2, path + "/" + bpStateNodeID, false);
      }
    }

  if (firebaseData1.streamAvailable()) {
//    Serial.println(firebaseData1.stringData());
      if (firebaseData1.stringData() != "null" && user != firebaseData1.stringData()) {
        user = firebaseData1.stringData();
//        Serial.println(user);
        chartPath = "/users/" + user;
        if (Firebase.getString(firebaseData3, "/status/chartId", chartId)) {
          chartPath += "/charts/" + chartId;
        }
        my_timer = millis();
      }
  }
  if (millis() - my_timer > 100000) {//period_time
      user = "null";
      Firebase.setString(firebaseDataIsReady, path + "/" + "power", "off");
      Firebase.setString(firebaseData1, "/status/currentUser", "null");
   }
   
  if (firebaseData2.streamAvailable()) {
//    Serial.println(firebaseData2.boolData());
//    Serial.println(user);
    if (user != "null") {
      if (firebaseData2.boolData() == 1) {
//        Serial.println("условие");
        my_timer = millis();   // "сбросить" таймер
//        bp = false;
        if (Firebase.getInt(firebaseData3, "/status/frequency", frequency)) {
//          Serial.println(frequency);
//          outbound.packOneInt("f", frequency); // Start a packet with the address called "value".
//          outbound.addInt(  ); // Add a reading of analog 0.
          outbound.streamOneInt(&Serial, "f", frequency); // End the packet and stream it.
          outbound.streamEmpty(&Serial, "");
        }
      }
    }
  }
}
