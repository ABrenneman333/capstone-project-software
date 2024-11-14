#include <stdio.h>
#include "i2c.h"
#include "wifi.h"
#include "bme280-temp-sensor.h"
#include "veml7700-light-sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "mqtt.h"
#include "timer.h"
#include "tasks.h"


// Entry point for firmware //
void app_main(void)
{
    // Initialize the NVS for WiFi credential flash storage
    init_flash();

    // Initialize WiFi driver and event handling
    init_wifi();

    // Configure WiFi settings and connect to network
    config_wifi();

    // Wait for the WiFi driver to start and connect to a network before starting MQTT broker
    xSemaphoreTake(wifi_semaphore, portMAX_DELAY);

    // Initialize MQTT and connect to broker
    init_mqtt();

    // Initialize the I2C bus and mutex
    init_i2c();

    // Add BME280 temperature and humidity sensor to I2C bus
    add_bme_i2c();

    // Configure the BME now that it is added to the I2C bus
    configure_bme280();

    // Read the compensation values for the BME280
    read_compensation_bme280();

    // Add VEML7700 light sensor to I2C bus
    add_veml_i2c();

    // Configure the VEML now that it is added to the I2C bus
    configure_veml7700();

    // Wait to connect to MQTT broker before starting tasks
    xSemaphoreTake(mqtt_semaphore, portMAX_DELAY);

    // Start timer to manage task synchronization
    init_timer();

    // Create tasks to take sensor readings and send over MQTT
    xTaskCreate(temp_and_humidity_readings, "Temp/Humidity Readings Task", 65536, NULL, 5, NULL);
    xTaskCreate(light_readings, "Light Level Readings Task", 65536, NULL, 5, NULL);
    xTaskCreate(mqtt_publish, "MQTT Publishing Task", 65536, NULL, 5, NULL);

    // Start timer for tasks
    xTimerStart(sensor_timer, 0);

    while (1) 
    {
        // Pause app_main
        vTaskDelay(portMAX_DELAY);
    }
}
