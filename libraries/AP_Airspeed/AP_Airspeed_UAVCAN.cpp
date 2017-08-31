#include <AP_HAL/AP_HAL.h>

#if HAL_WITH_UAVCAN

#include "AP_Airspeed_UAVCAN.h"
#include <AP_BoardConfig/AP_BoardConfig_CAN.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

extern const AP_HAL::HAL& hal;

#define debug_airspeed_uavcan(level, fmt, args...) do { if ((level) <= AP_BoardConfig_CAN::get_can_debug()) { hal.console->printf(fmt, ##args); }} while (0)

// There is limitation to use only one UAVCAN airspeed now.

/*
  constructor - registers instance at top Airspeed driver
 */
AP_Airspeed_UAVCAN::AP_Airspeed_UAVCAN(AP_Airspeed &airspeed) :
    AP_Airspeed_Backend(airspeed)
{
}

AP_Airspeed_UAVCAN::~AP_Airspeed_UAVCAN()
{
    if (hal.can_mgr[_manager] != nullptr) {
        AP_UAVCAN *ap_uavcan = hal.can_mgr[_manager]->get_UAVCAN();
        if (ap_uavcan != nullptr) {
            ap_uavcan->remove_airspeed_listener(this);
            debug_airspeed_uavcan(2, "AP_Airspeed_UAVCAN destructed\n\r");
        }
    }
}

bool AP_Airspeed_UAVCAN::init()
{
    if (hal.can_mgr[_manager] != nullptr) {
        AP_UAVCAN *ap_uavcan = hal.can_mgr[_manager]->get_UAVCAN();
        if (ap_uavcan != nullptr) {
            // Give time to receive some packets from CAN if airspeed sensor is present
            // This way it will get calibrated correctly
            hal.scheduler->delay(1000);
            ap_uavcan->register_airspeed_listener(this, 1);

            debug_airspeed_uavcan(2, "AP_Airspeed_UAVCAN loaded\n\r");
        } else {
            return false;
        }
    } else {
        return false;
    }

    _sem_airspeed = hal.util->new_semaphore();
    return true;
}

// return the current differential_pressure in Pascal
bool AP_Airspeed_UAVCAN::get_differential_pressure(float &_pressure)
{
    if ((AP_HAL::millis() - last_sample_time_ms) > 100) {
        return false;
    }
    if (sem->take(HAL_SEMAPHORE_BLOCK_FOREVER)) {
        if (press_count > 0) {
            pressure = pressure_sum / press_count;
            press_count = 0;
            pressure_sum = 0;
        }
        sem->give();
    }
    _pressure = pressure;
    return true;
}

// return the current temperature in degrees C, if available
bool AP_Airspeed_UAVCAN::get_temperature(float &_temperature)
{
    if ((AP_HAL::millis() - last_sample_time_ms) > 100) {
        return false;
    }
    if (sem->take(HAL_SEMAPHORE_BLOCK_FOREVER)) {
        if (temp_count > 0) {
            temperature = temperature_sum / temp_count;
            temp_count = 0;
            temperature_sum = 0;
        }
        sem->give();
    }
    _temperature = temperature;
    return true;
}

void AP_Airspeed_UAVCAN::handle_airspeed_msg(float _pressure, float _temperature)
{

    if (_sem_airspeed->take(HAL_SEMAPHORE_BLOCK_FOREVER)) {
        pressure_sum += _pressure;
        press_count++;
        temperature_sum += _temperature - 273.15f;
        temp_count++;
        last_sample_time_ms = AP_HAL::millis();
        _sem_airspeed->give();
    }
}

#endif // HAL_WITH_UAVCAN

