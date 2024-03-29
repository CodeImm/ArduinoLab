// Include ESP8266WiFi.h
#include <ESP8266WiFi.h>

// Include Firebase ESP8266 library
#include <FirebaseESP8266.h>

// Include FirebaseJson library
#include <ArduinoJson.h>

// Include AsciiMassage libraries
#include <AsciiMassagePacker.h>
#include <AsciiMassageParser.h>

#define FIREBASE_HOST "phys-labs.firebaseio.com"
#define FIREBASE_AUTH "is9YhJowq75WQr9Ioc2M6X3714p4owRZZSXdPiYA"

// Gorchakov
#define WIFI_SSID "ELTEX-F8BB"
#define WIFI_PASSWORD "GP21326554"
// Mobile WiFi
// #define WIFI_SSID "Neffos X1 Lite"
// #define WIFI_PASSWORD "12345678"
// Home WiFi
// #define WIFI_SSID "TP-Link_790C"
// #define WIFI_PASSWORD "16956483"
//#define WIFI_SSID "4G-UFI-954054"
//#define WIFI_PASSWORD "DaniIl_1998"
// 324 WiFi
//#define WIFI_SSID "koef-324"
//#define WIFI_PASSWORD "A$pddmIedp"

// 2. Define FirebaseESP8266 data object for data sending and receiving
FirebaseData fbdo;
FirebaseData fbdoForStream;

// Define FirebaseJson data object
FirebaseJson jsonXY;

FirebaseJsonData result;

// Define Ascii Massage Parser and Packer objects
AsciiMassageParser inbound;
AsciiMassagePacker outbound;

String URL_TO_LAB = "/labs/kundt";
String URL_TO_USER_ID = "/labs/kundt/userId";
String URL_TO_CHART_ID = "/labs/kundt/chartId";
String URL_TO_IS_READY = "/labs/kundt/isReady";
String URL_TO_IS_TIME_OUT = "/labs/kundt/isTimeOut";
String URL_TO_FREQUENCIES = "/labs/kundt/frequencies";
String URL_TO_SWITCH_STATE = "/labs/kundt/switchState";

int frequency = 2000;
String userId = "";
String chartId = "";
String URL_TO_CHART = "";
bool toExecute = false;
// показывает, что можно посылать следующую частоту на ATMega
bool isOk = true;

//DynamicJsonDocument doc(1024);
FirebaseJsonArray frequencies;

unsigned long period_time = (long)600000;
// переменная таймера, максимально большой целочисленный тип (он же uint32_t)
unsigned long my_timer = millis();

void setup() {
  // start HW serial for ESP8266 (change baud depending on firmware)
  Serial.begin(115200);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
       would try to act as both a client and an access-point and could cause
       network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // 3. Set your Firebase info
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // 4. Enable auto reconnect the WiFi when connection lost
  Firebase.reconnectWiFi(true);

  // 6. Try to get int data from Firebase
  // The get function returns bool for the status of operation
  // fbdo requires for receiving the data
  if (Firebase.getString(fbdo, URL_TO_USER_ID)) {
    // Success
    userId = fbdo.stringData().substring(1, fbdo.stringData().length() - 1);
    Serial.print("User: ");
    Serial.println(userId);
  } else {
    // Failed?, get the error reason from fbdo
    Serial.print("Error in getString, ");
    Serial.println(fbdo.errorReason());
  }

  if (Firebase.getString(fbdo, URL_TO_CHART_ID)) {
    // Success
    chartId = fbdo.stringData().substring(1, fbdo.stringData().length() - 1);
    Serial.print("Chart ID: ");
    Serial.println(chartId);
  } else {
    // Failed?, get the error reason from fbdo
    Serial.print("Error in getString, ");
    Serial.println(fbdo.errorReason());
  }

  URL_TO_CHART += "/charts/kundt/users/" + userId + "/" + chartId;

  Serial.println("Chart Path: " + URL_TO_CHART);
  my_timer = millis();

  // 5. Try to set int data to Firebase
  // The set function returns bool for the status of operation
  // fbdo requires for sending the data
  if (Firebase.setBool(fbdo, URL_TO_IS_READY, true)) {
    // Success
    Serial.println("Ready to go");
  } else {
    // Failed?, get the error reason from fbdo
    Serial.print("Error in setBool, ");
    Serial.println(fbdo.errorReason());
  }

  /* In setup(), set the stream callback function to handle data
    streamCallback is the function that called when database data changes or updates occurred
    streamTimeoutCallback is the function that called when the connection between the server
    and client was timeout during HTTP stream */
  Firebase.setStreamCallback(fbdoForStream, streamCallback, streamTimeoutCallback);

  //In setup(), set the streaming path to "/status/bpState" and begin stream connection
  if (!Firebase.beginStream(fbdoForStream, URL_TO_FREQUENCIES)) {
    //Could not begin stream connection, then print out the error detail
    Serial.println(fbdoForStream.errorReason());
  }
}

