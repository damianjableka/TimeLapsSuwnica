/**********************************************************************************
 *  moduĹ‚ suwnicy z zaimplementowanymi 
 *  krokami bez delay(), 
 *  przerweaniami na kraĹ„cĂłwkach
 *  wprowadzaniem ustawieĹ„ przez wifi telefonu po resecie
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * ********************************************************************************/

 
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#define IlePinow 4
#define KrokNaSek 4
#define IleZmiennych 13
#define PinKrancowek D3

#include <Arduino.h>
#include <U8g2lib.h>

//Ĺ‚adowanie bibliotek do wyĹ›witlacza oled I2C
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//definicja wyĹ›wietlacza Oled I2C
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display



int piny[IlePinow]={D5,D7,D8,D6};       //piny uzyte do sterowania w odpowiendniej kolejnosci uzytej w sekwencji
int sekwencja[KrokNaSek][IlePinow]={    //sekwencja pinow 
  {HIGH,LOW,LOW,HIGH},
  {HIGH,LOW,HIGH,LOW},
  {LOW,HIGH,HIGH,LOW},
  {LOW,HIGH,LOW,HIGH}};    

    int AktKierunek=0;   //pocztkowy kierunek
    int NowyKierunek=1;  //czy jechac w drugim kierunku?
    float ZebyM=16;     //[n]ilosc zebow na silniku
    float ZebyD=120;    //[N]ilosc zebow na durzej zembatce
    float ZDmm=1;       //[Obr/mm] ilosc obrotow duzej zembatki na 1 mm
    float Dlugosc=1300; //[mm] dlugosc szyny w mm
    float dlugosc_wozka=155; //[mm] dlugosc wozka w mm trzeba ja odjac od dlugosci szyny aby nie uderzyc o koniec!
    float Noc=6;        //[h]dlugosc nocy w godzinach
    float Exp=30;       //[s] co ile nastepuje expozycja, expozycja w aparcie powinna byc krotsza aby dala sie zrobic ruch poza exp
    float KnaO=48;      //  krokow na obrot, katalogowa wartosc  
    
  
    
    float AccelStart=9;  //[ms] pocztkowa wartosc przerwy miedzy krokami
    float Speed=3;        //[ms] zasadnicza przerwa miedzy krokami
    int Accel=1;        //[ms] inkrement/dekrement akceleracji
    int AccelStep=2;    // [n] ilosc krokow w zadanej akceleracji

    int krancowkaA=1; // co zrobic po dotarciu do krancowki 0 stop 1 reverse 
    int start=1;      //czy startowac 0-stop, 1-start
   
    
     int i;
     int j;
     int k;
     int pauza=(int) AccelStart*1000;
  
int KrokNaExp;
  int KrokNaAcc;
  int IleExp;
  float mmnaruch;
bool wyswietlkoniec=1;

String getValue(String data, char separator, int index) //oddaje m-ta "intdex" wartoÄąâ€şĂ„â€ˇ z ciagu znakow "data" rozdzielonego separatorami 
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}


  
int zm_kierunek(int aaa){
  if(aaa!=AktKierunek)
  {              // 2         5
   //int piny[4]={D5,D7,D8,D6};   //odwracamy 4 elementowa tablice
   piny[3]+=piny[0]; //7
   piny[0]=piny[3]-piny[0]; //7 -2 =5
   piny[3]-=piny[0]; //7 -5 =2
   piny[2]+=piny[1]; //7
   piny[1]=piny[2]-piny[1]; //7 -2 =5
   piny[2]-=piny[1]; //7 -5=2

   
   AktKierunek=aaa;
  }
  
  return(AktKierunek);
  }
  

