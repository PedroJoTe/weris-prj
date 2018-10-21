#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DHT.h>

#define DHTPINI 14     //pin inside D5
#define DHTPINO 5     //pin outside D1
#define DHTTYPE DHT11

const char* server      = "pinrolinvic.com"; // alamat server
const char* ssid        = "Torang"; // SSID router wifi
const char* password    = "p"; // password wifi
const char* SensorID    = "PinDK.Simulator.LDR1"; // ID sensor
const char* SensorIDI   = "PinDK.Simulator.DHT1"; // ID inside
const char* SensorIDO   = "PinDK.Simulator.DHT2"; // ID outside
const char* segel       = "opened"; // sensor box status
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
DHT dhtos(DHTPINO, DHTTYPE);//instance2 outside
WiFiClient client;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(ledhij, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  digitalWrite(ledhij, LOW);
  digitalWrite(BUILTIN_LED, LOW);
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(100);
    ESP.restart();

  }

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
}

void loop() {
  ArduinoOTA.handle();
  //mulai disini
  if (nn < threshold) {
    nn++;
    delay(1000);
    illu = analogRead(A0);
    delay(500);
    dhti();
    //cari max temp
    if (ti > ti_max) {
      ti_max = ti;
    }
    if (ti < ti_min) {
      ti_min = ti;
    }
    //cari max humid
    if (hi > hi_max)  {
      hi_max = hi;
    }
    if (hi < hi_min)  {
      hi_min = hi;
    }
    delay(500);
    dhto();
    //cari max temp
    if (to > to_max) {
      to_max = to;
    }
    if (to < to_min) {
      to_min = to;
    }
    //cari max humid
    if (ho > ho_max)  {
      ho_max = ho;
    }
    if (ho < ho_min)  {
      ho_min = ho;
    }

    Serial.print("DATA Illu/T_inside/T_outside : ");
    Serial.print("\t");
    Serial.print(illu);
    Serial.print("\t");
    Serial.print(ti);
    Serial.print("\t");
    Serial.print(to);
    Serial.print("\t");
    Serial.print(ti_max);
    Serial.print("\t");
    Serial.println(to_max);
    Serial.println("loop ke - ");
    Serial.println(nn);
  }  else {
    kirimdata();
    delay(1000);
    kirimdatai();
    delay(1000);
    kirimdatao();
    delay(1000);
    Serial.print("Kirim data I/T_in/T_out : ");
    Serial.print(illu);
    Serial.print("\t");
    Serial.print(ti_max);
    Serial.print("\t");
    Serial.println(to_max);
    nn = 0;
    ti_max = 0;
    hi_max = 0;
    ti_min = 102;
    hi_min = 102;
    to_max = 0;
    ho_max = 0;
    to_min = 102;
    ho_min = 102;
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

void dhto() {
  ho = dhtos.readHumidity();
  to = dhtos.readTemperature();
  if (isnan(ho) || isnan(to))
  {
    Serial.println("Gagal baca DHT !!");
    return;
  }

  /*
         if (client.connect(server,80)) {
         String postStr = apiKey;
         postStr +="&field1=";
         postStr += String(t);
         postStr +="&field2=";
         postStr += String(h);
         postStr += "\r\n\r\n";

         client.print("POST /update HTTP/1.1\n");
         client.print("Host: api.thingspeak.com\n");
         client.print("Connection: close\n");
         client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
         client.print("Content-Type: application/x-www-form-urlencoded\n");
         client.print("Content-Length: ");
         client.print(postStr.length());
         client.print("\n\n");
         client.print(postStr);
  */
}

void kirimdata() {
  //kirim
  if (client.connect(server, 80)) {
    Serial.println("Mengirim data...");
    digitalWrite(BUILTIN_LED, LOW);
    client.println("GET /WERIS/i11.php?value1=" + String(illu) + "&value2=" + String(illu) + "&value3=" + String(illu) + "&state=" + String(segel) + "&id=" + SensorID + " HTTP/1.1");
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

void kirimdatai() {
  //kirim
  if (client.connect(server, 80)) {
    Serial.println("Mengirim data...");
    digitalWrite(BUILTIN_LED, LOW);
    client.println("GET /WERIS/i11.php?value1=" + String(hi_max) + "&value2=" + String(ti_max) + "&value3=" + String(ti_max) + "&state=" + String(segel) + "&id=" + SensorIDI + " HTTP/1.1");
    client.println("HOST: pinrolinvic.com");
    client.println("Connection: close");
    client.println();
    client.stop();
  } else {
    Serial.println("Koneksi ke server gagal");
  }
  delay(500);
  digitalWrite(BUILTIN_LED, HIGH);
}

void kirimdatao() {
  //kirim
  if (client.connect(server, 80)) {
    digitalWrite(ledhij, HIGH);
    Serial.print("Mengirim data...");
    digitalWrite(BUILTIN_LED, LOW);
    client.println("GET /WERIS/i11.php?value1=" + String(ho_max) + "&value2=" + String(to_max) + "&value3=" + String(to_max) + "&state=" + String(segel) + "&id=" + SensorIDO + " HTTP/1.1");
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
