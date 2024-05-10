#include <WiFi.h>
#include "DHT.h"

// local dependencies
#include "SystemConfig.h"
#include "UserConfig.h"
#include "MQTTHandler.h"

// Initialize MQTT
MQTTHandler mqttHandler;

// Initialize RTOS routines
TaskHandle_t hSensingTask;
TaskHandle_t hOutputControlTask;
TaskHandle_t hWiFiMonitoringTask;

// FPs
uint32_t _millis(void);

// Creating instances
DHT dht(DHT_PIN, DHTTYPE);

Sensors sensor;
bool irrigationPrevState = false;

SemaphoreHandle_t mutex;

void setup() {

  Serial.begin(9600);
  delay(1000);

  dht.begin();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_ON);
  delay(200);
  digitalWrite(RELAY_PIN, RELAY_OFF);
  delay(200);
  digitalWrite(RELAY_PIN, RELAY_ON);
  delay(200);
  digitalWrite(RELAY_PIN, RELAY_OFF);
  delay(200);

  irrigationPrevState = digitalRead(RELAY_PIN) == HIGH ? IRRIGATION_OFF : IRRIGATION_ON;

  
  // We start by connecting to a WiFi network
  Serial.print(F("\nConnecting to "));
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
#ifdef DEBUG_WIFI
  Serial.println(F(""));
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
#endif

 // Initialize MQTT
  mqttHandler.init(mqttServer, mqttPort, mqttUser, mqttPassword, deviceESN);
  

  // Create the mutex
  mutex = xSemaphoreCreateMutex();

  Serial.println(F("Initializing System"));

  // Create RTOS Tasks
  xTaskCreatePinnedToCore(SensingTask,       "Sensors Monitor",    2048, NULL, 1, &hSensingTask,        0);
  xTaskCreatePinnedToCore(OutputControlTask,  "Output Control",    1024, NULL, 2, &hOutputControlTask,  0);
  xTaskCreatePinnedToCore(WiFiMonitoringTask,   "WiFi Monitor",    2048, NULL, 1, &hWiFiMonitoringTask, 1);
  delay(500);

}

void loop() {

}
