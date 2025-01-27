/**
 * @file AdaptiveValve.cpp
 * @author Tomasz Marcin Kowalski
 * @brief Functions definitions for controling adaptive valve
 * @version 1.0
 * @date 2022-12-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "AdaptiveValve.h"

/**
 * @brief Construct a new AdaptiveValve::AdaptiveValve object
 * @param ads_i2c_addr I2C addres of analog to digital conventer - refer to electrical schematic
 * @param ads_channel analog to digital conventer channel (0-3) - refer to electrical schematic
 * @param tle_cs_pin chip select pin for motor driver (TLE) - refer to electrical schematic
 * @param set_position // set default absolute position for adaptive valve [mm]
 * @param displacement_min // absolute lower position for adaptive valve [mm]
 * @param displacement_max // absolute higher position for adaptive valve [mm]
 */

AdaptiveValve::AdaptiveValve(u_int8_t ads_i2c_addr, uint8_t ads_channel,
                             uint8_t tle_cs_pin,
                             float set_position,
                             float displacement_min,
                             float displacement_max)
{
    _ads_i2c_addr = ads_i2c_addr;
    _ads_channel = ads_channel;
    _tle_cs_pin = tle_cs_pin;
    _displacement_min = displacement_min;
    _displacement_max = displacement_max;
    _set_position = set_position;
}

/**
 * @brief Initializes AdaptiveValve object by calling motor driver and 
 * analog to digital conventer to begin
 * @return true if adaptive valve initialize succesfuly, false otherwise
 */

bool AdaptiveValve::begin()
{
    _tle.begin(_tle_cs_pin);
    return _ads.begin(_ads_i2c_addr);
}

/**
 * @brief Decode remote controler command and map it lineary
 * into valve position beetwen min and max displacement range
 * @return Set position for adaptive valve [mm] 
 */

float AdaptiveValve::decode_position(int remote_settings)
{
    if (remote_settings != -1)
    {
        _remote_code = remote_settings;
    }
    
    return (_remote_code / (_remote_combinations - 1.0)) * (_displacement_max - _displacement_min) + _displacement_min;
}

/**
 * @brief Update set position for adaptive valve [mm] 
 */

void AdaptiveValve::set_position(float new_position)
{
    _set_position = new_position;
}

/**
 * @brief Get voltage reading from ADC then compute absolute angular and linear position 
 * @return Absolute valve position [mm] 
 */

float AdaptiveValve::get_position()
{
    u = _ads.computeVolts(_ads.readADC_SingleEnded(_ads_channel));
    ang_position = (u * _ang_position_max) / _u_max;
    displacement = (ang_position / 360.0) * 3.14159 * _measurement_gear_diameter;
    return displacement;
}

/**
 * @brief Simple controller for adaptive valve
 * @param displacement_tolerance Displacement tolerance for valve positioning [mm] 
 * @return If valve is within set range or not 
 */

bool AdaptiveValve::controller(float displacement_tolerance)
{
    // update dispcacement tolerance
    _displacement_tolerance = displacement_tolerance;
    // update position on each call
    get_position();
    // check if valve position is within tolerance
    if (abs(_set_position - displacement) >= _displacement_tolerance)
    {
        // control motor direction
        if (_set_position >= displacement)
        {
            // turn motor on in forward direction
            _tle.set_pwm_dir(1, 1);
        }
        else
        {
            // turn motor on in reverse direction
            _tle.set_pwm_dir(1, 0);
        }
        // update object variable
        in_position = false;
        // return value as well
        return false;
    }
    else
    {
        // turn off the motor
        _tle.set_pwm_dir(0, 0);
        // update object variable
        in_position = true;
        // return value as well
        return true;
    }
}

/**
 * @brief Print valve set and measured position.
 * Serial communication has to be enable!
 */

void AdaptiveValve::print_position()
{
    get_position();
    Serial.print("Set position: ");
    Serial.print(_set_position);
    Serial.print(" [mm] \t");
    Serial.print(" Measured position: ");
    Serial.print(displacement);
    Serial.print(" [mm] \n");
}
