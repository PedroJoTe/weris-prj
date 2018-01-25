 /*
 * Kode sumber ini dibuat untuk digunakan dengan produk Smart-meter buatan PDKM
 * 1 sensor arus dan 1 sensor tegangan di hubungkan ke Analog Input
 * Display yang digunakan berupa OLED I2C dihubungkan pada Pin D1 dan D2
 * Switch untuk Segel dibubngkan pada D3 
 * sebagai indikator digunakan 3 buah LED
 * merah = wifi connected
 * hijau = data/internet connected
 * biru = data dikirim
 * periode pengiriman setiap 15 second dapat di atur
 * kalibrasi harus dilakukan sebelum penggunaan, untuk mengatur tegangan referensi sensors
 * penggunaan kode sumber silahkan menghubungi pmanembu@gmail.com
 * www.pinrolinvic.com
 * 
 * copenhagen 2017
 * 
 * P.S : alamat wifi manager, 192.168.1.104
 */

#include <ESP8266WebServer.h>
#include <WiFiManager.h>  
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
//#include <DNSServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

const unsigned long sampleTime      = 100000UL; // sample over 100ms, it is an exact number of cycles for both 50Hz and 60Hz mains 
const unsigned long numSamples      = 250UL; // choose the number of samples to divide sampleTime exactly, but low enough for the ADC to keep up 
const unsigned long sampleInterval  = sampleTime/numSamples; // the sampling interval, must be longer than then ADC conversion time 
const char* beacon      = "Intelligensia";                    // beacon ID
const char* segel       = "opened"; // ID sensor
const char* server      = "pinrolinvic.com"; // alamat server
const char* SensorID    = "PinMD.Simulator.CVv3"; // ID sensor
//const char* SensorID2   = "PinDK.Simulator.DHT1xx"; // ID sensor
const int mpx           = 14; //D6
const int ledmer        = 13; //D7
const int ledhij        = 12; //D6
const int buttonPin     = 0;  //D3
float data[250];  
float current           =0; 
float voltage           =0;
unsigned long sebawal   = 0; 
unsigned long intervalku = 20000; 
int buttonState         = 0; 
long jarak = 10000;
long tbefore = 0;

WiFiClient client;
 
void setup() {
  Serial.begin(115200);
  delay(10);
  Wire.begin();
  delay(10);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);// initialize with the I2C addr 0x3C (for the 128x32)(initializing the display)
  
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(mpx, OUTPUT);
  pinMode(ledmer, OUTPUT);
  pinMode(ledhij, OUTPUT);
  //pinMode(buttonPin, INPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  digitalWrite(mpx, LOW);
    
          //clear buffer dulu
          display.clearDisplay();
          // START animasi awal
          FrameSatu();
          delay(2000);
          display.clearDisplay();
          FrameDua();
          delay(2000);
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(10,0);
          display.println("Pinrolinvic");
          display.print(">>> Please Wait..");
          display.display();
          delay(2000);
          display.clearDisplay();
          display.setTextColor(WHITE);
          display.setTextSize(1);
          display.setCursor(0,0);
          display.println("Setting Wi-Fi, di >>");
          display.println(beacon);
          display.println("192.168.4.1");
          display.print("....");
          display.display();
          delay(10);
          display.clearDisplay();
  kedip();
  Serial.print("Connecting to wi-fi");
  WiFiManager wifi;
   // wifi.resetSettings();
    wifi.autoConnect(beacon);
  Serial.println("");
  Serial.println("WiFi Connected");
          display.clearDisplay();
          display.setTextColor(WHITE);
          display.setTextSize(1);
          display.setCursor(10,10);
          display.print(".. Wi-Fi Connected");
          display.display();
          delay(1000);
          display.clearDisplay();
  digitalWrite(ledmer, HIGH);
  }

