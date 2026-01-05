
#include "NanoProtoShield.h"
#include <EEPROM.h>

#define READING_DELAY 2000
#define MAX_TEMPERATURE 80
#define MIN_TEMPERATURE 40
#define MAX_READINGS 60  // Max readings to store (120s / 2s = 60 readings)
#define MIN_RELAY_CYCLE_TIME 30000  // Minimum 30 seconds between relay state changes
#define RELAY_PIN PIN_DEFAULT_IR_TX // Use the infrared transmit pin for the relay since we don't care about the IR system for this sketch
#define MIN_HYSTERESIS 0.5  // Minimum hysteresis tolerance (0.5°F)
#define MAX_HYSTERESIS 5.0  // Maximum hysteresis tolerance (5.0°F)
#define HYSTERESIS_STEP 0.5 // Hysteresis adjustment step size

// EEPROM addresses for stored settings
#define EEPROM_MAGIC_ADDR 0      // Magic number to verify EEPROM is initialized
#define EEPROM_SETTEMP_ADDR 1    // Address for set temperature (1 byte)
#define EEPROM_HYSTERESIS_ADDR 2 // Address for hysteresis (4 bytes for float)
#define EEPROM_MAGIC_VALUE 0x42  // Magic value to check if EEPROM has been initialized

NanoProtoShield g_nps(FEATURE_OLED | FEATURE_BUTTON_UP | FEATURE_BUTTON_DOWN | FEATURE_BUTTON_LEFT | FEATURE_BUTTON_RIGHT | FEATURE_SHIFTS);

unsigned int g_currentSetTemp;
float g_hysteresisTolerance = 1.0;  // Current hysteresis tolerance in degrees F
bool g_relayState = false;  // Track current relay state for hysteresis
unsigned long g_lastRelayChangeTime = 0;  // Track when relay last changed state

unsigned long g_lastTemperatureReadingTime;

// Circular buffer for temperature readings
float g_temperatureReadings[MAX_READINGS];
int g_readingIndex = 0;
int g_readingCount = 0;

void setup() {
  g_nps.begin();

  pinMode(RELAY_PIN , OUTPUT);

  // Load settings from EEPROM or use defaults
  loadSettingsFromEEPROM();

  g_nps.setUpButtonInterrupt(incrementCurrentSetTemp, BUTTON_PRESSED);
  g_nps.setDownButtonInterrupt(decrementCurrentSetTemp, BUTTON_PRESSED);
  g_nps.setLeftButtonInterrupt(decrementHysteresis, BUTTON_PRESSED);
  g_nps.setRightButtonInterrupt(incrementHysteresis, BUTTON_PRESSED);

  g_nps.takeTemperatureReading();
  g_lastTemperatureReadingTime = millis();
}

void loop() {
  unsigned long nowTime = millis();
  if( ( nowTime - g_lastTemperatureReadingTime ) > READING_DELAY ) {
    g_nps.takeTemperatureReading();
    addTemperatureReading(g_nps.getTempF());
    g_lastTemperatureReadingTime = nowTime;
    
    processAndSetHeatRelay();
  }

  printTemperatures();
}

void processAndSetHeatRelay() {
  float averageTemp = getAverageTemperature();
  float setTemp = (float)g_currentSetTemp;
  bool newRelayState = g_relayState;  // Start with current state
  unsigned long currentTime = millis();
  
  // Hysteresis control logic with configurable tolerance
  if (!g_relayState) {  // Relay is currently OFF
    // Turn ON if temperature drops below (setpoint - hysteresis)
    if (averageTemp <= (setTemp - g_hysteresisTolerance)) {
      newRelayState = true;
    }
  } else {  // Relay is currently ON
    // Turn OFF if temperature reaches setpoint (no overshoot needed)
    if (averageTemp >= setTemp) {
      newRelayState = false;
    }
  }
  
  // Update relay state only if it changed AND minimum cycle time has elapsed
  if (newRelayState != g_relayState) {
    // Check if enough time has passed since last relay state change
    if ((currentTime - g_lastRelayChangeTime) >= MIN_RELAY_CYCLE_TIME) {
      g_relayState = newRelayState;
      g_lastRelayChangeTime = currentTime;
      setRelay(g_relayState);
    }
    // If minimum cycle time hasn't elapsed, relay state remains unchanged
  }
}

