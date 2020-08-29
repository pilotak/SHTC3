/*
MIT License
Copyright (c) 2020 Pavel Slama
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef SHTC3_H
#define SHTC3_H

#include <chrono>
#include "mbed.h"
using namespace std::chrono;

#include "mbed-trace/mbed_trace.h"
#ifndef TRACE_GROUP
    #define TRACE_GROUP  "SHTC"
#endif

#define SHTC3_ADDRESS (0x70<<1)

class SHTC3 {
  public:
    SHTC3();
    SHTC3(PinName sda, PinName scl, uint32_t frequency = 400000);
    ~SHTC3(void);

    /**
     * @brief Initialize communication with sensor
     *
     * @param i2c_obj pass I2C object if you didn't specify pins in constructor
     * @return true if success; false otherwise
     */
    bool init(I2C *i2c_obj = nullptr);

    /**
     * @brief Read sensor values
     *
     * @param temp reference where to copy the result
     * @param humidity reference where to copy the result
     * @param low_power whether the sensor should work in low power mode
     * @return true if success; false otherwise
     */
    bool read(uint16_t &temp, uint16_t &humidity, bool low_power = false);

    /**
     * @brief Software reset
     *
     */
    void reset();

    /**
     * @brief Convert the result to human readable format
     *
     * @param raw result from .read()
     * @return float temperature in celsius
     */
    float toCelsius(uint16_t raw);

    /**
     * @brief Convert the result to human readable format
     *
     * @param raw result from .read()
     * @return float temperature in fahrenheit
     */
    float toFahrenheit(uint16_t raw);

    /**
     * @brief Convert the result to human readable format
     *
     * @param raw result from .read()
     * @return float relative humidity in percentage
     */
    float toPercentage(uint16_t raw);

  private:
    typedef enum {
        CMD_WAKEUP = 0x3517,
        CMD_RESET = 0x805D,
        CMD_SLEEP = 0xB098,
        CMD_READ_ID = 0xEFC8,
        CMD_MEASUREMENT_NORMAL = 0x58E0,
        CMD_MEASUREMENT_LOW_POWER = 0x401A,
    } shtc3_cmd_t;

    /**
     * @brief Send command to sensor
     *
     * @param cmd
     * @return true if ACK; false otherwise (NOACK)
     */
    bool sendCmd(shtc3_cmd_t cmd);

    /**
     * @brief Check CRC of data packet
     *
     * @param data first two bytes are data, third is computed checksum by the sensor
     * @param len lenght is data buffer
     * @return true if success; false otherwise
     */
    bool checkCRC(const char *data, size_t len);

    /**
     * @brief Get data from sensor
     *
     * @param data pointer where to store the result
     * @param len how many bytes to read
     * @return true if ACK; false otherwise (NOACK)
     */
    bool getData(char *data, size_t len);

    I2C *_i2c;
    uint32_t _i2c_obj[sizeof(I2C) / sizeof(uint32_t)] = {0};
};

#endif  // SHTC3_H