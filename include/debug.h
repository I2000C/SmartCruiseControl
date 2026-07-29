#pragma once
#include <Arduino.h>

static char buffer[512];

class Debug {
    public:
        static void setEnabled(bool enabled) {
            _enabled = enabled;
        }

        static bool isEnabled() {
            return _enabled;
        }

        static void print(const char *msg) {
            if(_enabled) Serial.print(msg);
        }

        static void println(const char *msg) {
            if(_enabled) Serial.println(msg);
        }

        static void printf(const char* format, ...) {
            if(!_enabled) {
                return;
            }

            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);

            Serial.print(buffer);
        }

    private:
        static inline bool _enabled = false;
};