void printTemperatures() {
  float averageTemp = getAverageTemperature();
  float currentTemp = g_nps.getTempF();
  
  // Display setpoint temperature on 7-segment display
  g_nps.shift7segPrint(g_currentSetTemp);
  
  g_nps.oledClear();
  g_nps.oledPrintln("Hello there!");
  g_nps.oledPrint("Current: ");
  g_nps.oledPrint(currentTemp, 1);
  g_nps.oledPrintln("F");
  g_nps.oledPrint("Avg(2min): ");
  g_nps.oledPrint(averageTemp, 1);
  g_nps.oledPrintln("F");
  g_nps.oledPrint("Set temp: ");
  g_nps.oledPrint(g_currentSetTemp);
  g_nps.oledPrintln("F");
  g_nps.oledPrint("Hysteresis: ");
  g_nps.oledPrint(g_hysteresisTolerance, 1);
  g_nps.oledPrintln("F");
  
  // Display relay state
  g_nps.oledPrint("Heat: ");
  g_nps.oledPrintln(g_relayState ? "ON" : "OFF");
  
  g_nps.oledDisplay();
}

void incrementCurrentSetTemp() {
  g_currentSetTemp++;

  if(g_currentSetTemp > MAX_TEMPERATURE) {
    g_currentSetTemp = MAX_TEMPERATURE;
  }
  
  saveSetTempToEEPROM();
}

void decrementCurrentSetTemp() {
  g_currentSetTemp--;

  if(g_currentSetTemp < MIN_TEMPERATURE) {
    g_currentSetTemp = MIN_TEMPERATURE;
  }
  
  saveSetTempToEEPROM();
}

// Add a new temperature reading to the circular buffer
void addTemperatureReading(float temperature) {
  g_temperatureReadings[g_readingIndex] = temperature;
  g_readingIndex = (g_readingIndex + 1) % MAX_READINGS;
  
  if (g_readingCount < MAX_READINGS) {
    g_readingCount++;
  }
}

// Calculate average temperature of all readings in the buffer
float getAverageTemperature() {
  if (g_readingCount == 0) {
    return 0.0;
  }
  
  float sum = 0.0;
  
  // Sum all readings in the buffer
  for (int i = 0; i < g_readingCount; i++) {
    sum += g_temperatureReadings[i];
  }
  
  return sum / g_readingCount;
}

// Set the relay state and visual indicator
void setRelay(bool relayOn) {
  digitalWrite(RELAY_PIN, (relayOn? HIGH:LOW));
  g_nps.shiftLedWrite(relayOn? 1:0);
}

void incrementHysteresis() {
  g_hysteresisTolerance += HYSTERESIS_STEP;
  
  if(g_hysteresisTolerance > MAX_HYSTERESIS) {
    g_hysteresisTolerance = MAX_HYSTERESIS;
  }
  
  saveHysteresisToEEPROM();
}

void decrementHysteresis() {
  g_hysteresisTolerance -= HYSTERESIS_STEP;
  
  if(g_hysteresisTolerance < MIN_HYSTERESIS) {
    g_hysteresisTolerance = MIN_HYSTERESIS;
  }
  
  saveHysteresisToEEPROM();
}

// Load settings from EEPROM, or initialize with defaults if EEPROM is uninitialized
void loadSettingsFromEEPROM() {
  byte magicValue = EEPROM.read(EEPROM_MAGIC_ADDR);
  
  if (magicValue == EEPROM_MAGIC_VALUE) {
    // EEPROM has been initialized, load saved settings
    g_currentSetTemp = EEPROM.read(EEPROM_SETTEMP_ADDR);
    EEPROM.get(EEPROM_HYSTERESIS_ADDR, g_hysteresisTolerance);
    
    // Validate loaded values and apply bounds
    if (g_currentSetTemp < MIN_TEMPERATURE) g_currentSetTemp = MIN_TEMPERATURE;
    if (g_currentSetTemp > MAX_TEMPERATURE) g_currentSetTemp = MAX_TEMPERATURE;
    if (g_hysteresisTolerance < MIN_HYSTERESIS) g_hysteresisTolerance = MIN_HYSTERESIS;
    if (g_hysteresisTolerance > MAX_HYSTERESIS) g_hysteresisTolerance = MAX_HYSTERESIS;
  } else {
    // First time initialization - use defaults
    g_currentSetTemp = 50;
    g_hysteresisTolerance = 1.0;
    
    // Write magic value and defaults to EEPROM
    EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
    EEPROM.write(EEPROM_SETTEMP_ADDR, g_currentSetTemp);
    EEPROM.put(EEPROM_HYSTERESIS_ADDR, g_hysteresisTolerance);
  }
}

// Save set temperature to EEPROM
void saveSetTempToEEPROM() {
  EEPROM.update(EEPROM_SETTEMP_ADDR, g_currentSetTemp);
}

// Save hysteresis tolerance to EEPROM
void saveHysteresisToEEPROM() {
  // Use EEPROM.put which handles multi-byte values like floats
  EEPROM.put(EEPROM_HYSTERESIS_ADDR, g_hysteresisTolerance);
}
