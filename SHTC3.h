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

    bool init(I2C *i2c_obj = nullptr);
    bool read(uint16_t &temp, uint16_t &humidity, bool low_power = false);
    void reset();
    float toCelsius(uint16_t raw);
    float toFahrenheit(uint16_t raw);
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

    bool sendCmd(shtc3_cmd_t cmd);
    bool checkCRC(const char *data, size_t len);
    bool getData(char *data, size_t len);

    I2C *_i2c;
    uint32_t _i2c_obj[sizeof(I2C) / sizeof(uint32_t)] = {0};
};

#endif  // SHTC3_H