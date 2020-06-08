int microphonePin = A0;
const int SoundPin = 9;
int delaySound = 1000;
int microphoneValue = 0;
int chr;
int val;
String inString = ""; 
int frequency=5000;

void setup() {
  //delay(delaySound);
  //noTone(SoundPin);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {        //если есть принятый символ,
    //chr = Serial.read();    //  то читаем его
    inString = Serial.readString();
    val = inString.toInt();
//    Serial.println(val);
    if (val>32 && val<10000){
      frequency=val;
//    }
//    if (val == 30) { //проверяем его на нужную команду
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
      Serial.println(microphoneValue, DEC);
      delay(3000);
      noTone(SoundPin);
    }
  }
}
