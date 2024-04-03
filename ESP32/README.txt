
# MPPT Charge Controller Source Code

This is the source code for the MPPT charge controller, which runs on an ESP32 microcontroller. The circuit uses an external ADC and current sensor to measure input voltage and current. These are inputs for the "perturb-and-observe" MPPT algorithm, which dictates the duty cycle of a FET used in a buck-boost converter. A separate version of the code also allows the ESP32 to act as a Wi-Fi access point, enabling IoT functionality and for monitoring of circuit parameters over Wi-Fi.
