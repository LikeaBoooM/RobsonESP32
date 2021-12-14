#include "time.h"
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>
#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"

#define TX_PIN           27 
#define RX_PIN           26

#define RST_PIN          22          
#define SS_PIN           21        
 
MFRC522 mfrc522(SS_PIN, RST_PIN);   
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

SoftwareSerial mySerial(RX_PIN, TX_PIN); 
Adafruit_Thermal printer(&mySerial); 

char tabb[12] = {'0','0','0','0','0','0','0','0','0','0','0','0'};


const char* ssid     = "Internet_Domowy";
const char* password = "Mjg3OUNG";


const char* ntpServer = "europe.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

//////////////////////////////////// WEB API //////////////////////////////////////////////////////////////
String apikey = ""; // KLUCZ API DO WSZYSTKICH POSTÓW JAKO 1 ARGUMENT
const char* serverNamePrzewozy = "https://przewozy.vxm.pl/api/carriages.php"; // POST DO PRZEWOZÓW 
const char* serverKartyName = "https://przewozy.vxm.pl/api/fuel.php"; // GET PO ID KARTY, ABY DOSTAC ILOSC PALIWA NA DANEJ KARCIE
const char* serverKartyPOST = "https://przewozy.vxm.pl/api/card.php"; // POST ABY WYZEROWAC ILOSC PALIWA NA DANEJ KARCIE
const char* serverTankowanieName = "https://przewozy.vxm.pl/api/refueling.php"; // POST ABY DODAC TANKOWANIE
const char* serverOdbiory = "https://przewozy.vxm.pl/api/pick.php"; // POST ODBIORY
////////////////////////////////////////////////////////////////////////////////////////////////////////////


unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

String wage="";

TaskHandle_t Task1; 
TaskHandle_t Task2;
  
void setup() {
  delay(0);
  Serial.begin(9600);        // Initialize serial communications with the PC
  Serial2.begin(9600);
  mySerial.begin(19200); 
  printer.begin();
  SPI.begin();               // Init SPI bus
  mfrc522.PCD_Init();        // Init MFRC522 card
  Serial.println(F("Write personal data on a MIFARE PICC "));
 // WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
delay(0);
 xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */
    delay(500); 
}



void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    wage = get_waga();
    Serial.println(wage.toInt());
    delay(1000);
  }
}
 
void loop() {

  czytaj();
}

