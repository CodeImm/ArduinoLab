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

// Mobile WiFi
// #define WIFI_SSID "Neffos X1 Lite"
// #define WIFI_PASSWORD "12345678"


// 2. Define FirebaseESP8266 data object for data sending and receiving
FirebaseData fbdo;
FirebaseData fbdoForStream;

// Define FirebaseJson data object
FirebaseJson jsonXY;

// Define Ascii Massage Parser and Packer objects
AsciiMassageParser inbound;
AsciiMassagePacker outbound;

int frequency = 2000;
String path = "/status";
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
  if (Firebase.getString(fbdo, "/status/currentUser")) {
    // Success
    user = fbdo.stringData();
    Serial.print("User: ");
    Serial.println(user);
  } else {
    // Failed?, get the error reason from fbdo
    Serial.print("Error in getString, ");
    Serial.println(fbdo.errorReason());
  }

  chartPath = "/users/" + user;

  if (Firebase.getString(fbdo, "/status/chartId")) {
    // Success
    chartId = fbdo.stringData();
    Serial.print("Chart ID: ");
    Serial.println(chartId);
  } else {
    // Failed?, get the error reason from fbdo
    Serial.print("Error in getString, ");
    Serial.println(fbdo.errorReason());
  }

  chartPath += "/charts/" + chartId;

  Serial.println("Chart Path: " + chartPath);
  my_timer = millis();

  // 5. Try to set int data to Firebase
  // The set function returns bool for the status of operation
  // fbdo requires for sending the data
  if (Firebase.setString(fbdo, path + "/" + "isReady", "true")) {
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
  if (!Firebase.beginStream(fbdoForStream, "/status/bpState")) {
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

  //Print out the value
  Serial.println("bpState Stream " + data.stringData());

  if ( isOk == "true") {
    isSignal = data.stringData();
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

  if (isSignal == "true") {
    isSignal = "false";
    isOk = "false";
    my_timer = millis();   // "сбросить" таймер

    // Получаем частоту сигнала
    if (Firebase.getInt(fbdo, "/status/frequency")) {
      // Success
      frequency = fbdo.intData();
      Serial.print("frequency: ");
      Serial.println(frequency);
    } else {
      // Failed?, get the error reason from fbdo
      Serial.print("Error in getInt, ");
      Serial.println(fbdo.errorReason());
    }

    // Передаем значение ATmega328
    outbound.streamOneInt(&Serial, "f", frequency); // End the packet and stream it.
    outbound.streamEmpty(&Serial, "");
  }

  if ( inbound.parseStream( &Serial ) ) {
    // parse completed massage elements here.
    if ( inbound.fullMatch ("v") ) {
      // Get the first long.
      long value = inbound.nextLong();

      String valueStr = String(value);

      jsonXY.set("x", frequency);
      jsonXY.set("y", valueStr);

      if (Firebase.setString(fbdo, path + "/" + bpStateNodeID, "false")) {
        // Success
      } else {
        // Failed?, get the error reason from fbdo
        Serial.print("Error in setString, ");
        Serial.println(fbdo.errorReason());
      }

      if (Firebase.pushJSON(fbdo, chartPath, jsonXY)) {
        Serial.println(fbdo.dataPath());
        Serial.println(fbdo.pushName());
        Serial.println(fbdo.dataPath() + "/" + fbdo.pushName());
      } else {
        Serial.println(fbdo.errorReason());
      }

      isOk = "true";
    }
  }

  if (millis() - my_timer > period_time) {
    user = "null";
    Firebase.setString(fbdo, "/status/currentUser", "null");
    Firebase.setString(fbdo, path + "/" + "isReady", "false");
    Firebase.setString(fbdo, path + "/" + "isTimeOut", "true");
    Firebase.setString(fbdo, path + "/" + "power", "off");
  }
}
