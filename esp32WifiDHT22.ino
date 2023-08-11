//Include required libraries
#include "WiFi.h"
#include <HTTPClient.h>
#include "time.h"

#include "Wire.h"
#include <DHT.h>
#include <LiquidCrystal_I2C.h>


const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

// WiFi credentials
const char* ssid = ""; 
const String sheet = "";
// Google script ID and required credentials
String GOOGLE_SCRIPT_ID = "";

//Sensores e variáveis de dados

double umidade1 = 0;
double temperatura1 = 0;

int contagem = 1;

int countdown = 0;
//-----------------------------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

DHT dht1(23, 22);

int relay = 18;


void setup() {

  pinMode(relay, OUTPUT);
  //-------------------------------------------
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.home();
  
  
  //-------------------------------------------
  dht1.begin();
  Serial.begin(115200);
  delay(1000);

  // connect to WiFi
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();
  WiFi.begin(ssid);

  delay(1500);
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
void loop() {
  temperatura1 = dht1.readTemperature();
  umidade1 = dht1.readHumidity();
  if(WiFi.status() != WL_CONNECTED){
    WiFi.reconnect();
    Serial.println("desconectado");
  }


  if (countdown == 0) {
    if(WiFi.status() == WL_CONNECTED){
      contagem += 1;
      String url = assembleUrl(sheet, temperatura1, umidade1);
      deployData(url);
      Serial.println("Deploy");
      
    }
    countdown = 1800;
  }
  if(umidade1 > 50){
    digitalWrite(relay, LOW);
  }
  if(umidade1 <= 50){
    digitalWrite(relay, HIGH);
  }
  lcd.home();
  lcd.clear();
  lcd.print("Temp: " + String(dht1.readTemperature()) + " C");
  lcd.setCursor(0, 1);
  lcd.print("Umi: " + String(dht1.readHumidity()) + " %"); 
  
  delay(1000);
  countdown = countdown-1;
  Serial.println(countdown);
}

String assembleUrl(String sheet, double temp1, double umi1) {
  static bool flag = false;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }
  char timeStringBuff[50];  //50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);

  String asString(timeStringBuff);
  asString.replace(" ", "-");
  String urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + "sheet=" + sheet + "&contagem=" + contagem + "&date=" + asString + "&temp1=" + temp1 + "&umi1=" + umi1;
  return urlFinal;
}

void deployData(String urlFinal) {
  Serial.println(urlFinal);
  HTTPClient http;
  http.begin(urlFinal.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  String payload;
  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("Payload: " + payload);
  }
  http.end();
}