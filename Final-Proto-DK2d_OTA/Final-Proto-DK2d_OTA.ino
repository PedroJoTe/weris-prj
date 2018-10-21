#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define DHTPINI 2     //pin inside
//#define DHTPINO 15     //pin outside
#define DHTTYPE DHT11 
#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

const int ledhij        = 15; //D6
const int buttonPin     = 5;  //D3
const int threshold     = 26; 
float ti, hi, to, ho    = 0;
float ti_max, hi_max    = 0;
float ti_min, hi_min    = 102;
float to_max, ho_max    = 0;
float to_min, ho_min    = 102;
float data[26];
int illu, nn            = 0;
 
DHT dhtis(DHTPINI, DHTTYPE);//instance1 inside
WiFiClient client;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(ledhij, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  digitalWrite(ledhij, LOW);
  digitalWrite(BUILTIN_LED, LOW);       
  Serial.begin(115200);
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);// initialize with the I2C addr 0x3C (for the 128x32)(initializing the display)
  

  display.clearDisplay();
  // START
  FrameSatu();
  delay(2000);
  display.clearDisplay();
  FrameDua();
  delay(2000);
  display.clearDisplay();
  // END
 
  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.println("Pinrolinvic");
  display.print(">>> Please Wait..");
  display.display();
  delay(2000);
 // text display tests
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Connecting to : ");
  display.println("");
  display.print("....");
  display.display();
  delay(10);
  display.clearDisplay();
  Serial.println("Booting");
  
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");
  // No authentication by default
  ArduinoOTA.setPassword((const char *)"123");
  ArduinoOTA.onStart([]() {
  Serial.println("Start");
    });
  ArduinoOTA.onEnd([]() {
  Serial.println("\nEnd");
    });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
  ArduinoOTA.onError([](ota_error_t error) {
  Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
        //mulai sini
   digitalWrite(BUILTIN_LED, HIGH);
       // text display tests
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(10,10);
  display.print(".. Wi-Fi Connected");
  display.display();
  delay(1000);
  display.clearDisplay();     
   }

void loop() {
  ArduinoOTA.handle();
    //mulai disini   
    if (nn < threshold){ 
      nn++;
      delay(1000);
      illu = analogRead(A0);
      delay(500);
      dhti();
      //cari max temp
      if(ti > ti_max) {ti_max = ti;}
      if(ti < ti_min) {ti_min = ti;}
      //cari max humid
      if(hi > hi_max)  { hi_max = hi; }
      if(hi < hi_min)  { hi_min = hi;}
      delay(50);

      Serial.print("DATA Illu/T_inside : ");
            Serial.print("\t");
            Serial.print(illu);
            Serial.print("\t");
            Serial.print(ti);
            Serial.print("\t");
            Serial.print(ti_max);
            Serial.println("loop ke - ");
            Serial.println(nn);
            display.clearDisplay();
            FrameDua();
            delay(2000);
            DisplayOled();  
            display.display();
    
    }  else{
     delay(1000);
     //kirimdatai();
      delay(1000);
      Serial.print("Kirim data I/T_in/T_out : ");
            Serial.print(illu);
            Serial.print("\t");
            Serial.print(ti_max);
           
      nn = 0;
      ti_max = 0;
      hi_max = 0;
      ti_min = 102;
      hi_min = 102;
      
    }
 }

void dhti() {
    hi = dhtis.readHumidity();
    ti = dhtis.readTemperature();
    if (isnan(hi) || isnan(ti)) 
    {
      Serial.println("Gagal baca DHT !!");
      return;
    }
}

void DisplayOled(){
  delay(100);
  //tampilan oled
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20,0);
  display.print("Pinrolinvic.com"); //  baris 0 
  display.setCursor(0,0+5);
  display.print("---------------------");
  display.setCursor(3,13);//baris 1
  display.print("Humid  :  "); 
  display.print(hi,2);
  display.print(" %"); 
  display.setCursor(3,13+10);//baris 2
  display.print("Temp   :  "); 
  display.print(ti,1);
  display.print(" C");
}

void FrameSatu(void) {
    uint8_t color = WHITE;
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
    delay(1);
  }
}

void FrameDua(void) {
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, WHITE);
    display.display();
    delay(1);
  }
}

