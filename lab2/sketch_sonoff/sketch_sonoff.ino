#include <ESP8266WiFi.h>
#include "FirebaseESP8266.h"

#define FIREBASE_HOST "phys-labs.firebaseio.com"
#define FIREBASE_AUTH "is9YhJowq75WQr9Ioc2M6X3714p4owRZZSXdPiYA"
#define WIFI_SSID "Neffos X1 Lite"
#define WIFI_PASSWORD "12345678"
//#define WIFI_SSID "koef-324"
//#define WIFI_PASSWORD "A$pddmIedp"

//Define Firebase Data objects
FirebaseData firebaseData;

String power = "off";

int gpio13Led = 13;
int gpio12Relay = 12;

void setup(void) {
  // preparing GPIOs
  pinMode(gpio13Led, OUTPUT);
  digitalWrite(gpio13Led, HIGH);

  pinMode(gpio12Relay, OUTPUT);
  digitalWrite(gpio12Relay, HIGH);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //  Serial.println("");

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

  if (!Firebase.beginStream(firebaseData, "/labs/2/status/power")) {
    //    Serial.println("Could not begin stream");
    //    Serial.println("REASON: " + firebaseData1.errorReason());
    //    Serial.println();
  }
}

void loop(void) {

  if (!Firebase.readStream(firebaseData)) {
    //    Serial.println();
    //    Serial.println("Can't read stream data");
    //    Serial.println("REASON: " + firebaseData1.errorReason());
    //    Serial.println();
  }

  if (firebaseData.streamTimeout()) {
    //    Serial.println();
    //    Serial.println("Stream timeout, resume streaming...");
    //    Serial.println();
  }

  if (firebaseData.streamAvailable()) {
    if (Firebase.getString(firebaseData, "/labs/2/status/power", power)) {
      if (power == "off") {
        digitalWrite(gpio13Led, HIGH);
        digitalWrite(gpio12Relay, LOW);
        delay(1000);
      } else if (power == "on") {
        digitalWrite(gpio13Led, LOW);
        digitalWrite(gpio12Relay, HIGH);
        delay(1000);
      }
    } else {
      // Serial.println(firebaseData1.errorReason());
    }
  }
}
