#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <AsciiMassagePacker.h>
#include <AsciiMassageParser.h>
#include <ArduinoJson.h>
AsciiMassageParser inbound;
AsciiMassagePacker outbound;

#define FIREBASE_HOST "phys-labs.firebaseio.com"
#define FIREBASE_AUTH "is9YhJowq75WQr9Ioc2M6X3714p4owRZZSXdPiYA"
#define WIFI_SSID "koef-324"
#define WIFI_PASSWORD "A$pddmIedp"

////Define Firebase Data objects
//FirebaseData firebaseData1;
//FirebaseData firebaseData2;
//FirebaseData firebaseData3;
//FirebaseData firebaseDataIsReady;

int frequency = 2000;
String path = "/labs/2/status";
String userNodeID = "currentUser";
String bpStateNodeID = "bpState";
String user = "null";
String chartId = "";
String chartPath = "";
String isSignal = "false";
String isOk = "true";

unsigned long period_time = (long)600000;
// переменная таймера, максимально большой целочисленный тип (он же uint32_t)
unsigned long my_timer = millis();
StaticJsonBuffer<200> jsonBuffer;

JsonObject& root = jsonBuffer.createObject();

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  //  //  Serial.println("isReady");
  //  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  //  Firebase.reconnectWiFi(true);
  //
  //  //  if (!Firebase.beginStream(firebaseData1, path + "/" + userNodeID));
  //  if (!Firebase.beginStream(firebaseData2, path + "/" + bpStateNodeID));
  //

  user = Firebase.getString("labs/2/status/currentUser");
  chartPath = "/users/" + user;
  chartId = Firebase.getString("labs/2/status/chartId");
  chartPath += "/charts/2/" + chartId;

  Serial.println(chartPath);
  my_timer = millis();
  Serial.println(my_timer);
  Firebase.setString(path + "/" + "isReady", "true");
  Firebase.stream("labs2/2/status/bpState");
}

void loop(void) {
  if (Firebase.failed()) {
    Serial.println("streaming error");
    Serial.println(Firebase.error());
  }
  if ( isOk == "true" && Firebase.available()) {
    FirebaseObject event = Firebase.readEvent();
    isSignal = Firebase.getString("/labs/2/status/bpState");
//    Serial.println("firebase signal: " + isSignal);
    //    Serial.println(event.getString("type"));
    //    Serial.println(event.getString("path"));
    //    Serial.println(event.getString("data"));
    //    //    isSignal = firebaseData2.boolData();
  }

  if (isSignal == "true") {
    Serial.println(isSignal);
    isSignal = "false";
    isOk="false";
    my_timer = millis();   // "сбросить" таймер
//    frequency = Firebase.getInt("/status/frequency");
    outbound.streamOneInt(&Serial, "step", 0); // End the packet and stream it.
    outbound.streamEmpty(&Serial, "");
  }

  if ( inbound.parseStream( &Serial ) ) {
    // parse completed massage elements here.
    if ( inbound.fullMatch ("result") ) {
      // Get the first long.

      float value = inbound.nextFloat();
      String valueStr = String(value);
      root["x"] = frequency;//???? шаг
      root["y"] = valueStr;
      Firebase.setString(path + "/" + bpStateNodeID, "false");
      
      //        delay(500);
      Firebase.push(chartPath, root);
      isOk="true";
    }
  }

  if (millis() - my_timer > period_time) {//
//    Serial.println(millis()-my_timer);
    
    user = "null";
    Firebase.setString("/status/currentUser", "null");
    Firebase.setString(path + "/" + "isReady", "false");
    Firebase.setString(path + "/" + "isTimeOut", "true");
    Firebase.setString(path + "/" + "power", "off");
  }
}
