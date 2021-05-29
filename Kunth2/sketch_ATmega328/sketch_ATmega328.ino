#include <AsciiMassagePacker.h>
#include <AsciiMassageParser.h>

//вариант без библиотеки с прерыванием
//Выводы шагового двигателя
#define OUTA 8
#define OUTB 9
#define OUTC 10
#define OUTD 11
#define holdLengthMicroseconds 100

AsciiMassageParser inbound;
AsciiMassagePacker outbound;

const int microphonePin = A0;
const int SoundPin = 2;
float microphoneValue;
int frequency = 3000;

unsigned long currentTime;
unsigned long loopTime;
short currentStep = 0; // Текущий шаг

const int MAX_ADC_VALUE = 1023;//возможно изменить на 255
String valueStr; ///?

int st = 0;//?

void setup() {
  initStepper();
  pinMode(SoundPin, OUTPUT);
  pinMode(microphonePin, INPUT);
  Serial.begin(115200);
}

void loop() {
  if ( inbound.parseStream( &Serial ) ) {
    // parse completed massage elements here.
    // Does the massage's address match "value"?
    if ( inbound.fullMatch ("H") ) {
      //шагнули на 4 шага и делаем измерение
      for (int i = 0; i < 128; i++) {
        rewStep();
        currentTime = millis();
        while ((millis() - currentTime) < 300) {};
      }

      //инициализируем переменные для определения размаха
      int mn = 1024;
      int mx = 0;
      tone(SoundPin, frequency); //запускаем генератор звука
      for (int i = 0; i < 1000; ++i) { //ищем макс и мин значения амплитуды
        int val = analogRead(microphonePin);
        mn = min(mn, val);
        mx = max(mx, val);
      }
      //нашли размах амплитуды
      microphoneValue = (float) (5.0 / MAX_ADC_VALUE * (mx - mn));
      //      dtostrf(microphoneValue, 5, 3, valueStr);  // выводим в строку myStr 1 разряд до, 3 после

      outbound.beginPacket("V"); // Start a packet with the address called "value".
      outbound.addFloat(microphoneValue); // Add a reading of analog 0.
      outbound.streamPacket(&Serial); // End the packet and stream it.
      outbound.streamEmpty(&Serial, "");
      noTone(SoundPin);
    }
  }
}

void initStepper() {
  //Настраиваем выводы
  pinMode(OUTA, OUTPUT);
  pinMode(OUTB, OUTPUT);
  pinMode(OUTC, OUTPUT);
  pinMode(OUTD, OUTPUT);
  off();
}

void setStepleft(short step) {
  switch (step % 4) {
    case 0:
      digitalWrite(OUTA, 1);
      digitalWrite(OUTB, 0);
      digitalWrite(OUTC, 1);
      digitalWrite(OUTD, 0);
      break;
    case 1:
      digitalWrite(OUTA, 0);
      digitalWrite(OUTB, 1);
      digitalWrite(OUTC, 1);
      digitalWrite(OUTD, 0);
      break;
    case 2:
      digitalWrite(OUTA, 0);
      digitalWrite(OUTB, 1);
      digitalWrite(OUTC, 0);
      digitalWrite(OUTD, 1);
      break;
    case 3:
      digitalWrite(OUTA, 1);
      digitalWrite(OUTB, 0);
      digitalWrite(OUTC, 0);
      digitalWrite(OUTD, 1);
      break;
  }
}

void setStepright(short step) {
  switch (step % 4) {
    case 0:
      digitalWrite(OUTA, 1);
      digitalWrite(OUTB, 0);
      digitalWrite(OUTC, 0);
      digitalWrite(OUTD, 1);
      break;
    case 1:
      digitalWrite(OUTA, 0);
      digitalWrite(OUTB, 1);
      digitalWrite(OUTC, 0);
      digitalWrite(OUTD, 1);
      break;
    case 2:
      digitalWrite(OUTA, 0);
      digitalWrite(OUTB, 1);
      digitalWrite(OUTC, 1);
      digitalWrite(OUTD, 0);
      break;
    case 3:
      digitalWrite(OUTA, 1);
      digitalWrite(OUTB, 0);
      digitalWrite(OUTC, 1);
      digitalWrite(OUTD, 0);
      break;
  }
}

void off() {
  //Выключаем все контакты
  digitalWrite(OUTA, 0);
  digitalWrite(OUTB, 0);
  digitalWrite(OUTC, 0);
  digitalWrite(OUTD, 0);
}

void gor() {
  setStepright(currentStep);
  delayMicroseconds(holdLengthMicroseconds);
}

void gol() {
  setStepleft(currentStep);
  delayMicroseconds(holdLengthMicroseconds);
}

void fwdStep() {
  currentStep = currentStep % 4;
  gor(); currentStep++;
}
void rewStep() {
  currentStep = currentStep % 4;
  gol(); currentStep++;
}
