#include <ESP8266WiFi.h>
#include "FirebaseESP8266.h"

#define FIREBASE_HOST "phys-labs.firebaseio.com"
#define FIREBASE_AUTH "is9YhJowq75WQr9Ioc2M6X3714p4owRZZSXdPiYA"
// Home WiFi
#define WIFI_SSID "4G-UFI-954054"
#define WIFI_PASSWORD "DaniIl_1998"
//#define WIFI_SSID "TP-Link_790C"
//#define WIFI_PASSWORD "16956483"
// Mobile WiFi
//#define WIFI_SSID "Neffos X1 Lite"
//#define WIFI_PASSWORD "12345678"
// 324 WiFi
//#define WIFI_SSID "koef-324"
//#define WIFI_PASSWORD "A$pddmIedp"

//Define Firebase Data objects
FirebaseData firebaseData;

String URL_TO_SWITCH_STATE = "labs/kundt/switchState";
String switchState = "off";

int gpio13Led = 13;
int gpio12Relay = 12;

void setup(void) {
  // preparing GPIOs
  pinMode(gpio13Led, OUTPUT);
  digitalWrite(gpio13Led, HIGH);

  pinMode(gpio12Relay, OUTPUT);
  digitalWrite(gpio12Relay, HIGH);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA); //?
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //  Serial.println("");

  if (WiFi.getAutoConnect() != true) {   //configuration will be saved into SDK flash area
    WiFi.setAutoConnect(true);   //on power-on automatically connects to last used hwAP
  }
  WiFi.setAutoReconnect(true);    //automatically reconnects to hwAP in case it's disconnected 

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  //  Serial.println("");
  //  Serial.print("Connected to ");
  //  Serial.println(ssid);
  //  Serial.print("IP address: ");
  //  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //In setup(), set the stream callback function to handle data
  //streamCallback is the function that called when database data changes or updates occurred
  //streamTimeoutCallback is the function that called when the connection between the server
  //and client was timeout during HTTP stream

  Firebase.setStreamCallback(firebaseData, streamCallback, streamTimeoutCallback);

  //In setup(), set the streaming path to "/test/data" and begin stream connection

  if (!Firebase.beginStream(firebaseData, URL_TO_SWITCH_STATE)) {
    //Could not begin stream connection, then print out the error detail
    //    Serial.println(firebaseData.errorReason());
  }
}

//Global function that handles stream data
void streamCallback(StreamData data) {
  //Stream data can be many types which can be determined from function dataType

  switchState = data.stringData();

  if (switchState == "off") {
    digitalWrite(gpio13Led, HIGH);
    digitalWrite(gpio12Relay, LOW);
    delay(1000);
  } else if (switchState == "on") {
    digitalWrite(gpio13Led, LOW);
    digitalWrite(gpio12Relay, HIGH);
    delay(1000);
  }
}

//Global function that notifies when stream connection lost
//The library will resume the stream connection automatically
void streamTimeoutCallback(bool timeout)
{
  if (timeout) {
    //Stream timeout occurred
    //    Serial.println("Stream timeout, resume streaming...");
  }
}

void loop(void) {}
