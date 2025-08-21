#pragma once
#include <Arduino.h>
// Remplace les appels Debug("...") de Waveshare
#define Debug(...) do { Serial.printf(__VA_ARGS__); } while(0)