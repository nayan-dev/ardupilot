#pragma once

#if HAL_WITH_UAVCAN

#include "AP_Airspeed_Backend.h"
#include <AP_UAVCAN/AP_UAVCAN.h>

class AP_Airspeed_UAVCAN : public AP_Airspeed_Backend {
public:
    AP_Airspeed_UAVCAN(AP_Airspeed &);
    ~AP_Airspeed_UAVCAN() override;
    
    // probe and initialise the sensor
    bool init() override;

    // This method is called from UAVCAN thread
    void handle_airspeed_msg(float diff_pressure, float temperature) override;

    // return the current differential_pressure in Pascal
    bool get_differential_pressure(float &pressure) override;

    // return the current temperature in degrees C, if available
    bool get_temperature(float &temperature) override;

private:
    float pressure;
    float temperature;
    float temperature_sum;
    float pressure_sum;
    uint32_t temp_count;
    uint32_t press_count;
  
    uint32_t last_sample_time_ms;

    AP_HAL::Semaphore *_sem_airspeed;
};
#endif