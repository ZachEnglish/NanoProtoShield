#ifndef DallasTemperatureWithPullup
#define DallasTemperatureWithPullup

#include "OneWireWithPullup.h"
#include <DallasTemperature.h>

class DallasTemperatureWithPullup : public DallasTemperature {
    
	DallasTemperatureWithPullup(OneWireWithPullup* _oneWire) {
        DallasTemperature::DallasTemperature(_oneWire);
    }
}

#endif