void setup() {
  //inicjalizacjia wyswietlacza
   u8g2.begin();
   u8g2.enableUTF8Print();    // enable UTF8 support for the Arduino print() function
   u8g2.setFont(u8g2_font_mozart_nbp_t_all); // choose a suitable font
  
  // initialize the serial port: w sumie ostatecznie to bedzie niepotrzebne
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(PinKrancowek), krancowka, FALLING);
  
   u8g2.clearBuffer();          // clear the internal memory
   u8g2.drawStr(30,16,"Start");  // write something to the internal memory
   u8g2.drawStr(10,32,"Wszystkie piny:");
   u8g2.drawStr(30,48,"HIGH");
   
   u8g2.sendBuffer();
  
  for(i=0;i<IlePinow;i++){
     pinMode(piny[i], OUTPUT);
     digitalWrite(piny[k], HIGH);
  }
    delay(1000);

    for(uint8_t t = 8; t > 0; t--) {
      Serial.printf("[SETUP] WAIT %d...\n", t);
       Serial.flush();
      u8g2.clearBuffer();    
      u8g2.drawStr(10,16,"Czekam");  
      u8g2.setCursor(50, 32);
      u8g2.print(t);
      u8g2.sendBuffer();
       
      delay(1000);
  }
     
     int linia;
     int zmienne[IleZmiennych];  //tablica zmiennych zdekodowana z pliku konfiguracyjnego w tej samej kolejnosci co zmienne ponizej
      
      u8g2.clearBuffer();          // clear the internal memory
      u8g2.drawStr(10,16,"Lacze z WIFI");  // write something to the internal memory
      
      u8g2.sendBuffer();
      
   WiFi.begin("MEIZU", "1qazxsw2");
   Serial.println(WiFi.localIP());

      u8g2.setCursor(1, 32);
      u8g2.print(WiFi.localIP());
      u8g2.sendBuffer();
      
   HTTPClient http;
   Serial.print("[HTTP] begin...\n");                   // configure traged server and url
   http.begin("http://192.168.43.1:12345/suwnica.txt"); //HTTP
         u8g2.setCursor(0, 48);
    u8g2.print("Lacze z serwerem");  // write something to the internal memory
    u8g2.sendBuffer();
      
   Serial.print("[HTTP] GET...\n");   // start connection and send HTTP header
   int httpCode = http.GET();         // httpCode will be negative on error

   
    if(httpCode > 0) {                // HTTP header has been send and Server response header has been handled
      u8g2.clearBuffer();          // clear the internal memory
      u8g2.drawStr(0,16,"Serwer odpowiedzial:");  // write something to the internal memory
       u8g2.setCursor(1, 32);
      u8g2.print(httpCode);
      u8g2.sendBuffer();
      
         Serial.printf("[HTTP] GET... code: %d\n", httpCode);        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            u8g2.drawStr(0,48, "Czytam z serwera");
            u8g2.sendBuffer();
           String payload = http.getString();
       
         for( linia=0;linia<IleZmiennych;linia++){
          zmienne[linia]=getValue(getValue(payload,'\r\n',linia),'=',1).toInt();
 
             Serial.println(zmienne[linia]);
         }
    
    NowyKierunek=zmienne[0];  //czy jechac w drugim kierunku?
    ZebyM=(float)zmienne[1];     //[n]ilosc zebow na silniku
    ZebyD=(float)zmienne[2];    //[N]ilosc zebow na durzej zembatce
    ZDmm=(float)zmienne[3];       //[Obr/mm] ilosc obrotow duzej zembatki na 1 mm
    Dlugosc=(float)zmienne[4]; //[mm] dlugosc szyny w mm
    Noc=(float)zmienne[5];        //[h]dlugosc nocy w godzinach
    Exp=(float)zmienne[6];       //[s] co ile nastepuje expozycja, expozycja w aparcie powinna byc krotsza aby dala sie zrobic ruch poza exp
    KnaO=(float)zmienne[7];      //  krokow na obrot, katalogowa wartosc podzielona przez 4 
    AccelStart=(float)zmienne[8];  //[ms] pocztkowa wartosc przerwy miedzy krokami
    Speed=(float)zmienne[9];        //[ms] zasadnicza przerwa miedzy krokami
    Accel=zmienne[10];        //[ms] inkrement/dekrement akceleracji
    AccelStep=zmienne[11];    // [n] ilosc krokow w zadanej akceleracji
    krancowkaA=zmienne[12];   // bool co zrobic jak dotrzeby do krancowki 0- stop 1 reverse ??moze 2  cofnij i od poczatku?? ale bez implementacji 2
            
            u8g2.drawStr(0,64, "wczytalem zmienne");
            u8g2.sendBuffer();
            Serial.println("wczytalem nowe zmienne"); 
        }
    } else {
       u8g2.drawStr(0,56,"brak polaczenia");  // write something to the internal memory
       u8g2.setCursor(1, 64);
      u8g2.print(http.errorToString(httpCode).c_str());
      u8g2.sendBuffer();
         Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
     http.end();
     
  KrokNaExp=(int)(ZebyD/ZebyM*KnaO*(Dlugosc-dlugosc_wozka)/(Noc*60*60/Exp));
  KrokNaAcc=(int)((AccelStart-Speed+1)/Accel); //ilosc krokow na przyspeiszenie zwolnienie
  IleExp=(int)(Noc*60*60/Exp);// ile ekspozycji w ciagu nocy
  mmnaruch=(float)KrokNaExp/(float)KnaO*(float)ZebyM/(float)ZebyD;

//wyswietlenie zmiennych
delay(1000);
u8g2.clearBuffer(); 
      u8g2.drawStr(2,8,"NowyKierunek");  // write something to the internal memory
      u8g2.setCursor(80,8);
      u8g2.print(NowyKierunek);
      
      u8g2.drawStr(2,16,"ZebyM");  // write something to the internal memory
      u8g2.setCursor(80,16);
      u8g2.print(ZebyM);


    u8g2.drawStr(2,24,"ZebyD");  // write something to the internal memory
      u8g2.setCursor(80,24);
      u8g2.print(ZebyD);
 

     u8g2.drawStr(2,32,"ZDmm");  // write something to the internal memory
      u8g2.setCursor(80,32);
      u8g2.print(ZDmm);

          u8g2.drawStr(2,40,"Dlugosc");  // write something to the internal memory
      u8g2.setCursor(80,40);
      u8g2.print((Dlugosc-dlugosc_wozka));
    
             u8g2.drawStr(2,48,"Noc");  // write something to the internal memory
      u8g2.setCursor(80,48);
      u8g2.print(Noc);
      
             u8g2.drawStr(2,56,"Exp");  // write something to the internal memory
      u8g2.setCursor(80,56);
      u8g2.print(Exp);

               u8g2.drawStr(2,64,"KnaO");  // write something to the internal memory
      u8g2.setCursor(80,64);
      u8g2.print(KnaO);
        u8g2.sendBuffer();
        delay(5000);
u8g2.clearBuffer(); 
      u8g2.drawStr(2,8,"AccelStart");  // write something to the internal memory
      u8g2.setCursor(80,8);
      u8g2.print(AccelStart);
      
      u8g2.drawStr(2,16,"Speed");  // write something to the internal memory
      u8g2.setCursor(80,16);
      u8g2.print(Speed);


    u8g2.drawStr(2,24,"Accel");  // write something to the internal memory
      u8g2.setCursor(80,24);
      u8g2.print(Accel);
 

     u8g2.drawStr(2,32,"AccelStep");  // write something to the internal memory
      u8g2.setCursor(80,32);
      u8g2.print(AccelStep);

          u8g2.drawStr(2,40,"krancowkaA");  // write something to the internal memory
      u8g2.setCursor(80,40);
      u8g2.print(krancowkaA);
    
             u8g2.drawStr(2,48,"KrokNaExp");  // write something to the internal memory
      u8g2.setCursor(80,48);
      u8g2.print(KrokNaExp);
      
             u8g2.drawStr(2,56,"KrokNaAcc");  // write something to the internal memory
      u8g2.setCursor(80,56);
      u8g2.print(KrokNaAcc);

               u8g2.drawStr(2,64,"IleExp");  // write something to the internal memory
      u8g2.setCursor(80,64);
      u8g2.print(IleExp);
        u8g2.sendBuffer();

             u8g2.sendBuffer();
        delay(5000);
        
        
}


  