void czytaj()
{
  //////////////////////////////////////////////////////////////////////////////////////
    byte buffer[18];
    byte size = sizeof(buffer);
    String id = "";
    String przewoznik = "";
    String rejestracja = "";
    String kontrahent = "";
    String tara = "";
    String waga = "";
    String produkt = "";
    String waga_netto = "";
////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
 
 if ( ! mfrc522.PICC_IsNewCardPresent())
        return;
 if ( ! mfrc522.PICC_ReadCardSerial())
        return;

  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : ""));
     id.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
    
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

    // Sprawdzenie kompatybilności
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    //    Serial.println(F("This sample only works with MIFARE Classic cards."));
        return;
    }
    
   status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 7, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(4, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
   
  for(int i=0; i < (size -3) ; i++)
  {
  przewoznik.concat((char)buffer[i]);
  }
  
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 7, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(5, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
 
 for(int j=0; j < (size -3) ; j++)
  {
  rejestracja.concat((char)buffer[j]);
  }
 
///////////////////////////////////////////////////////////////////////////////////////////////////////

    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 7, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(6, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    //Serial.print(F("Data in block ")); Serial.print(6); Serial.println(F(":"));
    //dump_byte_array(buffer, 16); Serial.println();
    //Serial.println();
    
  
  for(int k=0; k < (size -3) ; k++)
  {
  kontrahent.concat((char)buffer[k]);
  }

////////////////////////////////////////////////////////////////////////////////////////////////////////

    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 11, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(8, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    //Serial.print(F("Data in block ")); Serial.print(8); Serial.println(F(":"));
    //dump_byte_array(buffer, 16); Serial.println();
    //Serial.println();
  
  for(int l=0; l < (size -3) ; l++)
  {
  tara.concat((char)buffer[l]);
  }
  tara.toInt();
  
 
////////////////////////////////////////////////////////////////////////////////////////////////////////

    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 11, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }


    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(9, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    
  for(int m=0; m < (size -3) ; m++)
  {
  produkt.concat((char)buffer[m]);
  }
  

  delay(50);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  waga_netto = (int)wage.toInt() - (int)tara.toInt();
  /////////////////////////////////////////////////////////////////////////////////////////
  Serial.print("Id karty: ");
  Serial.println(id);
  Serial.print("Przewoznik: ");
  Serial.println(przewoznik);
  Serial.print("Rejestracja: ");
  Serial.println(rejestracja);
  Serial.print("Kontrahent: ");
  Serial.println(kontrahent);
  Serial.print("Produkt: ");
  Serial.println(produkt);
  Serial.print("Wazenie :");
  Serial.println(wage + " kg");
  Serial.print("Tara: ");
  Serial.println(tara + " kg");
  Serial.print("Netto :");
  Serial.println(waga_netto + " kg");
  
  /////////////////////////////////////////////////////////////////////////////////////////////

 //POSTDodajPrzewozy(przewoznik, rejestracja, kontrahent, produkt, waga_netto);

  drukuj(rejestracja,kontrahent, produkt,wage,tara,waga_netto);
  
 //POSTRequestOdbiory(rejestracja,2);

  //GETIloscPaliwaNaKarciePoID(id);
  //POSTZerujIloscPaliwaNaKarciePoID(id);
 
  //POSTDodawanieTankowania(id, rejestracja, "testowy");
  //POSTRequestOdbiory(rejestracja,2);
}

String GETIloscPaliwaNaKarciePoID(String cardId) {
  HTTPClient http;

  String httprequest = "http://przewozy.vxm.pl/api/fuel.php?card_id=" + cardId;
  http.begin(httprequest.c_str());
  Serial.println(httprequest);
  
  String payload = "{}"; 
  int httpCode = http.GET(); 
  if (httpCode > 0) { //Check for the returning code
        String payload = http.getString();
        //Serial.println(httpCode);
        Serial.println(payload);
  }else {
      Serial.println(httpCode);
      Serial.println("Error on HTTP request");
  }
  http.end();
  Serial.print(payload);
  return payload;
}


void POSTZerujIloscPaliwaNaKarciePoID(String cardID) {
     HTTPClient http;
     
      const char* serverKartyPOST = "https://przewozy.vxm.pl/api/fuel.php";
      http.begin(serverKartyPOST);      

      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
   
      String httprequest ="key=" + apikey + "&card_id=" + cardID;
      Serial.println(httprequest);
      
      int httpResponseCode = http.POST(httprequest);
      String httpResponseText = http.getString();
      
      Serial.println("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.println("HTTP Response Text: ");
      Serial.println(httpResponseText);

      http.end();
     
 }

void POSTRequestKartydodawanie(String cardID, String rejestracja) { //DZIAŁA
      HTTPClient http;
   
      http.begin(serverKartyPOST);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
      String httprequest ="key=" + apikey + "&card_id=" + cardID + "&registration=" + rejestracja;
      
      Serial.println(httprequest);

      int httpResponseCode = http.POST(httprequest);
      String httpResponseText = http.getString();
      
      Serial.println("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.println("HTTP Response Text: ");
      Serial.println(httpResponseText);
      

      http.end();
 }

void POSTDodajPrzewozy(String przewoznik, String rejestracja, String kontrahent, String produkt, String wagaNettoProduktu) {
  if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      
     
      http.begin(serverNamePrzewozy);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
      String httprequest ="key=" + apikey + "&carrier=" + przewoznik + "&vehicle_registration=" + rejestracja + "&contractor=" + kontrahent + "&product=" + produkt + "&product_net_weight=" + wagaNettoProduktu;
      
      Serial.println(httprequest);
      
      int httpResponseCode = http.POST(httprequest);
      String httpResponseText = http.getString();
      
      Serial.println("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.println("HTTP Response Text: ");
      Serial.println(httpResponseText);

      http.end();

  }
  else {
    Serial.println("WiFi Disconnected");
  }
  }


void POSTRequestOdbiory(String rejestracja, int punktodbioru) 
{ 
      HTTPClient http;
      
      http.begin(serverOdbiory);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
      String httprequest ="key=" + apikey + "&vehicle_registration=" + rejestracja + "&pickup_point=" + String(punktodbioru);
      
      Serial.println(httprequest);

      int httpResponseCode = http.POST(httprequest);
      String httpResponseText = http.getString();
      
      Serial.println("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.println("HTTP Response Text: ");
      Serial.println(httpResponseText);
      
      http.end();
 }


 void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

String get_waga()
{
  
  int i = 0;
  String waga = "";


  
  while (Serial2.available() > 0) 
  {
   
  tabb[i] = Serial2.read();
  delay(30);

  
  //Serial.print(tabb[i]);
  
     if(i==11)
    {
      Serial.println();
        waga.concat((char)tabb[2]);
        waga.concat((char)tabb[3]);
        waga.concat((char)tabb[4]);
        waga.concat((char)tabb[5]);
        waga.concat((char)tabb[6]);
        waga.toInt();
   
   
      Serial2.end();
      Serial2.begin(9600);

      
        
      
 for(int j=0;j<12;j++)
   {
   tabb[j] = '0';
   }
      
      return waga;
    }
     i++;
  }
  return "err";
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%x %H:%M");
  printer.print(&timeinfo, "%x %H:%M");
 
}

void drukuj(String rejestracja, String kontrahent,String produkt,String waga, String tara, String waga_netto)
{

 
  
  printer.doubleHeightOn();
  printer.justify('C');
  printer.boldOn();
  printer.setSize('L');
  printer.print("ROBSON");
  printer.feed(2);
  printer.setSize('S');
  printer.justify('L');
  printer.boldOff();
  printer.doubleHeightOff();
  printer.print("Robson Kopalnia nr 9");
  printer.feed(2);
  printer.print("Nr wazenia:\t 1");
  printer.feed();
  printer.print("Nr pojazdu:\t " + rejestracja);
  printer.feed(2);
  printer.print("KONTRAHENT:\t " + kontrahent);
  printer.feed();
  printer.print("PRODUKT:\t " + produkt);
  printer.feed();
  printer.print("TYP POJAZDU: 4 osiowy");
  printer.feed(2);
  printer.print("Wazenie:  \t " + waga + " kg");
  printer.feed();
  printer.print("Tarowanie:\t " + tara + " kg");
  printer.feed();
  printer.print("Waga netto:\t " + waga_netto + " kg");
  printer.feed();
  printer.print("Data:\t\t ");
  printLocalTime();
  printer.feed(4);
}
