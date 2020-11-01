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
FirebaseData firebaseData1;
FirebaseData firebaseData2;
FirebaseData firebaseData3;
FirebaseData firebaseDataIsReady;
FirebaseJson json2;

int frequency = 2000;
String path = "/status";
String userNodeID = "currentUser";
String bpStateNodeID = "bpState";
String user = "null";
String chartId = "";
String chartPath = "";
bool isSignal = 0;

unsigned long period_time = (long)600000;
// переменная таймера, максимально большой целочисленный тип (он же uint32_t)
unsigned long my_timer = millis();

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

  //  if (!Firebase.beginStream(firebaseData1, path + "/" + userNodeID));
  if (!Firebase.beginStream(firebaseData2, path + "/" + bpStateNodeID));

  if (Firebase.getString(firebaseData1, "/status/currentUser")) {
    Serial.println(firebaseData1.stringData());
    user = firebaseData1.stringData();
    chartPath = "/users/" + user;
    if (Firebase.getString(firebaseData3, "/status/chartId", chartId)) {
      chartPath += "/charts/" + chartId;
    }
    //      Serial.println(chartPath);
    my_timer = millis();
  }

  if (!Firebase.setBool(firebaseDataIsReady, path + "/" + "isReady", true));
}

void loop(void) {
  if (!Firebase.readStream(firebaseData2));
  if (!firebaseData2.streamTimeout());

  if (firebaseData2.streamAvailable()) {
//    Serial.println("firebase signal: " + firebaseData2.boolData());
    isSignal = firebaseData2.boolData();
  }
  
  if (isSignal == 1) {
//    Serial.println(isSignal);
    isSignal = 0;
    my_timer = millis();   // "сбросить" таймер
    if (Firebase.getInt(firebaseData3, "/status/frequency", frequency)) {
      outbound.streamOneInt(&Serial, "f", frequency); // End the packet and stream it.
      outbound.streamEmpty(&Serial, "");
    }
  }

  if ( inbound.parseStream( &Serial ) ) {
    // parse completed massage elements here.
    if ( inbound.fullMatch ("v") ) {
      // Get the first long.
      long value = inbound.nextLong();
      String valueStr = String(value);
      json2.set("x", frequency);
      json2.set("y", valueStr);
      if (!Firebase.setBool(firebaseData2, path + "/" + bpStateNodeID, false));
      //        delay(500);
      if (!Firebase.pushJSON(firebaseData3, chartPath, json2));
    }
  }

  if (millis() - my_timer > period_time) {//
    user = "null";
    if (!Firebase.setString(firebaseData1, "/status/currentUser", "null"));
    if (!Firebase.setBool(firebaseDataIsReady, path + "/" + "isReady", false));
    if (!Firebase.setBool(firebaseDataIsReady, path + "/" + "isTimeOut", true));
    if (!Firebase.setString(firebaseDataIsReady, path + "/" + "power", "off"));
  }
}
