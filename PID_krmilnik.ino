/* Program za krmiljenje DC motorja. DKS - 2016 , tim9 */

#define F_CPU 16000000    //dolocimo frekvenco ure za fast PWM

int PWM_pin = 6;          //pin ki je povezan na tranzistor
int enk1 = 2;             //prvi enkoder
int enk2 = 3;             //drugi enkoder
int trig=11;              //pin triggerja, s katerim zacnemo zajemanje na osciloskopu
int pwm =0;               //začetna vrednost PWM (korekcija)
int setpnt=0;             //to je naša zeljena vrednost, ki jo bomo vnasali prek serijske komunikacije
int napaka=0;             //zacetna napaka  
int vsota=0;              //vsota napak, ki bo uporabljena za integrirni del
int prejnapaka=0;         //prejsna napaka za diferencirni del
int korekcija=0;          //z vrednostjo korekcije krmilimo PWM
int a=0;                  //stevec, da se vrednosti na serial monitorju izpisujejo pocasneje
int prejsetpnt=0;         //prejsna nastavljena vrednost. Pomembno za trigger
int prejkorekcija=0;      
double gred=0;

volatile unsigned long casK=0;
volatile unsigned long casZ=0;
volatile unsigned long perioda;
volatile unsigned long frekvenca=0;

double Kp=0.102;          //vrednost proporcionalnega ojacenja
double Ki=0.02;           //vrednost integrirnega ojacenja
double Kd=0;              //vrednost diferencirnega ojacenja

void setup() {

  //attachInterrupt(digitalPinToInterrupt(enk1),prekinitev,CHANGE); //Prvi enkoder ni delal pravilno, zato ga nismo vkljucili v program
  attachInterrupt(digitalPinToInterrupt(enk2),prekinitev,RISING);   //Preracun frekvence se zgodi na rising edge pulza enkoderja

  //inicializacija serijske komunikacije
  Serial.begin(9600); 
  Serial.println("Program za krmiljenje DC motorja."); 
  Serial.println("Vnesi frekvenco: ");


  TCCR0A|=(1<<WGM01)|(1<<WGM00);      //nastavimo registre za Fast PWM
  TCCR0B|=(1<<CS01);                  //nastavimo prescaler na 8 s katerim delimo frekvenco ure
  OCR0A=0;                            //nastavimo zacetno vrednost PWM - output compare register A 0/255
  DDRD|=(1<<PD6);                     //nastavimo izhod na pin6 - 0c0A pin - output-PD6
  TCCR0A|=(1<<COM0A1);                //Clear 0C0A on compare match

  delay(100);
  pinMode(trig,OUTPUT);
  digitalWrite(trig,LOW);
}

void loop() {
  
    prejsetpnt=setpnt;                //to potrebujemo za prozenje triggerja
    napaka = (setpnt - frekvenca);    //tu se izracuna razlika med zeljeno in dejansko vrtilno frekvenco
    vsota+=napaka;                    
    korekcija=(napaka*Kp)+vsota*Ki+(napaka-prejnapaka)*Kd;                //glavni izracun korekcije
    //korekcija= prejkorekcija+27/256 *(napaka - 252/256*prejnapaka);     //poskus implementacije krmilnika iz Sisotool
    prejnapaka=napaka
    prejkorekcija=korekcija;
    
    if((korekcija)>=255){             //ce je korekcija vec kot 255 omejimo PWM na najvisjo vrednost
      korekcija=255; 
    }
    else if((korekcija)<0){           //ce je korekcija negativna nastavimo PWM na nic
      korekcija=0; 
    }
    
  OCR0A=korekcija;                    //Registru za PWM pripisemo vrednost korekcije
  gred=frekvenca/9.68;                //Na izhodni gredi je se reduktor, ki zmanjsa hitrost za 9,68
    
   if(a==1000){                       //tukaj bo potekal izpis podatkov, s katerimi si pomagamo
     Serial.print("Napaka: ");       
     Serial.print(napaka);
     Serial.print("\t Korak: ");
     Serial.print(korekcija);
     Serial.print("\t: PWM: ");
     Serial.print(OCR0A,DEC);
     Serial.print("\t Period: ");
     Serial.print(perioda,DEC);
     Serial.print("\t Frekv: ");
     Serial.print(frekvenca,DEC);
     Serial.print("\t FrekvGr: ");
     Serial.println(gred,2);
     a=0;                             //spremenljivko postavimo na 0, da se zopet zacne stetje
     digitalWrite(trig,LOW);          //trigger postavimo na nizko stanje
   }
   a++;

// Beremo serijski port - nastavitev hitrosti
  while(Serial.available() > 0)
  {
    setpnt = Serial.parseInt();       //preberemo nastavljeno vrednost
    Serial.println(setpnt);
    if(prejsetpnt!=setpnt){           //tukaj zazna, ce smo zamenjali zeljeno vrednost, takrat prozi triger in zacne zajemati
      digitalWrite(trig,HIGH);        
    }
  }
}

void prekinitev()                     //tu se bo izracunala frekvenca vrtenja gredi
{
  casK=micros();                      //zabelezi se cas od zagona programa v mikrosekundah
  perioda=casK-casZ;                  //izracuna se perioda med posamezimi pulzi enkoderja
  casZ=casK;                          //shranimo čas za naslednji cikel
  frekvenca=(1000000/perioda)/12;     //frekvenca gredi enkoderja v Hz, delimo z 12, ker da enkoder 12 pulzov na obrat
}