unsigned long czas;   //czas rozpoczecia dzialania funkcji loop w microsekundach odswierzany przy kazdej expozycji
int Krok=0;// licznik kroku dla danej expozycji
int NExp=0;// licznik bierzacej expozycji;
unsigned long czekaj_do;
unsigned long zegarek;
bool s=1; //czy ruszamy poraz pierwszy to poto aby zdefiniowaÄ‡ jednoznaczy znacnzik czasu zaraz na poczatku funkcji loop i siÄ™ go trzymaÄ‡
bool po_krokach=1;
bool drzenie=1;
float przejechane=0;





void loop(){//bez delay

zm_kierunek(NowyKierunek);


if(s){
  po_krokach=1;
  s=0;
  Krok=0;
  pauza=(int) AccelStart*1000+1000;
    Serial.print("KrokNaExp:\t");
    Serial.println(KrokNaExp);
    Serial.print("KrokNaAcc:\t");
Serial.println(KrokNaAcc);
  Serial.println(pauza);
  czas=micros();
  czekaj_do=0;
  }

  
if(NExp<IleExp)
  {
/*  Serial.print("Expozycja numer\t\t"); 
  Serial.println(NExp);
    Serial.print("krokwow na exp\t\t"); 
  Serial.println(KrokNaExp);
*/
  
  
   if((start)&&(Krok<KrokNaExp))
     {

/*Serial.print("micros czas micros-czas czekaj do\t\t"); 
  Serial.print(micros());
    Serial.print("\t"); 
  Serial.print(czas);
  Serial.print("\t"); 
  Serial.print(micros()-czas);
    Serial.print("\t"); 
  Serial.println(czekaj_do);
  */
      
      if((micros()-czas)>czekaj_do)//prÄ™dkoĹ›Ä‡ kroku ale tak aby byla mozlisc aaccel deaccel
        {
         
 // Serial.print(Krok);
 // Serial.print("\t");
 // Serial.println(micros());
  
       //  j=Krok%4;
          //Serial.print("j\t\t"); 
  //Serial.println(j);
         for(k=0;k<IlePinow;k++){  
           
           digitalWrite(piny[k], sekwencja[Krok%4][k]);
          }
           if(Krok<KrokNaAcc){
              pauza-=Accel*1000; // 
             }  

              if(KrokNaExp-Krok<KrokNaAcc){
                 pauza+=Accel*1000; // 
                }         
      
    //       Serial.print("pauza\t\t"); 
    //       Serial.println(pauza);
           czekaj_do+=pauza;  //

      //     Serial.print("czekaj do\t\t"); 
        //   Serial.println(czekaj_do);


          Krok++;  
        }

     
      }
      
    if(Krok==KrokNaExp)//to po krokach to do wyĹ›wietlenia monitu tylko raz
    {
      if(po_krokach){
      u8g2.clearBuffer();          // clear the internal memory
      u8g2.drawStr(2,10,"Ruch zakonczony");  // write something to the internal memory
      u8g2.drawStr(2,24,"Wszystkie piny:");
      u8g2.drawStr(30,36,"HIGH");
      u8g2.sendBuffer();
      
      for(k=0;k<IlePinow;k++){  
          
           digitalWrite(piny[k], HIGH);
          }

          
      u8g2.clearBuffer();  
       u8g2.drawStr(2,10,"Koniec Ruchu");
      u8g2.drawStr(2,24,"Kroki zajely:");  // write something to the internal memory
      u8g2.setCursor(2,34);
      u8g2.print(((float)(czekaj_do/10000))/100);
      u8g2.drawStr(2,44,"Czekam do:");
      u8g2.setCursor(2,54);
      u8g2.print(Exp);
       u8g2.sendBuffer();
      
      Serial.print("kroki zajely\t\t"); 
      Serial.println(czekaj_do);
      Serial.print("czekam do\t\t"); 
      Serial.println(Exp);
      po_krokach=0;
      drzenie=1;
      przejechane+=mmnaruch;
      zegarek=czekaj_do+3000000;
      }
      if((micros()-czas)>zegarek)
        {
           u8g2.clearBuffer();
           u8g2.drawStr(2,24,"Czekam do:");
           u8g2.setCursor(78,24);
           u8g2.print(Exp);
           u8g2.drawStr(2,34,"Expozycja:");
           u8g2.setCursor(60,34);
           u8g2.print(NExp);
           u8g2.print(" z ");
           u8g2.print(IleExp);
           u8g2.drawStr(2,44,"Przejechalem:");
           u8g2.setCursor(2,54);
           u8g2.print(przejechane);
           u8g2.print(" z ");
           u8g2.setCursor(80,54);
           u8g2.print((Dlugosc-dlugosc_wozka));
           
           u8g2.setCursor(50,10);
           u8g2.print(((float)((micros()-czas)/10000))/100);
           
           u8g2.sendBuffer();
         zegarek+=100000;
         if(((micros()-czas)+100000)>(long)(Exp*1000000)){
           u8g2.clearBuffer();
           u8g2.drawStr(20,16,"! UWAGA !");
           u8g2.drawStr(20,30,"! RUCH !");
           
         //u8g2.drawStr(0,50,"zmien kierunek reczni"); dlugosc mieszczacego sie ciegu znakow
           u8g2.drawStr(0,50,"Nie dotykaj suwnicy");
           u8g2.drawStr(0,60,"Nie dotykaj aparatu");
          u8g2.sendBuffer();
          }
        }
    }
   /*   Serial.print("czekam do\t\t"); 
      Serial.print(micros()); 
      Serial.print("\t"); 
      Serial.print(czas); 
      Serial.print("\t"); 
      Serial.println((long)(Exp*1000000));
      */
   if((micros()-czas)>(long)(Exp*1000000)){ //srprawdzenie czy minal czas expozycji
      Serial.print("od nowa po \t\t"); 
      Serial.println(micros()-czas);
        
      s=1;
      NExp++;  
      return;
     }

   }
else{
    if(wyswietlkoniec){
     u8g2.clearBuffer();
     u8g2.drawStr(2,10,"Koniec krokow!");
    }
     
       if(krancowkaA){
         u8g2.drawStr(2,24,"Zawracam");
         u8g2.sendBuffer();
         start=1;
         s=1;
         NExp=0;
         NowyKierunek=!AktKierunek;
     
        }   
        else{
          if(wyswietlkoniec){
          u8g2.drawStr(20,24,"! STOJE !");
          //u8g2.drawStr(0,50,"zmien kierunek reczni"); dlugosc mieszczacego sie ciegu znakow
            u8g2.drawStr(0,32,"zmien kierunek");
            u8g2.drawStr(20,40,"recznie ");
            u8g2.drawStr(0,50,"wcisnij reset");
            u8g2.drawStr(0,60,"aby uruchomic");
           u8g2.sendBuffer();
           wyswietlkoniec=0;
           }
           
          } 
  }
}


void krancowka(){ //to drzenie ma zabezpieczac przed drzeniem, interrupt zadziala kolejny raz po nastepnym calym obrocie
  if(drzenie){ 

     u8g2.clearBuffer();
           u8g2.drawStr(20,10,"! KRANCOWKA !");
          u8g2.sendBuffer();
          
  Serial.println("dotarlem do krancowki!");
  if(krancowkaA)
  {
     u8g2.drawStr(20,30,"! ZAWRACAM !");
      u8g2.sendBuffer();
        
    Serial.println("Zawracam!");
    start=1;
    s=1;
    NExp=0;
    
    NowyKierunek=!AktKierunek;
    }
    else
    {
      u8g2.drawStr(20,24,"! STOJE !");
            u8g2.drawStr(0,32,"zmien kierunek");
            u8g2.drawStr(0,40,"recznie ");
            u8g2.drawStr(0,50,"wcisnij reset");
            u8g2.drawStr(0,60,"aby uruchomic");
            u8g2.sendBuffer();
      Serial.println("Stoje zresetuj aby uruchomic");
      start=0;
      }
      
      }
      drzenie=0;
  }
