// Raimundo Alfonso Sanchez
// Ray Ingeniería Electrónica, S.L. - 2016
// Test ArduPLC MEGA

// Para usar ethernet, selecciona contolador W5500 dentro de la librería Ethernet.

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <Wire.h>
// https://github.com/orbitalair/Rtc_Pcf8563
#include "src/Rtc_Pcf8563.h"

// Mapa de pines ArduPLC MEGA - Arduino
#define RELE1   30
#define RELE2   31
#define RELE3   32
#define RELE4   33
#define RELE5   34
#define RELE6   35
#define DIN1    22
#define DIN2    23
#define DIN3    24
#define DIN4    25
#define DIN5    26
#define DIN6    27
#define DIN7    28
#define DIN8    29
#define AIN1    A6
#define AIN2    A7
#define AIN3    A8
#define AIN4    A9
#define AIN5    A10
#define AIN6    A11
#define POT1    A14
#define POT2    A15
#define AOUT1   44
#define AOUT2   45
#define LED1    36
#define LED2    37
#define WIRE    38
#define BUZZER  49
#define CS_uSD      46
#define INSERT_uSD  47

// Ethernet...
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192,168,0,190);
EthernetServer server(80);

// uSD...
Sd2Card card;
SdVolume volume;
SdFile root;

// rtc...
#define ADDR_RTC   0x51  // Direccion I2C del RTC
Rtc_Pcf8563 rtc;  


void menu_inicio(void){
  Serial.println(F("---    Test ArduPLC MEGA v1.00    ---"));
  Serial.println(F("   Ray Ingenieria Electronica,S.L."));  
  Serial.println();
  Serial.println(F("Press '?' for help menu"));    
}

void menu(void){
  Serial.println(F("       MAIN MENU"));
  Serial.println(F("-----------------------------------------------"));
  Serial.println(F("Rn       Activate relay n (n from 1 to 6)"));
  Serial.println(F("Fn       Deactivate relay n (n from 1 to 6)"));
  Serial.println(F("D        Read digital inputs (1-8)"));
  Serial.println(F("1        Read 1-WIRE"));  
  Serial.println(F("A        Read analog inputs"));
  Serial.println(F("Onxxxxx  Set analog output n (n is 1 or 2, xxxxx = mV)"));
  Serial.println(F("RT       Read date and time (only with RTC)"));  
  Serial.println(F("WDddmmaa Write date to RTC"));
  Serial.println(F("WThhmmss Write time to RTC"));
  Serial.println(F("B        Test LEDs L1, L2 and buzzer"));  
  Serial.println(F("S        Test uSD"));    
  Serial.println(F("E        Read digital inputs from expansion module"));    
}

String inString;
int aout1 = 0;
int aout2 = 0;
int param = 0;


void setup(){ 
  pinMode(RELE1, OUTPUT);
  pinMode(RELE2, OUTPUT);
  pinMode(RELE3, OUTPUT);
  pinMode(RELE4, OUTPUT);
  pinMode(RELE5, OUTPUT);
  pinMode(RELE6, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(DIN1, INPUT_PULLUP);
  pinMode(DIN2, INPUT_PULLUP);
  pinMode(DIN3, INPUT_PULLUP);
  pinMode(DIN4, INPUT_PULLUP);
  pinMode(DIN5, INPUT_PULLUP);
  pinMode(DIN6, INPUT_PULLUP);
  pinMode(DIN7, INPUT_PULLUP);    
  pinMode(DIN8, INPUT_PULLUP);  
  pinMode(WIRE, INPUT);

  pinMode(CS_uSD, OUTPUT);
  pinMode(INSERT_uSD, INPUT);  
  digitalWrite(CS_uSD, HIGH); // Deshabilita uSD para no interferir con el controlador W5500

  
  // Referencia externa
  analogReference(EXTERNAL);
  
  // Configura puertos serie...
  Serial.begin(9600); 
  Serial2.begin(9600); 




  // Inicializa ethernet y servidor web...
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }

  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());


  // Espera un poco mostrando el logo...
  delay(3000);

  // Mensaje inicial...
  menu_inicio();

  // Menu de opciones...
  menu();

} 

