#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecureBearSSL.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

unsigned long eventInterval = 60000;  
unsigned long previousTime  = 0;

//Web Server address to read/write from 
const char* BMKG  ="https://data.bmkg.go.id/DataMKG/TEWS/autogempa.json";
const char* SM    ="https://data.bmkg.go.id/DataMKG/TEWS/";
String Databmkg;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");


//Week Days
String weekDays[7]={"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};

//Month names
String months[12]={"Januari", "Februari", "Maret", "April", "Mei", "Juni", "July", "Agustus", "September", "Oktober", "November", "Desember"};

//Json
DynamicJsonDocument doc(1024);

String httpGETRequest(const char *BMKG){

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();

    HTTPClient https;

    https.begin(*client, BMKG);
    String payload;

  int response = https.GET();

  if (response == HTTP_CODE_OK){
    payload = https.getString();
  }

  else{
    Serial.print("Error code: ");
    Serial.println(response);
  }

  https.end();
  return payload;

}

void setup() {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    Serial.begin(9600);
    WiFiManager wm;
    //wm.resetSettings();
    bool res;
    res = wm.autoConnect("Smart Vase","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        timeClient.begin();
        timeClient.setTimeOffset(25200);
    }

}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - previousTime >= eventInterval) {
    Infogempa();
    previousTime = currentTime;
  }
  DTime();
}

void DTime() {
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  
  String weekDay = weekDays[timeClient.getDay()];
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  String currentMonthName = months[currentMonth-1];
  int currentYear = ptm->tm_year+1900;

  //Print complete date:
  String currentDate = (weekDay) + ", " + String(monthDay) + " " + (currentMonthName) + " " +  String(currentYear) ;

  Serial.print("Time.txt=");
  Serial.print("\"");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print("\"");
  ends();

  Serial.print("Date.txt=");
  Serial.print("\"");
  Serial.println(currentDate);
  Serial.print("\"");
  ends();

  delay(1000);
}

void Infogempa() {
    // put your main code here, to run repeatedly:   
    Databmkg = httpGETRequest(BMKG);
    //Serial.println("----- Request Result from API BMKG -----");
    //Serial.println(Databmkg);

    //Deserialize
    deserializeJson(doc, Databmkg);
    JsonObject Infogempa_gempa = doc["Infogempa"]["gempa"];
    const char* Infogempa_gempa_Tanggal = Infogempa_gempa["Tanggal"]; // "12 Jun 2022"
    const char* Infogempa_gempa_Jam = Infogempa_gempa["Jam"]; // "06:55:32 WIB"
    const char* Infogempa_gempa_Magnitude = Infogempa_gempa["Magnitude"]; // "5.3"
    const char* Infogempa_gempa_Wilayah = Infogempa_gempa["Wilayah"]; // "Pusat gempa berada di laut 64 km ...
    const char* Infogempa_gempa_Potensi = Infogempa_gempa["Potensi"]; // "Gempa ini dirasakan untuk ...
    const char* Infogempa_gempa_Shakemap = Infogempa_gempa["Shakemap"]; // "20220612065532.mmi.jpg"
    
    Serial.print("Tanggal.txt=");
    Serial.print("\"");
    Serial.print(Infogempa_gempa_Tanggal);
    Serial.print("\"");
    ends();

    Serial.print("Jam.txt=");
    Serial.print("\"");
    Serial.print(Infogempa_gempa_Jam);
    Serial.print("\"");
    ends();

    Serial.print("Magnitude.txt=");
    Serial.print("\"");
    Serial.print(Infogempa_gempa_Magnitude);
    Serial.print("\"");
    ends();

    Serial.print("Wilayah.txt=");
    Serial.print("\"");
    Serial.print(Infogempa_gempa_Wilayah);
    Serial.print("\"");
    ends();

    Serial.print("Potensi.txt=");
    Serial.print("\"");
    Serial.print(Infogempa_gempa_Potensi);
    Serial.print("\"");
    ends();

    Serial.print("Qr.txt=");
    Serial.print("\"");
    Serial.print(SM);
    Serial.print(Infogempa_gempa_Shakemap);
    Serial.print("\"");
    ends();
    
    delay(5000);
}

void ends(){
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}