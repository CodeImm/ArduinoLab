#include <AsciiMassagePacker.h>
#include <AsciiMassageParser.h>

#include <Stepper.h>

AsciiMassageParser inbound;
AsciiMassagePacker outbound;

const int BUTTON_PIN = 7; // the Arduino's input pin that connects to the sensor's SIGNAL pin

const int STEPS_PER_REVOLUTION = 960;
const int MAX_ADC_VALUE = 1023;//возможно изменить на 255
const int cm = STEPS_PER_REVOLUTION / 4;
const int mm = STEPS_PER_REVOLUTION / 40;
int count = 0;

bool isInitialized = false;

const int microphonePin = A0;
const int SOUND_PIN = 2;
float microphoneValue = 0;
int frequency = 5000;
int engineStep = 10;

Stepper myStepper(STEPS_PER_REVOLUTION, 8, 9, 10, 11); //подключение к пинам 8…11 на Ардуино

void setup() {
  // initialize the Arduino's pin as aninput
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  myStepper.setSpeed(60); //установка скорости вращения ротора
  Serial.begin(115200);
}

void loop() {
  // read the state of the the input pin:
  int buttonState = digitalRead(BUTTON_PIN);

  // initializing of stepper
  if (isInitialized == false) {
    // print state to Serial Monitor
    Serial.println(buttonState);
    myStepper.step(cm / 5);
    delay(100);
  }

  if (buttonState == 0 && !isInitialized) {
    isInitialized = true;
    outbound.beginPacket("I");
    outbound.streamPacket(&Serial);
    outbound.streamEmpty(&Serial, "");
  }

  if ( inbound.parseStream( &Serial ) ) {
    if ( inbound.fullMatch ("F") ) {
      frequency = inbound.nextInt();
    }

    if ( inbound.fullMatch ("S") ) {
      engineStep = inbound.nextInt() * mm;
    }

    // parse completed massage elements here.
    // Does the massage's address match "value"?
    if ( inbound.fullMatch ("L") ) {
      Serial.print("Move left "); //по часовой стрелке
      Serial.println( ++count);
      myStepper.step(-engineStep);

      delay(1000);
      if (frequency >= 30 && frequency <= 10000) {
        //инициализируем переменные для определения размаха
        int mn = 1024;
        int mx = 0;
        tone(SOUND_PIN, frequency); //запускаем генератор звука
        for (int i = 0; i < 1000; ++i) { //ищем макс и мин значения амплитуды
          int val = analogRead(microphonePin);
          mn = min(mn, val);
          mx = max(mx, val);
        }
        //нашли размах амплитуды
        microphoneValue = (float) (5.0 / MAX_ADC_VALUE * (mx - mn));
        Serial.print("Размах амплитуды: ");
        Serial.println(mx - mn);

        outbound.beginPacket("V"); // Start a packet with the address called "value".
        outbound.addFloat(microphoneValue); // Add a reading of analog 0.
        outbound.streamPacket(&Serial); // End the packet and stream it.
        outbound.streamEmpty(&Serial, "");
        noTone(SOUND_PIN);
      }
    }

    if ( inbound.fullMatch ("RS") ) {
      Serial.print("Move right "); //по часовой стрелке
      Serial.println( ++count);
      myStepper.step(cm);
    }

    if ( inbound.fullMatch ("LS") ) {
      Serial.print("Move left "); //по часовой стрелке
      Serial.println( ++count);
      myStepper.step(-cm);
    }
  }
}
