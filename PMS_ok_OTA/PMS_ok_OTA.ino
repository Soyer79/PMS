#include <Wire.h>
#include <SoftwareSerial.h>
SoftwareSerial swSer(D5, D6, false, 256);
#include "PMS.h"
#include <Timers.h>
#include <SimpleDHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

int pinDHT11 = D4;
SimpleDHT11 dht11(pinDHT11);
char auth[] = "4fc87431c0ec4c1e90778d8c5765b1c8";
char ssid[] = "eiNET_eu_035A9922";
char pass[] = "305937442";

Timer sekunda;//led
Timer minutDwie;//
Timer sekundPiec;
Timer sekundTrzydziesci;
Timer minuta;
//Timer minutPiec;
Timer minutDziesiec;
Timer sekundDziesiec;
Timer klapka;

PMS pms(swSer);
PMS::DATA data;

byte temperatura;
byte wilgotnosc;

RTC_DS1307 rtc;

#define treconnect 30 //czasy wywoływania Blynk.run przy braku łączności w sek
int liczreconnect = 10;

enum FIRST { ONE=1,TWO,THREE,FOUR,FIVE,SIX,SEVEN,EIGHT,NINE,TEN};
int menu=THREE;
LiquidCrystal_I2C lcd(0x3F,16,2);
#define pin1silnika D7
#define pin2silnika D8
#define pinFan D3
#define pinPrzekaznik D0
WidgetLED ledNode(V41);
int blueLED = 0;

boolean dhtStart = 0;

void setup()
{
  Wire.begin(D2,D1);
  Serial.begin(9600);   // GPIO1, GPIO3 (TX/RX pin on ESP-12E Development Board)
  swSer.begin(9600);
  WiFi.begin(ssid, pass);
  Blynk.begin(auth, ssid, pass);
  
  Blynk.config(auth, IPAddress(139, 59, 206, 133));
  Blynk.connect(); 
  ArduinoOTA.setHostname("PMS5003_ESP");   
  ArduinoOTA.begin();    
 //rtc.adjust(DateTime(__DATE__, __TIME__));
  rtc.begin();
  pms.passiveMode();    // Switch to passive mode
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  pinMode(pin1silnika, OUTPUT); //Sygnały sterujące kierunkiem obrotów silnika nr 1
  pinMode(pin2silnika, OUTPUT);
  pinMode(pinFan, OUTPUT);
  digitalWrite(pinFan, HIGH);
  sekunda.begin(995);
  minutDwie.begin(120000);//spr. DHT
  sekundPiec.begin(4850);//przeł. one<->two
  sekundTrzydziesci.begin(9998);//po wybudzeniu PMS
  minutDziesiec.begin(600000);//uspienie PMS
  minuta.begin(59987);//went
  sekundDziesiec.begin(9689);//blynk run
  klapka.begin(3000);//klapka
  //minutPiec.begin(299895);
}
void loop() {
  ArduinoOTA.handle();
  miganieLED();
  Blynk.run();
switch(menu){
  case ONE:
   
   one();
   checkTimeAndHumidity();
   if(sekundPiec.available()){
    lcd.clear();
    sekundPiec.restart();
    menu=TWO;
   }
  break;
  case TWO:
   two();
   checkTimeAndHumidity();
   if(sekundPiec.available()){
    lcd.clear();
    sekundPiec.restart();
    menu=ONE;
   }
  break;
  case THREE:
   three();
  break;
  case FOUR:
   four();
  break;
  case FIVE:
   five();
  break;
  case SIX:
   six();
  break;
  case SEVEN:
   seven();
  break;
  case EIGHT:
   eight();
  break;
  case NINE:
   nine();
  break;
  case TEN:
  ten();
  break;
}
}
 
void one(){
  DateTime now = rtc.now();
  lcd.setCursor(0,0);
  lcd.print("      ");
  if(now.hour()<10){
    lcd.print("0");
  }
  lcd.print(now.hour(), DEC);
  lcd.print(":");
  if(now.minute()<10){
    lcd.print("0");
  }
  lcd.print(now.minute(), DEC);
  lcd.setCursor(0,1);
  lcd.print("   ");
  if(now.day()<10){
    lcd.print("0");
  }
  lcd.print(now.day(), DEC);
  lcd.print('.');
  lcd.print(now.month(), DEC);
  if(now.month()<10){
    lcd.print("0");
  }
  lcd.print('.');
  lcd.print(now.year(), DEC);
 }
 void two(){
  lcd.setCursor(0,0);
  lcd.print("PM1 ");
  lcd.print(data.PM_AE_UG_1_0);
  lcd.print(" PM2,5 ");
  lcd.print(data.PM_AE_UG_2_5);
  lcd.setCursor(0,1);
  lcd.print("    PM 10  ");
  lcd.print(data.PM_AE_UG_10_0);
 }
 void three(){
  lcd.setCursor(0,0);
  lcd.print("  Wilg. < 70%");
  lcd.setCursor(0,1);
  lcd.print("  Wybudzam PMS");
  
  pms.wakeUp(); //30 sekund poczekaj
  if(sekundTrzydziesci.available()){
    lcd.clear();
    sekundPiec.restart();
    menu=FOUR;
  }
 }
