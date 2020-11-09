//вариант без библиотеки с прерыванием
//Выводы шагового двигателя
#define OUTA 8
#define OUTB 9
#define OUTC 10
#define OUTD 11
#define holdLengthMicroseconds 100
unsigned long currentTime;
unsigned long loopTime;
short currentStep = 0; // Текущий шаг
const byte dynPin=2;
//генерация звука на втором пине
const int IN_PIN = A0;
//прием звука на анлоговом пине а0
const int MAX_ADC_VALUE = 1023;//возможно изменить на 255
int frec;
float amp;
int chr;                           //здесь будет храниться принятый символ
String MyStr;
char myStr[15];
int st=0;
void setup()
{   initStepper();   
    Serial.begin(9600);
//установка порта на скорость 9600 бит/сек
pinMode(dynPin,OUTPUT);
  pinMode(IN_PIN, INPUT);
frec=3000;//исходное значение частоты 3000 Гц
  }
void loop()
{ 
    if (Serial.available()) {         //если есть принятый символ,
    chr = Serial.read();            //  то читаем его
    if (chr == 'H') { //проверяем его на нужную команду
      //инициализируем переменные для определения размаха
 for(int i=0;i<128;i++){
    rewStep();
        currentTime=millis();
while ((millis()-currentTime)<300){};    
        
      }
   //шагнули на 4 шага и делаем измерение
int mn = 1024;
int mx = 0;
tone(dynPin,frec);//запускаем генератор звука
for (int j=0; j < 1000; ++j) {//ищем макс и мин значения амплитуды
int val = analogRead(IN_PIN);
mn = min(mn, val);
mx = max(mx, val);
}
//нашли размах амплитуды
 amp = (float) (5.0 / MAX_ADC_VALUE* (mx - mn));
     dtostrf(amp, 5, 3, myStr);  // выводим в строку myStr 1 разряд до, 3 после 
  MyStr = myStr;
  MyStr='<'+MyStr+'>';
   Serial.println(MyStr);
noTone(dynPin);//отключаем звук
 st++;
          }
    }
}

void initStepper()
{
  //Настраиваем выводы
  pinMode(OUTA, OUTPUT);
  pinMode(OUTB, OUTPUT);
  pinMode(OUTC, OUTPUT);
  pinMode(OUTD, OUTPUT);
  off();
}
void setStepleft(short step)
{
  switch (step % 4)
  {
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
void setStepright(short step)
{
  switch (step % 4)
  {
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

void off()
{
  //Выключаем все контакты
  digitalWrite(OUTA, 0);
  digitalWrite(OUTB, 0);
  digitalWrite(OUTC, 0);
  digitalWrite(OUTD, 0);
}
void gor()
{
  setStepright(currentStep);
  delayMicroseconds(holdLengthMicroseconds);
   }
   void gol()
{
  setStepleft(currentStep);
  delayMicroseconds(holdLengthMicroseconds);
   }
void fwdStep()
{
  currentStep = currentStep % 4;
  gor();currentStep++;
}
void rewStep()
{
  currentStep = currentStep % 4;
  gol();currentStep++;
}



