
#define DHTTYPE DHT11

#define DHT_PIN   25
#define SOIL_PIN  32
#define RELAY_PIN 33


typedef struct Sensors{
  float temperature;
  float humidity;
  float soilMoisture;
};

//MQTT Parameters
const char* mqttServer   = "3.27.210.100";
const int mqttPort       = 1883;
const char* mqttUser     = "mqtt";
const char* mqttPassword = "ICPHmqtt!";
const char* deviceESN    = "P006";


#define MOISTURE_CAL_DRY 2505
#define MOISTURE_CAL_WET 826

#define RELAY_ON  LOW
#define RELAY_OFF HIGH

#define IRRIGATION_ON  true
#define IRRIGATION_OFF false

//#define RELAY_MONITOR_INTERVAL 10
