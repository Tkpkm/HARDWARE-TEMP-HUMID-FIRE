#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wifi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define TEMP_HU 18
#define DHTTYPE DHT11
#define A0 33
#define D0 13

DHT dht(TEMP_HU, DHTTYPE);

int Flame;
int Humid;
int Temp;
int Id = 1;
String What_Alert;

void GET_All();
void PUT_Status();
void Connect_Wifi();
TaskHandle_t TaskMinMax = NULL;
void GET_min_max(void *param);



void setup(){
  Serial.begin(115200);
  Connect_Wifi();
  xTaskCreatePinnedToCore(GET_min_max, "GET_min_max", 15000, NULL, 1, &TaskMinMax, 0);
}

int min_temp;
int max_temp;
int min_humid;
int max_humid;
float humidity;
float temperature;

int sensor_flame = 1;
int sensor_humid = 1;
int sensor_temp = 0;

void GET_All(){
  while(1){
    DynamicJsonDocument doc(150000);
    const String url = "https://ecourse.cpe.ku.ac.th/exceed04/status/1";
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();
  
    if (httpResponseCode >= 200 && httpResponseCode < 300){
      Serial.println(httpResponseCode);
      String payload = http.getString();
      //Serial.println(payload);
      deserializeJson(doc,payload);

      //Serial.println(payload);
      min_temp = doc["min_temp"];
      max_temp = doc["max_temp"];
      min_humid = doc["min_humid"];
      max_humid = doc["max_humid"];
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
 
  }
}




void PUT_Status(){
  String json;
  DynamicJsonDocument doc(65203);
  doc["safe_id"] = Id;
  doc["flame_alert"] = sensor_flame;
  doc["humid_alert"] = sensor_humid;
  doc["temp_alert"] = sensor_temp;
  doc["ultrasonic_alert"] = 0;
  
  serializeJson(doc,json);

  const String url = "https://ecourse.cpe.ku.ac.th/exceed04/alert";
  HTTPClient http;
  http.begin(url);

  int httpResponseCode = http.PUT(json);
  if (httpResponseCode >= 200 && httpResponseCode < 300) {
    Serial.print("HTTP ");
    Serial.println(httpResponseCode);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  } 
}

void loop(){
  
  //temp and humid
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  //Serial.print("Humidity : ");
  //Serial.println(humidity);
  //Serial.print("Temperature : ");
  //Serial.println(temperature);
  
  if (temperature < min_temp || temperature > max_temp){
    sensor_temp = 1;
    Serial.println("temp alert !");
    PUT_Status();
  }else{
    sensor_temp = 0;
    PUT_Status();
  }

  if (humidity < min_humid || humidity > max_humid){
    Serial.println("humidity alert !");
    sensor_humid = 1;
    PUT_Status();
  }else{
    sensor_humid = 0;
    PUT_Status();
  }
  

  
  
  //vTaskDelay(1000/portTICK_PERIOD_MS);
  

  // flame
 
  //vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println(analogRead(A0));
  if (analogRead(A0) == 0){
    Serial.println("FIRE!!!!");
    sensor_flame = 1;
    PUT_Status();
    
  }
  sensor_flame = 0;
  PUT_Status();

  vTaskDelay(100/portTICK_PERIOD_MS);  

}



void GET_min_max(void *param){
  while(1){
    GET_All();
    vTaskDelay(1000/portTICK_PERIOD_MS);


  }
}




void Connect_Wifi()
{
  const char *ssid = "Texxy";
  const char *password = "300300300";
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("OK! IP=");
  Serial.println(WiFi.localIP());
  Serial.println("----------------------------------");
}
