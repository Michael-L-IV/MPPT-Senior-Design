#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_INA219.h>

// PWM settings
const int ledChannel = 0;
const int resolution = 8;
const int freq = 10000; // Frequency for MPPT control
const int outputPin = 5;

// ADS1115 settings
Adafruit_ADS1115 ads; // Use the default I2C address
const float multiplier = 0.1875F; // mV per bit for gain setting of 2/3x which allows for up to 6.144V

// MPPT settings
float D = 0.5; // Initial duty cycle
const float changeBy = 0.01; // Step change for duty cycle adjustment
float VAprev = 0;
float PAprev = 0;

// INA219 Current Sensor Setup
Adafruit_INA219 ina219;

void setup() {
  Serial.begin(115200);
  delay(1000); // Delay for serial communication stabilization

  // Initialize PWM channel
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(outputPin, ledChannel);

  // Initialize ADS1115
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1); // Halt if initialization fails
  }
  delay(1000);

  // Initialize INA219
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1);
  }
  ina219.setCalibration_16V_400mA();
  delay(1000);
  
}
float readCurrent() {
  float current_mA = ina219.getCurrent_mA(); // Current in mA from INA219
  return current_mA / 1000.0; // Convert to Amps
}

void loop() {
  // Read voltage and current
  int16_t adc0 = ads.readADC_SingleEnded(0);
  int16_t adc1 = ads.readADC_SingleEnded(1);
  float battValue0 = adc0 * multiplier * 0.001 ; // Apply voltage divider correction * (12330 / 330)
  // battValue0 += 0.34444; // Adjusting based on sensor behavior
  float current = readCurrent(); // Current in Amps

  // Ensure the system is active
  if (D == 0) {
    battValue0 = 0;
    current = 0; 
  }

  // Compute power and changes in power and voltage
  float PA = fabs(battValue0 * current); // Current power, use fabs for absolute value
  float DeltaPA = PA - PAprev;
  float DeltaVA = battValue0 - VAprev;

  // MPPT logic adjustment
  if (fabs(DeltaPA) > 0.001) { // Ensuring significant change
    if (DeltaPA > 0 && DeltaVA > 0) D -= changeBy;
    else if (DeltaPA > 0 && DeltaVA < 0) D += changeBy;
    else if (DeltaPA < 0 && DeltaVA > 0) D += changeBy;
    else if (DeltaPA < 0 && DeltaVA < 0) D -= changeBy;
  }

  // Ensure D stays within bounds
  D = constrain(D, 0, 0.9);

  // Update previous measurements
  VAprev = battValue0;
  PAprev = PA;

  // Adjust PWM duty cycle based on D
  int duty = D * ((1 << resolution) - 1);
  ledcWrite(ledChannel, duty);

  // Debugging outputs
  Serial.print("V: "); Serial.print(battValue0, 5);
  Serial.print(" V, I: "); Serial.print(current, 5);
  Serial.print(" A, P: "); Serial.print(PA, 5);
  Serial.print(" W, D: "); Serial.println(D, 5);

  delay(1000); // Delay for readability
}


