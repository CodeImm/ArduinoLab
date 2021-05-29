#include <AsciiMassagePacker.h>
#include <AsciiMassageParser.h>

AsciiMassageParser inbound;
AsciiMassagePacker outbound;

const int microphonePin = A0;
const int SoundPin = 9;
long microphoneValue = 0;
int frequency = 5000;

void setup() {
  Serial.begin(115200);
}

void loop() {
  if ( inbound.parseStream( &Serial ) ) {
    // parse completed massage elements here.
    // Does the massage's address match "value"?
    if ( inbound.fullMatch ("f") ) {
      frequency = inbound.nextInt();
      if (frequency >= 30 && frequency <= 10000) {
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
        microphoneValue = mx - mn;

        outbound.beginPacket("v"); // Start a packet with the address called "value".
        outbound.addLong(microphoneValue); // Add a reading of analog 0.
        outbound.streamPacket(&Serial); // End the packet and stream it.
        outbound.streamEmpty(&Serial, "");
        noTone(SoundPin);
      }
    }
  }
}