void loop(){ 
  int c;
  static long timer_oled = 0;


  // Servidor web...
  test_server();
  
  // Comprueba lo que recibes por puerto RS485....
  if (Serial2.available() > 0) {
    int c = Serial2.read();
    inString += (char)c; 
    if (c == 0x0D) {
      // Quita el retorno de carro...
      inString = inString.substring(0, inString.length() - 1);
      // comprueba el comando...
      // Menu...
      if(inString == "?"){
        Serial2.println(F("             MENU PRINCIPAL POR RS485 "));
        Serial2.println(F("-----------------------------------------------"));
        Serial2.println(F("Rn       Activa rele n (n de 1 a 6)"));
        Serial2.println(F("Fn       Desactiva rele n (n de 1 a 6)"));
      }
      // Rele ON...
      if(inString.startsWith("R")){
        releOn(inString.charAt(1));
        Serial2.println("OK");
      }
      // Rele OFF...
      if(inString.startsWith("F")){
        releOff(inString.charAt(1));
        Serial2.println("OK");        
      }         
      inString = ""; 
    }  
  }
  // Comprueba lo que recibes por puerto USB....
  if (Serial.available() > 0) {
    int c = Serial.read();
    inString += (char)c; 
    if (c == 0x0D) {
      // Quita el retorno de carro...
      //inString = inString.substring(0, inString.length() - 1);
      inString.trim();
      // comprueba el comando...
      // Menu...
      if(inString == "?"){
        menu();
      }
      // Rele ON...
      if(inString.startsWith("R")){
        releOn(inString.charAt(1));
        Serial.println("OK");
      }
      // Rele OFF...
      if(inString.startsWith("F")){
        releOff(inString.charAt(1));
        Serial.println("OK");        
      }   
      // Lee entradas analogicas...
      if(inString.startsWith("A")){
        analogR();
        Serial.println("OK");        
      }  
      // Lee entradas digitales...
      if(inString.startsWith("D")){
        digitalR();
        Serial.println("OK");        
      }
      // Lee entradas 1-wire...
      if(inString.startsWith("1")){
        digitalWire();
        Serial.println("OK");        
      }      
      // Salidas analógicas...
      if(inString.startsWith("O")){
        analogOut(inString);
        Serial.println("OK");
      }      
      // Test leds y buzzer...
      if(inString.startsWith("B")){
        leds();
        Serial.println("OK");        
      }      
      // Test uSD...
      if(inString.startsWith("S")){
        test_sd();
        Serial.println("OK");        
      }  
      // Lee fecha y hora...
      if(inString.startsWith("RT")){
        Serial.println(leeFecha());
        Serial.println(leeHora());        
        Serial.println("OK");
      }
       // Escribe hora...
      if(inString.startsWith("WT")){
        escribeHora(inString.substring(2));
        Serial.println("OK");        
      }
       // Escribe fecha...
      if(inString.startsWith("WD")){
        escribeFecha(inString.substring(2));
        Serial.println("OK");        
      }     
 
    
      inString = ""; 
      Serial.flush();
    }    
  }

  // Actualiza salidas analógicas...
  analogWrite(AOUT1, aout1); 
  analogWrite(AOUT2, aout2); 

} 


void releOn(char rele){
  byte r;
  r = byte(rele) - 0x30;
  if(r == 1) digitalWrite(RELE1, HIGH);
  if(r == 2) digitalWrite(RELE2, HIGH);  
  if(r == 3) digitalWrite(RELE3, HIGH);  
  if(r == 4) digitalWrite(RELE4, HIGH);  
  if(r == 5) digitalWrite(RELE5, HIGH);  
  if(r == 6) digitalWrite(RELE6, HIGH);      
}

void releOff(char rele){
  byte r;
  r = byte(rele) - 0x30;
  if(r == 1) digitalWrite(RELE1, LOW);
  if(r == 2) digitalWrite(RELE2, LOW);  
  if(r == 3) digitalWrite(RELE3, LOW);  
  if(r == 4) digitalWrite(RELE4, LOW);  
  if(r == 5) digitalWrite(RELE5, LOW);  
  if(r == 6) digitalWrite(RELE6, LOW);  
}

void analogR(){
  int an1, an2, an3, an4, an5, an6, an7, an8, an9;
  float v1, v2, v3, v4, v5, v6, pot1, pot2;
  an1 = analogRead(AIN1);  
  an2 = analogRead(AIN2);
  an3 = analogRead(AIN3);    
  an4 = analogRead(AIN4);  
  an5 = analogRead(AIN5);  
  an6 = analogRead(AIN6);  
  an7 = analogRead(POT1);  
  an8 = analogRead(POT2);    
  
  
  v1 = ((float)an1 * 10.0) / 1023.0;
  v2 = ((float)an2 * 10.0) / 1023.0;
  v3 = ((float)an3 * 10.0) / 1023.0;  
  v4 = ((float)an4 * 10.0) / 1023.0;  
  v5 = ((float)an5 * 10.0) / 1023.0;  
  v6 = ((float)an6 * 10.0) / 1023.0;  
  pot1 = ((float)an7 * 100.0) / 1023.0;  
  pot2 = ((float)an8 * 100.0) / 1023.0;        
 
  Serial.print("AN1: ");
  Serial.print(v1,3);
  Serial.println(" v");
  Serial.print("AN2: ");
  Serial.print(v2,3);
  Serial.println(" v");
  Serial.print("AN3: ");
  Serial.print(v3,3);
  Serial.println(" v");
  Serial.print("AN4: ");
  Serial.print(v4,3);
  Serial.println(" v");
  Serial.print("AN5: ");
  Serial.print(v5,3);
  Serial.println(" v");
  Serial.print("AN6: ");
  Serial.print(v6,3);
  Serial.println(" v");
  Serial.print("POT1: ");
  Serial.print(pot1,0);
  Serial.println(" %");
  Serial.print("POT2: ");
  Serial.print(pot2,0);
  Serial.println(" %");     
}