void four(){
  
  lcd.setCursor(0,0);
  lcd.print("      PMS");
  lcd.setCursor(0,1);
  lcd.print("wykonuje pomiar");
  pms.requestRead();
  pms.readUntil(data);
  if(sekundPiec.available()){
    Blynk.virtualWrite(35,"   ",data.PM_AE_UG_1_0," ","ug/m3");
    Blynk.virtualWrite(36,"   ",data.PM_AE_UG_2_5," ","ug/m3");
    Blynk.virtualWrite(37,"   ",data.PM_AE_UG_10_0," ","ug/m3");
    Blynk.virtualWrite(38,data.PM_AE_UG_1_0);
    Blynk.virtualWrite(39,data.PM_AE_UG_2_5);
    Blynk.virtualWrite(40,data.PM_AE_UG_10_0);
    lcd.clear();
    sekundPiec.restart();
    menu=FIVE;
  }
 }
 void five(){
  
  lcd.setCursor(0,0);
  lcd.print("Usypiam czujnik");
  lcd.setCursor(0,1);
  lcd.print("  na 10 minut");
  pms.sleep();
  if(sekundPiec.available()){
    lcd.clear();
    sekundPiec.restart();
    klapka.restart();
    menu=SIX;
  }
 }
 void six(){
  
  lcd.setCursor(0,0);
  lcd.print("Otwieram doplyw");
  lcd.setCursor(0,1);
  lcd.print("   powietrza");
  digitalWrite(pin1silnika, LOW); //Silnik nr 1 - obroty w lewo
  digitalWrite(pin2silnika, HIGH); 
  if(klapka.available()){
  digitalWrite(pin1silnika, LOW); //Silnik nr 1 - obroty stop
  digitalWrite(pin2silnika, LOW); 
  }
   if(sekundPiec.available()){
    lcd.clear();
    minuta.restart();
    menu=SEVEN;
  }
 }
 void seven(){
  
  lcd.setCursor(0,0);
  lcd.print("   Wentylator");
  lcd.setCursor(0,1);
  lcd.print("    wlaczony");
  digitalWrite(pinFan, LOW);
  if(minuta.available()){
    lcd.clear();
     sekundPiec.restart();
    menu=EIGHT;
  }
  
 }
 void eight(){
 
  lcd.setCursor(0,0);
  lcd.print("   Wentylator");
  lcd.setCursor(0,1);
  lcd.print("   wylaczony");
  digitalWrite(pinFan, HIGH);
  if(sekundPiec.available()){
    lcd.clear();
     sekundPiec.restart();
     dhtStart=false;
     klapka.restart();
    menu=NINE;
  }
 
 }
 void nine(){
 
  lcd.setCursor(0,0);
  lcd.print(" Zamykam doplyw");
  lcd.setCursor(0,1);
  lcd.print("   powietrza");
  digitalWrite(pin1silnika, HIGH); //Silnik nr 1 - obroty w lewo
  digitalWrite(pin2silnika, LOW); 
  if(klapka.available()){
  digitalWrite(pin1silnika, LOW); //Silnik nr 1 - obroty stop
  digitalWrite(pin2silnika, LOW); 
  }
  if(dhtStart==false){
    dht11.read(&temperatura, &wilgotnosc, NULL);
   Serial.println(temperatura);
   Serial.println(wilgotnosc);
   dhtStart=true;
  }
  if(sekundPiec.available()){
    lcd.clear();
    sekundPiec.restart();
    menu=TEN;
  }
 }
 void ten(){
  
  lcd.setCursor(0,0);
  lcd.print("   Temp.  ");
  lcd.print(temperatura);
  lcd.print(" *C");
  lcd.setCursor(0,1);
  lcd.print("   Wilg.  ");
  lcd.print(wilgotnosc);
  lcd.print(" %RH");
  if(sekundPiec.available()){
    lcd.clear();
    sekundPiec.restart();
    menu=ONE;
  }
 }
 void checkTimeAndHumidity(){
  if(minutDwie.available()){
   dht11.read(&temperatura, &wilgotnosc, NULL);
  Serial.println(temperatura);
  Serial.println(wilgotnosc);
  minutDwie.restart();
  }
  if((minutDziesiec.available()) && (wilgotnosc < 77)){
    lcd.clear();
    sekundTrzydziesci.restart();
    minutDziesiec.restart();
    menu=THREE;
  }
 }

void miganieLED(){
  if(sekunda.available()){
   blueLED = !blueLED ;
   if (blueLED == 0) {
    ledNode.off(); //miganie vLED dla kontroli połączenia
   }
     else {
    ledNode.on();
    }
    sekunda.restart();
}
}
