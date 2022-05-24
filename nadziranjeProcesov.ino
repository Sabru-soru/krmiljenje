#include <SoftwareSerial.h>
#include <SD.h>
File dataFile;
#define power 4
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"
unsigned long tz=0;
unsigned long tk=0;
unsigned long sek=300;

SoftwareSerial mySerial(5, 6);        //5 Rx rumena, 6 Tx črna
#define SD_CS 9                       //SD kartica
#define SPI_CS 10                     //kamera
ArduCAM myCAM(OV2640,SPI_CS);
boolean isShowFlag = true;
char inc=0;

void setup()
{ 
  mySerial.begin(57600);              //Baud rate GSM modula
  
  uint8_t vid,pid;                    //za kamero
  uint8_t temp;
  #if defined(__SAM3X8E__)
  Wire1.begin();
  #else
  Wire.begin();
  #endif 
  
  Serial.begin(115200);               // Setting the baud rate of Serial Monitor (Arduino)
  pinMode(3,INPUT);
  pinMode(power,OUTPUT);
  pinMode(2,OUTPUT);                  //sluzi za priziganje ledice, debuggig
  digitalWrite(2,LOW);
  
  Serial.println(F("ArduCAM Start!")); 
  // set the SPI_CS as an output:
  pinMode(SPI_CS, OUTPUT);

  // initialize SPI:
  SPI.begin(); 
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if(temp != 0x55)
  {
    Serial.println(F("SPI interface Error!"));
    while(1);                         //Ce je problem z SPI komunikacijo se program tu ustavi, ideja, da bi se poslalo SMS da ne dela
  }
  //mySerial.print("AT+CMGF=1\r");  // set SMS mode to text
  //delay(100);
  //mySerial.print("AT+CNMI=2,2,0,0,0\r");
  //delay(1000);
  
  //Check if the camera module type is OV2640
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if((vid != 0x26) || (pid != 0x42))
    Serial.println(F("Can't find OV2640 module!"));
  else
    Serial.println(F("OV2640 detected"));
    
  //Nastavi JPEG format in inicializira kamero     
  myCAM.set_format(JPEG);

  myCAM.InitCAM();
  
  //Initialize SD Card
  if (!SD.begin(SD_CS)) 
  {
    //while (1);    
    Serial.println(F("SD Card Error"));
  }
  else
    Serial.println(F("SD Card detected!"));

Serial.println(F("s - slikaj, m - mms"));
}

void loop()
{
  if(digitalRead(3)==LOW){
  digitalWrite(power,HIGH);
  delay(2000);
  digitalWrite(power,LOW);
  }
  
  if (Serial.available()>0)
   switch(Serial.read())
  {
    case 's':
      Slikaj();
      break;
    case 'm':
      sendMMS();
      break;
  }

 /* if (mySerial.available()>0)
   Serial.write(mySerial.read());*/

  tk=millis();
  if((tk-tz)>(sek*1000)){
  Slikaj();
  tz=tk;
  }

  if(mySerial.available() >0){
    inc=mySerial.read();          //Get the character from the cellular serial port.
    if(inc=='#'){                  //Ce prejme # zazene slikanje in posiljanje
      Slikaj();
    }
    else if(inc=='*'){
      sek=sek+60;
    }
    else if(inc=='_'){
      sek=sek-60;
      if(sek<180)
      sek=180;
    }
  }
 
}

//+++++++++++++++++++++++++Podfunkcije+++++++++++++++++++++