void digitalR(){
  Serial.print("DIN1 : ");
  Serial.println(!digitalRead(DIN1));

  Serial.print("DIN2 : ");
  Serial.println(!digitalRead(DIN2));

  Serial.print("DIN3 : ");
  Serial.println(!digitalRead(DIN3));
  
  Serial.print("DIN4 : ");
  Serial.println(!digitalRead(DIN4));

  Serial.print("DIN5 : ");
  Serial.println(!digitalRead(DIN5));

  Serial.print("DIN6 : ");
  Serial.println(!digitalRead(DIN6));

  Serial.print("DIN7 : ");
  Serial.println(!digitalRead(DIN7));

  Serial.print("DIN8 : ");
  Serial.println(!digitalRead(DIN8));
}

void digitalWire(){
  Serial.print("1-WIRE PIN : ");
  Serial.println(digitalRead(WIRE));
}

void analogOut(String s){
  byte r;
  char out;
  int mv;
  
  String s1;
  
  out = s.charAt(1);
  s1 = s.substring(2);
  mv = s1.toInt();
  r = byte(out) - 0x30;
  if(r == 1){
    aout1 = (((long)mv * 255L) / 10000L); 
  }
  if(r == 2){
    aout2 = (((long)mv * 255L) / 10000L); 
  } 
}

void leds(void){
  digitalWrite(BUZZER, HIGH);
  delay(500);
  digitalWrite(BUZZER, LOW);
  digitalWrite(LED1, HIGH);
  delay(500);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, HIGH);
  delay(500);
  digitalWrite(LED2, LOW);
}

void test_server(){
  int an1, an2, an3, an4, an5, an6, an7, an8, an9;
  float v1, v2, v3, v4, v5, v6, pot1, pot2;
  
 // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin

          an1 = analogRead(AIN1);  
          an2 = analogRead(AIN2);
          an3 = analogRead(AIN3);    
          an4 = analogRead(AIN4);  
          an5 = analogRead(AIN5);  
          an6 = analogRead(AIN6);  
          an7 = analogRead(POT1);  
          an8 = analogRead(POT2);    
          v1 = ((float)an1 * 10.0) / 1023.0;
          v2 = ((float)an2 * 10.0) / 1023.0;
          v3 = ((float)an3 * 10.0) / 1023.0;  
          v4 = ((float)an4 * 10.0) / 1023.0;  
          v5 = ((float)an5 * 10.0) / 1023.0;  
          v6 = ((float)an6 * 10.0) / 1023.0;  
          pot1 = ((float)an7 * 100.0) / 1023.0;  
          pot2 = ((float)an8 * 100.0) / 1023.0;        
          client.print("AN1: ");
          client.print(v1,3);
          client.println(" v");
          client.println("<br />");               
          client.print("AN2: ");
          client.print(v2,3);
          client.println(" v");
          client.println("<br />");               
          client.print("AN3: ");
          client.print(v3,3);
          client.println(" v");
          client.println("<br />");               
          client.print("AN4: ");
          client.print(v4,3);
          client.println(" v");
          client.println("<br />");               
          client.print("AN5: ");
          client.print(v5,3);
          client.println(" v");
          client.println("<br />");               
          client.print("AN6: ");
          client.print(v6,3);
          client.println(" v");
          client.println("<br />");               
          client.print("POT1: ");
          client.print(pot1,0);
          client.println(" %");
          client.println("<br />");               
          client.print("POT2: ");
          client.print(pot2,0);
          client.println(" %");  
          client.println("<br />");               
          
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

void test_sd(void){
   // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if(digitalRead(INSERT_uSD) == 1){
    Serial.println("uSD no preset.");
    return;
  }
  if (!card.init(SPI_HALF_SPEED, CS_uSD)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.print("\nCard type: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
  Serial.println(volumesize);
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);


  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}


String leeFecha(void){
  return(rtc.formatDate(RTCC_DATE_WORLD));
}

String leeHora(void){
  return(rtc.formatTime());
}

void escribeFecha(String mDate){
  //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  int dia = mDate.substring(0,2).toInt();
  int mes = mDate.substring(2,4).toInt();
  int ano = mDate.substring(4,6).toInt();
  rtc.setDate(dia, 1, mes, 0, ano);
}
  
void escribeHora(String mTime){
  rtc.setTime(mTime.substring(0,2).toInt(), mTime.substring(2,4).toInt(), mTime.substring(4,6).toInt());
}
