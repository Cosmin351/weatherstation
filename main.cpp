#include <Arduino.h>
#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h>
#include <WiFi.h>
#include "DHT.h"
#include "time.h"
#include <HTTPClient.h>
#include <Arduino_JSON.h>


#define LCD_CS 33 /// Chip Select goes to Analog 3
#define LCD_RS 15 // Command/Data goes to Analog 2
#define LCD_WR 4 /// LCD Write goes to Analog 1
#define LCD_RD 2 /// LCD Read goes to Analog 0
#define LCD_RESET 32
#define DHT11PIN 5


#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

MCUFRIEND_kbv tft;
DHT dht(DHT11PIN, DHT11);

const char* SSIDPARAM = "gioconda";//"DIGI-02225969";
const char* PASSPARAM = "cristian";//"6ht5Qeee";
const char* ntpServer = "pool.ntp.org";

char hournow[3];
char Lastminute[] = "nan";
char day[10];

int CityTemp = 0;
int CityHum = 0;
int CityTempMax = 0;
int CityTempMin = 0;

String openWeatherMapApiKey = "2693de7ea633224657701690bcbe4f42";
String hostname = "ESP32 Termometru";
String City = "Bucharest";
String countryCode = "RO";
String jsonBuffer;
String CityWeather;

bool WIFICONNECTED = false;
bool gotWIFIPARAMS = false;

const int ScreenWidth = 320;
const int ScreenHeight = 240;
const int daylightOffset_sec = 10800;
const long gmtOffset_sec = 0;

long int LastMillis;

void initTFT();
void initWIFI();
int workTIME();
tm getTIME();
void DisplayConnnectInfo(int WhatToPrint);
void connectToWifi();
void displayERROR();
String httpGETRequest(const char* serverName);
void getWeather();

void setup() {
  Serial.begin(115200);
  initWIFI();
  Serial.println("Wifi inited.");
  initTFT();
  Serial.println("TFT inited.");
  dht.begin();
  getWeather();
  LastMillis = millis();
}

void loop() {
  int loops = 0;
  if(millis() - LastMillis > 300000){
    LastMillis = millis();
    Serial.println("new data");
    getWeather();
  }
  DisplayConnnectInfo(3);
  delay(5000);
  DisplayConnnectInfo(4);
  delay(5000);
  strcpy(Lastminute, "nan");
  workTIME();
  DisplayConnnectInfo(5);
  while(loops < 30){
    if(workTIME()){
      DisplayConnnectInfo(5);
    }
  loops += 1;
  delay(1000);
  }
}

void initTFT(){
  uint16_t ID = tft.readID();
  Serial.print("ID = 0x");
  Serial.println(ID, HEX);
  if (ID == 0xD3D3) ID = 0x9481;
  tft.begin(ID);
  tft.setRotation(1); 
}

void initWIFI(){
  WiFi.mode(WIFI_STA);
  DisplayConnnectInfo(1);
  connectToWifi();
  DisplayConnnectInfo(2);
  delay(500);
}

int workTIME(){
  tm timenow = getTIME();
  char minutenow[3];
  strftime(hournow, 3, "%H", &timenow);
  strftime(minutenow, 3, "%M", &timenow);
  strftime(day, 10, "%A", &timenow);
  if(strcmp(minutenow, Lastminute) != 0){
      strcpy(Lastminute, minutenow);
      return 1;
  }
  return 0; 
}

tm getTIME(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    displayERROR();
  }
  else{
    return timeinfo;  
  }
}

void displayERROR(){
  tft.fillScreen(RED);
  tft.setTextSize(4);
  tft.setCursor(40*ScreenWidth/100, 1/2*ScreenHeight);
  tft.print("Eroare");
  while(true){}
}

void DisplayConnnectInfo(int WhatToPrint){
  tft.fillScreen(BLACK);
  if(WhatToPrint == 1){
    tft.setCursor(25*ScreenWidth/100, 45*ScreenHeight/100);
    tft.println("Connecting to wifi...");
  }
  if(WhatToPrint == 2){
    tft.setCursor(25*ScreenWidth/100, 45*ScreenHeight/100);
    tft.println("Connected successfully.");
  }
  if(WhatToPrint == 3){
    int tempT = dht.readTemperature();
    int tempH = dht.readHumidity();
    tft.setCursor(5*ScreenWidth/100, 0);
    tft.setTextSize(4);
    tft.println("In interior:");
    tft.setTextSize(8);
    tft.setCursor(5*ScreenWidth/100, 45*ScreenHeight/100);
    tft.print(tempT);
    tft.setTextSize(4);
    tft.print("C");
    tft.setTextSize(8);
    tft.setCursor(60*ScreenWidth/100, 45*ScreenHeight/100);
    tft.print(tempH);
    tft.setTextSize(4);
    tft.print("%");

  }
  if(WhatToPrint == 4){
    tft.setCursor(10*ScreenWidth/100, 0);
    tft.setTextSize(5);
    tft.println(City);
    tft.setTextSize(8);
    tft.setCursor(5*ScreenWidth/100, 45*ScreenHeight/100);
    tft.print(CityTemp);
    tft.setTextSize(4);
    tft.print("C");
    tft.setTextSize(8);
    tft.setCursor(60*ScreenWidth/100, 45*ScreenHeight/100);
    tft.print(CityHum);
    tft.setTextSize(4);
    tft.print("%");
  }
  if(WhatToPrint == 5){
    tft.setCursor(35*ScreenWidth/100, 0);
    tft.setTextSize(5);
    tft.println("Ora:");
    tft.setCursor(5*ScreenWidth/100, 30*ScreenHeight/100);
    tft.setTextSize(10);
    tft.print(hournow);
    tft.print(":");
    tft.print(Lastminute);
    tft.setCursor(20*ScreenWidth/100, 75*ScreenHeight/100);
    tft.setTextSize(6);
    tft.println(day);
  }
  if(WhatToPrint == 6){
    tft.setCursor(20*ScreenWidth/100, 0);
    tft.setTextSize(4);
    tft.println("Prognoza");
    tft.setTextSize(5);
    tft.setCursor(10*ScreenWidth/100, 25*ScreenHeight/100);
    tft.print("Min");
    tft.setCursor(60*ScreenWidth/100, 25*ScreenHeight/100);
    tft.print("Max");
    tft.setTextSize(8);
    tft.setCursor(10*ScreenWidth/100, 55*ScreenHeight/100);
    tft.print(CityTempMin);
    tft.setCursor(60*ScreenWidth/100, 55*ScreenHeight/100);
    tft.print(CityTempMax);
    tft.setTextSize(9);
    tft.setCursor(30*ScreenWidth/100, 75*ScreenHeight/100);
    //tft.println(CityWeather);
  }
}

void connectToWifi(){
  WiFi.begin(SSIDPARAM, PASSPARAM);
  WIFICONNECTED = true;
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, serverName);
  int httpResponseCode = http.GET();
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  return payload;
}

void getWeather(){
  String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + City + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
  jsonBuffer = httpGETRequest(serverPath.c_str());
  JSONVar myObject = JSON.parse(jsonBuffer);
  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }
  CityTemp = int(myObject["main"]["temp"]) - 273;
  CityHum = int(myObject["main"]["humidity"]);
  CityWeather = JSON.stringify(myObject["weather"]["main"]);
  //Serial.println(CityWeather);
  CityTempMax = int(myObject["main"]["temp_max"]) - 273;
  CityTempMin = int(myObject["main"]["temp_min"]) - 273;
      Serial.print("JSON object = ");
      Serial.println(myObject);
}