// Global function that handles stream data
void streamCallback(StreamData data) {

  // Print out all information
  Serial.println("Stream Data...");
  Serial.println(data.streamPath());
  Serial.println(data.dataPath());
  Serial.println(data.dataType());

  if (data.dataType() == "array") {
    frequencies = data.jsonArray();
    Serial.println("frequencies ");
    frequencies.toString(Serial, true);
    Serial.print("size: ");
    Serial.println(frequencies.size());
  }

  if (data.dataType() == "null" && data.dataPath() == "/") {
    frequencies.clear();
    Serial.println("frequencies ");
    frequencies.toString(Serial, true);
    Serial.print("size: ");
    Serial.println(frequencies.size());
  } else if (data.dataType() == "null") {
    Serial.println("[" + data.dataPath().substring(1, data.dataPath().length()) + "]");
    frequencies.remove("[" + data.dataPath().substring(1, data.dataPath().length()) + "]");
    Serial.println("frequencies ");
    frequencies.toString(Serial, true);
    Serial.print("size: ");
    Serial.println(frequencies.size());
  }
}



//Global function that notifies when stream connection lost
//The library will resume the stream connection automatically
void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    //Stream timeout occurred
    Serial.println("Stream timeout, resume streaming...");
  }
}

void loop(void) {
  if (frequencies.size() != 0 && isOk && frequencies.size() != 0) {
    my_timer = millis();
    frequencies.get(result, frequencies.size() - 1);
    // Передаем значение ATmega328
    outbound.streamOneInt(&Serial, "f", result.to<int>()); // End the packet and stream it.
    outbound.streamEmpty(&Serial, "");
    isOk = false;
  }


  if ( inbound.parseStream( &Serial ) ) {
    // parse completed massage elements here.
    if ( inbound.fullMatch ("v") ) {
      // Get the first long.
      long value = inbound.nextLong();

      jsonXY.set("x", result.to<int>());
      jsonXY.set("y", String(value));

      if (Firebase.pushJSON(fbdo, URL_TO_CHART, jsonXY)) {
        Serial.println(fbdo.dataPath());
        Serial.println(fbdo.pushName());
        Serial.println(fbdo.dataPath() + "/" + fbdo.pushName());
      } else {
        Serial.println(fbdo.errorReason());
      }

      if (Firebase.deleteNode(fbdo, URL_TO_FREQUENCIES + "/" + (frequencies.size() - 1))) {
        Serial.println(fbdo.dataPath());
        Serial.println(fbdo.pushName());
        Serial.println(fbdo.dataPath() + "/" + fbdo.pushName());
      } else {
        Serial.println(fbdo.errorReason());
      }
      isOk = true;
    }
  }

  if (millis() - my_timer > period_time) {
    userId = "";
    Firebase.deleteNode(fbdo, URL_TO_FREQUENCIES);
    Firebase.setString(fbdo, URL_TO_USER_ID, "");
    Firebase.setBool(fbdo, URL_TO_IS_READY, false);
    Firebase.setBool(fbdo, URL_TO_IS_TIME_OUT, true);
    Firebase.setString(fbdo, URL_TO_SWITCH_STATE, "off");
  }
}
