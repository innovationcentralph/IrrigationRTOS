void SensingTask( void * pvParameters ) {

  Serial.print(F("Sensor Monitoring running on core "));
  Serial.println(xPortGetCoreID());
  #ifdef DEBUG_SENSORS
    uint32_t printMillis = 0;
  #endif
  for (;;) {
    /* TODO: Read Temp and Humidity
     *       Read Soil Moisture -> Calibrate
     */
     
     

     if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        sensor.humidity    = dht.readHumidity();
        sensor.temperature = dht.readTemperature();
        sensor.soilMoisture = map(analogRead(SOIL_PIN), MOISTURE_CAL_DRY, MOISTURE_CAL_WET, 0, 100);
        sensor.soilMoisture = constrain(sensor.soilMoisture, 0, 100);
        xSemaphoreGive(mutex);
     }
     #ifdef DEBUG_SENSORS
     if(_millis() - printMillis > SENSOR_DEBUG_PRINT_INTERVAL){
        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
          
          Serial.println(" - - - - - Sensor Logs - - - - - ");
          Serial.println("  Temperature: " + String(sensor.temperature));
          Serial.println("     Humidity: " + String(sensor.humidity));
          Serial.println("Soil Moisture: " + String(sensor.soilMoisture));
          Serial.println("\n\n");
          
          xSemaphoreGive(mutex);
        }
        printMillis = _millis();
     }
     #endif

     /* Send Data to MQTT */
    

     vTaskDelay(pdMS_TO_TICKS(SENSOR_READING_INTERVAL)); 
  }

}

void OutputControlTask(void * pvParameters) {

  Serial.print(F("Output Control running on core "));
  Serial.println(xPortGetCoreID());

 

  for (;;) {
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        
        digitalWrite(RELAY_PIN, (sensor.soilMoisture < DRY_MOIST_TH) ? RELAY_ON : (sensor.soilMoisture > WET_MOIST_TH) ? RELAY_OFF : digitalRead(RELAY_PIN));
        xSemaphoreGive(mutex);
     }
    
     vTaskDelay(pdMS_TO_TICKS(1)); 
  }

}

void WiFiMonitoringTask( void * pvParameters ) {

  Serial.print("WiFi Control running on core ");
  Serial.println(xPortGetCoreID());

  static uint32_t sendingMillis = 0;
  for (;;) {
    // monitor MQTT Connectivity
    mqttHandler.checkConnectivity();
    
    if(_millis() - sendingMillis > WEB_UPDATE_INTERVAL){
      
      if (mqttHandler.checkConnectivity()) {
        
          if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
            
            String mqttMSG = "{\"SM\":\"" + String(sensor.soilMoisture) + "\", \"T\":\"" + String(sensor.temperature, 1) + "\", \"H\":\"" + String(sensor.humidity, 1) + "\"}"; 
            
            
            
            #ifdef DEBUG_MQTT
              Serial.print("Sending logs: ");
              Serial.print("UID: "); Serial.println(mqttMSG.c_str());
              Serial.println();
            #endif
            mqttHandler.publish("P006sensor", mqttMSG.c_str());

            xSemaphoreGive(mutex);
          }
          
        }
        sendingMillis = _millis();
    }
    
    
    
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {

     
      if (irrigationPrevState != digitalRead(RELAY_PIN) ? IRRIGATION_OFF : IRRIGATION_ON) {
        
        irrigationPrevState = digitalRead(RELAY_PIN) ? IRRIGATION_OFF : IRRIGATION_ON;
    
        // Execute the code block only if there's a state change
        String mqttMSG = "{\"SM\":\"" + String(sensor.soilMoisture) + "\", \"T\":\"" + String(sensor.temperature, 1) + "\", \"H\":\"" + String(sensor.humidity, 1) + "\"}"; 
        #ifdef DEBUG_MQTT
          Serial.println("On Change Event Detected!");  
          Serial.print("UID: "); Serial.println(mqttMSG.c_str());
          Serial.println();
        #endif
        mqttHandler.publish("P006sensor", mqttMSG.c_str());
      }
      
      // Release the mutex
      xSemaphoreGive(mutex);
    }

  vTaskDelay(pdMS_TO_TICKS(SENSOR_READING_INTERVAL)); 
  }

}

uint32_t _millis(void) {
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}
