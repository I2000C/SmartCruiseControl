#include "constants.h"
#include "elm327.h"
#include "elm327_pids.h"
#include <Arduino.h>

// Background task that reads serial commands and keeps vehicle state fresh
void elm327Task(void* args) {
    if(args == nullptr) {
        return;
    }
    Elm327* elm327 = (Elm327*) args;

    char buffer[64];
    uint8_t pos = 0;

    VehicleState vehicleState;
    uint32_t lastUpdateTime = 0;

    while(true) {
        while(Serial.available()) {
            uint32_t now = millis();
            if(now - lastUpdateTime > REFRESH_COMPUTED_DATA_TIME_MS) {
                elm327->canReader.readState(vehicleState);
                lastUpdateTime = now;
            }

            char c = Serial.read();
            if(c == '\r' || c == '\n') {
                buffer[pos] = '\0';
                if(pos > 0) {
                    elm327->processCommand(buffer, vehicleState);
                    elm327->sendPrompt();
                }
                pos = 0;
            } else if(pos < sizeof(buffer) - 1) {
                buffer[pos++] = toupper(c);
            }
        }
        delay(10);
    }
}

void Elm327::init() {
    // Initialize ELM327 emulator task
    xTaskCreatePinnedToCore(elm327Task, "Elm327Task", 5000, this, ELM327_TASK_PRIORITY, nullptr, APP_CORE_ID);
}

void Elm327::sendText(const char* text) {
    // Send a single line of text to the ELM327 serial port
    Serial.write(text);
    Serial.write('\r');
    if(context.linefeeds) {
        Serial.write('\n');
    }
}

void Elm327::sendPrompt() {
    // Send the ELM327 prompt character after a response
    sendText("");
    Serial.write('>');
}

void Elm327::sendResponse(const uint8_t* data, uint8_t len, const char* header) {
    // Format a response using configured header and spacing options
    char out[64];
    int pos = 0;

    if(context.headers) {
        pos += snprintf(out, sizeof(out), "%s", header);
    }

    for(int i=0; i<len; i++) {
        if(pos == 0 || !context.spaces) {
            pos += snprintf(out + pos, sizeof(out) - pos, "%02X", data[i]);
        } else {
            pos += snprintf(out + pos, sizeof(out) - pos, " %02X", data[i]);
        }
    }

    sendText(out);
}

void Elm327::processCommand(const char* cmd, const VehicleState& vehicleState) {
    // Route command to appropriate handler based on prefix
    if(cmd[0] == 'A' && cmd[1] == 'T') {
        processAT(cmd, vehicleState);
        return;
    }

    if(cmd[0] == '0' && cmd[1] == '1') {
        processMode01(cmd, vehicleState);
        return;
    }

    if(cmd[0] == '2' && cmd[1] == '2') {
        processMode22(cmd, vehicleState);
        return;
    }

    sendText("?");
}

void Elm327::processAT(const char* cmd, const VehicleState& vehicleState) {
    // Handle AT commands used by ELM327 clients
    if(!strncmp(cmd, "ATZ", 3)) {
        context.echo = false;
        context.headers = false;
        context.spaces = true;
        context.linefeeds = true;

        sendText("ELM327 v1.5");
        return;
    }

    if(!strncmp(cmd, "ATI", 3) || !strncmp(cmd, "ATDESC", 6)) {
        sendText("ELM327 v1.5");
        return;
    }

    if(!strncmp(cmd, "AT@1", 4)) {
        sendText("OBDII to RS232 Interpreter");
        return;
    }

    if(!strncmp(cmd, "ATDPN", 5)) {
        sendText("A6");
        return;
    }

    if(!strncmp(cmd, "ATRV", 4)) {
        char out[10];
        snprintf(out, sizeof(out), "%.2fV", vehicleState.batteryVoltage);
        sendText(out);
        return;
    }

    if(!strncmp(cmd, "ATE", 3)) {
        context.echo = (cmd[3] == '1');
        sendText("OK");
        return;
    }

    if(!strncmp(cmd, "ATH", 3)) {
        context.headers = (cmd[3] == '1');
        sendText("OK");
        return;
    }

    if(!strncmp(cmd, "ATS", 3)) {
        context.spaces = (cmd[3] == '1');
        sendText("OK");
        return;
    }

    if(!strncmp(cmd, "ATL", 3)) {
        context.linefeeds = (cmd[3] == '1');
        sendText("OK");
        return;
    }

    sendText("OK");
}

void Elm327::processMode01(const char* cmd, const VehicleState& vehicleState) {
    // Handle standard OBD-II request mode 01
    if(strnlen(cmd, 4) != 4) {
        sendText("?");
        return;
    }

    char* end;
    uint8_t pid = (uint8_t) strtol(cmd + 2, &end, 16);
    if(end == cmd + 2) {
        sendText("?");
        return;
    }

    if(pid == 0x00 || pid == 0x20 || pid == 0x40) {
        uint8_t out[6] = {0x41, pid};
        uint8_t* bitmap = &out[2];
        buildPidBitmap(pid, bitmap);
        sendResponse(out, sizeof(out));
        return;
    }

    PidResponse response;
    if(processPid(pid, response, vehicleState)) {
        uint8_t out[sizeof(response.data) + 2] = {0x41, pid};
        memcpy(&out[2], response.data, response.len);
        sendResponse(out, response.len + 2);
        return;
    }

    sendText("NO DATA");
}

void Elm327::processMode22(const char* cmd, const VehicleState& vehicleState) {
    // Handle extended OBD-II request mode 22
    if(strnlen(cmd, 6) != 6) {
        sendText("?");
        return;
    }

    char* end;
    uint8_t pid = (uint8_t) strtol(cmd + 2, &end, 16);
    if(end == cmd + 2) {
        sendText("?");
        return;
    }

    PidResponse response;
    if(processExtraPid(pid, response, vehicleState)) {
        uint8_t out[sizeof(response.data) + 3] = {0x62, 0x00, pid};
        memcpy(&out[3], response.data, response.len);
        sendResponse(out, response.len + 3);
        return;
    }

    sendText("NO DATA");
}