void sendMMS()
{
  int i;
  unsigned char data=0;
  digitalWrite(2,HIGH);

 //Nastavitve omrežja ---------------------------
  Serial.println(F("MMS"));
  mySerial.println(F("AT+CMMSINIT\r"));
  delay(1000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+CMMSCURL=\"mmc\"\r"));                 //http://mms.celcom.net.my
  delay(1000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+CMMSCID=1\r"));
  delay(1000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+CMMSPROTO=\"80.95.224.46\",9201\r"));  //10.128.1.242\",8080
  delay(1000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+CMMSSENDCFG=6,3,0,0,2,4\r"));
  delay(1000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r"));   //MMS
  delay(1000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+SAPBR=3,1,\"APN\",\"mms.simobil.si\"\r")); //mms.celcom.net.my
  delay(1000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+SAPBR=1,1\r"));
  delay(1000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+SAPBR=2,1\r"));
  delay(1000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+CMMSEDIT=1\r"));
  delay(1000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  dataFile=SD.open(F("1.jpg"));                                     //pošiljal bo sliko z imenom 1.jpg
  //mySerial.print("AT+CMMSDOWN=\"PIC\",5809,2000000,\"3.jpg\"\r");
  mySerial.print(F("AT+CMMSDOWN=\"PIC\","));
  mySerial.print(dataFile.size());                                  //velikost slike v bytih
  mySerial.println(F(",2000000,\"1.jpg\"\r"));
  delay(1000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }

  if(dataFile)
  {
    while(dataFile.available())
    {
      data=dataFile.read();
      if(data<0x10) Serial.print("0");
      Serial.print(data,HEX);
      i++;
      if((i%40)==0) Serial.println();
      mySerial.write(data);
    }
    dataFile.close();
  }  
  else
  {
    Serial.println(F("error opening 1.jpg"));
  }
  delay(1000);
  mySerial.println(F("AT+CMMSRECP=\"nadziranjeslike@gmail.com\"\r"));
  /*delay(1000);
  mySerial.println("AT+CMMSBCC=\"luka.selak@fs.uni-lj.si\"\r");     //Skrivni prejemnik
  delay(1000);*/
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+CMMSVIEW\r"));
  delay(2000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+CMMSSEND\r"));
  delay(2000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+CMMSEDIT=0\r"));
  delay(2000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  mySerial.println(F("AT+CMMSTERM\r"));
  delay(2000);
  if(mySerial.available())
  {
    while(mySerial.available()) Serial.write(mySerial.read());
  }
  digitalWrite(2,LOW);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
void Slikaj()
{
  char str[8];
  File outFile;
  byte buf[256];
  static int i = 0;
  static int k = 1;
  static int n = 0;
  uint8_t temp,temp_last;
  uint8_t start_capture = 0;
  int total_time = 0;
  digitalWrite(2,HIGH);
  
  isShowFlag = false;
  myCAM.InitCAM();

  //myCAM.OV2640_set_JPEG_size(OV2640_640x480);
  myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);
  //Wait until buttom released - ce bi uporabljal gumb na modulu
  while(myCAM.get_bit(ARDUCHIP_TRIG, SHUTTER_MASK));
  delay(1000);
  start_capture = 1;

  if(start_capture)
  {
    //Flush the FIFO 
    myCAM.flush_fifo();  
    //Clear the capture done flag 
    myCAM.clear_fifo_flag();     
    //Start capture
    myCAM.start_capture();
    Serial.println(F("Start Capture"));     
  }
  
  while(!(myCAM.read_reg(ARDUCHIP_TRIG)& CAP_DONE_MASK)){  //pocaka da se slika
    delay(10);
  }
  
    Serial.println(F("Capture Done!"));
    
    //Construct a file name - sharanjevanje slike
    //k = k + 1;
    itoa(k, str, 10); 
    strcat(str,".jpg");
    //Open the new file  
    outFile = SD.open(str,O_WRITE | O_CREAT | O_TRUNC);    
    if (! outFile) 
    { 
      Serial.println(F("open file failed"));
      return;
    }
    total_time = millis();
    i = 0;
    temp = myCAM.read_fifo();
    //Write first image data to buffer
    buf[i++] = temp;

    //Read JPEG data from FIFO
    while( (temp != 0xD9) | (temp_last != 0xFF) )
    {
      temp_last = temp;
      temp = myCAM.read_fifo();
      //Write image data to buffer if not full
      if(i < 256)
        buf[i++] = temp;
      else
      {
        //Write 256 bytes image data to file
        outFile.write(buf,256);
        i = 0;
        buf[i++] = temp;
      }
    }
    //Write the remain bytes in the buffer
    if(i > 0)
      outFile.write(buf,i);

    //Close the file 
    outFile.close(); 
    total_time = millis() - total_time;
    Serial.print(F("Total time used:"));
    Serial.print(total_time, DEC);
    Serial.println(F(" millisecond"));    
    //Clear the capture done flag 
    myCAM.clear_fifo_flag();
    //Clear the start capture flag
    start_capture = 0;
    
    //myCAM.set_format(BMP);
    //myCAM.InitCAM();
    isShowFlag = true;

    //Po slikanju izvede funkcijo pošiljanja MMSa
    digitalWrite(2,LOW);
    delay(2000);
    sendMMS();
    
}