void loop() {
  unsigned long awal = millis();
  //Pembacaan Tegangan, Relay High
  digitalWrite(mpx, HIGH);
  Serial.println("Relay Position High");
  delay(1500);
  VoltageSense();
  delay(1500);
  Serial.print("Tegangan: ");
  Serial.println(voltage,1);
  //Pembacaan Arus
  digitalWrite(mpx, LOW);
  Serial.println("Relay Position Low");
  delay(500); 
  CurrentSense();
  delay(500);
  Serial.print("Arus  : ");
  Serial.println(current,2);
      display.clearDisplay();
  FrameDua();
  DisplayOled();  
      display.display();
      delay(1000);
  kirimdata();
  delay(6800); //jedah
  unsigned long akhir = millis();
  unsigned long jedah = akhir - awal;
  Serial.println(jedah);
}

void DisplayOled(void){
  delay(100);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20,0);
  display.print("Pinrolinvic.com"); //  baris 0 
  display.setCursor(0,0+5);
  display.print("---------------------");
  display.setCursor(3,13);//baris 1
  display.print("Voltage  : "); 
  display.print(voltage,1);
  display.print(" V"); 
  display.setCursor(3,13+10);//baris 2
  display.print("Current  : "); 
  display.print(current,3);
  display.print(" A");
}
 void kirimdata(){
 //kirim
  if (client.connect(server,80)) {
    digitalWrite(ledhij, HIGH);
    Serial.println("Mengirim data...");
    digitalWrite(BUILTIN_LED, LOW);
    client.println("GET /testy/dht22.php?value1=" + String(voltage,1) + "&value2=" + String(current,3) + "&state="+ String(segel) + "&id=" +SensorID+ " HTTP/1.1");
    client.println("HOST: pinrolinvic.com");
    client.println("Connection: close");
    client.println();
    client.stop();
   
  } else {
    Serial.println("Koneksi ke server gagal");
    digitalWrite(ledhij, LOW);
    }
   delay(500);
   digitalWrite(BUILTIN_LED, HIGH);
}

void VoltageSense() {
  delay(100);
  unsigned long currentAcc = 0;
  unsigned int count = 0;
  unsigned long prevMicros = micros() - sampleInterval ;
    while (count < numSamples) { 
      if (micros() - prevMicros >= sampleInterval){
        data[count] = analogRead(A0)*3.3/1024 - 1.601 ;// VCC/2, -->> Running
        count=count+1;
        prevMicros += sampleInterval;
      }
    } 
    //finding the RMS value of samples Read. 
    float sum=0;
    //squaring all the values 
    for(int i=0;i<250;i++){
      data[i]=(data[i])*(data[i]);
    } 
    //finding the sum 
    for(int i=0;i<250;i++){
      sum = sum+data[i]; 
    }
    voltage = (sqrt(sum/numSamples))*1000; // konvert ke Volt
    }

void CurrentSense() {
  delay(100);
  unsigned long currentAcc = 0;
  unsigned int count = 0;
  unsigned long prevMicros = micros() - sampleInterval ;
    while (count < numSamples) { 
      if (micros() - prevMicros >= sampleInterval){
        data[count] = analogRead(A0)*3.3/1024 - 1.604; //tuning VCC/2
        count=count+1;
        prevMicros += sampleInterval;
      }
    } 
    //finding the RMS value of samples Read. 
    float sum=0;
    //squaring all the values 
    for(int i=0;i<250;i++){
      data[i]=(data[i])*(data[i]);
    } 
    //finding the sum 
    for(int i=0;i<250;i++){
      sum = sum+data[i]; 
    }
    current = sqrt(sum/numSamples)*1000/100; // 20A=100, 30A=66; 5A=185, lihat data sheet. 
}

void kedip(){
  digitalWrite(ledmer, HIGH);
  delay(50);
  digitalWrite(ledmer, LOW);
  delay(50);
  digitalWrite(ledmer, HIGH);
  delay(50);
  digitalWrite(ledmer, LOW);
  delay(50);
  digitalWrite(ledmer, HIGH);
  delay(50);
  digitalWrite(ledmer, LOW);
  delay(50);
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

