#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

float pressure, temperature, altitude;

void setup() 
{
  Serial.begin(9600);
  Serial.println("MeteoStation v1.0.0"); Serial.println("");
  
  /* Initialize the sensor */
  if(!bmp.begin())
  {
    Serial.print("ERROR: BMP180 no detectado.");
    while(1);
  }
}

void loop() 
{
  read_bmp();
  delay(1000);
}

void read_bmp()
{
  sensors_event_t event;
  bmp.getEvent(&event);

  if (event.pressure)
  {
    /* Display atmospheric pressure in hPa */
    Serial.print("Presión:    ");
    Serial.print(event.pressure);
    Serial.println(" hPa");
    pressure = event.pressure;

    /* Display temperature in Celsius */
    bmp.getTemperature(&temperature);
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.println(" C");

    /* Then convert the atmospheric pressure, and SLP to altitude         */
    /* Update this next line with the current SLP for better results      */
    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
    altitude = bmp.pressureToAltitude(seaLevelPressure, event.pressure);
    Serial.print("Altitud:    "); 
    Serial.print(altitude); 
    Serial.println(" m");
    Serial.println("");
  }
  else
  {
    Serial.println("Error de lectura del BMP180");
  }
}
