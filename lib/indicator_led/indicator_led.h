#pragma once
#include "constants.h"
#include "state.h"

namespace IndicatorLed {
    void init();

    void setState(const SystemState& state);
}

