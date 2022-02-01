#include <AsciiMassagePacker.h>
#include <AsciiMassageParser.h>

#include <Stepper.h>
#include <Capacitor.h>

AsciiMassageParser inbound;
AsciiMassagePacker outbound;

/*
A0 - 
A2 - 
13 pin -
8, 9, 10, 11 - stepper
*/

const int OUT_PIN = A2;
const int IN_PIN = A0;
const int BUTTON_PIN = 7;  // the Arduino's input pin that connects to the sensor's SIGNAL pin


const int STEPS_PER_REVOLUTION = 48;
float capacitance;
// int angle = 0;
int count = 0;
int engineStep;  // 1 - 2.5 градуса
// int degreeStep = 2;

bool isInitialized = false;
bool firstStep = true;

Stepper myStepper(STEPS_PER_REVOLUTION, 8, 9, 10, 11);  //подключение к пинам 8…11 на Ардуино
Capacitor cap1(OUT_PIN, IN_PIN);

void setup() {
  // initialize the Arduino's pin as aninput
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // pinMode(OUT_PIN, OUTPUT);
  // pinMode(IN_PIN, OUTPUT);

  myStepper.setSpeed(60);  //установка скорости вращения ротора
  Serial.begin(115200);
}

void loop() {
  // read the state of the the input pin:
  int buttonState = digitalRead(BUTTON_PIN);

  // initializing of stepper
  if (isInitialized == false) {
    // print state to Serial Monitor
    Serial.println(buttonState);
    myStepper.step(2);
    delay(10);
  }

  if (buttonState == 0 && !isInitialized) {
    isInitialized = true;
    firstStep = true;
    outbound.beginPacket("I");
    outbound.streamPacket(&Serial);
    outbound.streamEmpty(&Serial, "");
  }

  if (inbound.parseStream(&Serial)) {
    if (inbound.fullMatch("I")) {
      isInitialized = false;
    }

    if (inbound.fullMatch("S")) {
      engineStep = inbound.nextInt();
    }

    // parse completed massage elements here.
    // Does the massage's address match "value"?
    if (inbound.fullMatch("R")) {
      Serial.print("Move right ");  //по часовой стрелке
      Serial.println(++count);
      Serial.print("!firstStep^ ");
      Serial.println(!firstStep);
      if (firstStep) {
        firstStep = false;
          // повернуть до начала соприкосновения пластин
          myStepper.step(-7);
          delay(5000);
      } else {
        myStepper.step(-engineStep);
      }

      // delay(4000);

  capacitance = cap1.Measure();

        Serial.print("Ёмкость: ");
        Serial.println(capacitance);

        outbound.beginPacket("V");       // Start a packet with the address called "value".
        outbound.addFloat(capacitance);  // Add a reading of analog 0.
        outbound.streamPacket(&Serial);  // End the packet and stream it.
        outbound.streamEmpty(&Serial, "");
      
    }

    if (inbound.fullMatch("RS")) {
      Serial.print("Move right ");  //по часовой стрелке
      Serial.println(++count);
      myStepper.step(1);
    }

    if (inbound.fullMatch("LS")) {
      Serial.print("Move left ");  //по часовой стрелке
      Serial.println(++count);
      myStepper.step(-1);
    }
  }
}
