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

#include "SHTC3.h"

SHTC3::SHTC3(int8_t address):
    _address(address) {
}

SHTC3::SHTC3(PinName sda, PinName scl, int8_t address, uint32_t frequency):
    _address(address) {
    _i2c = new (_i2c_obj) I2C(sda, scl);
    _i2c->frequency(frequency);
}

SHTC3::~SHTC3(void) {
    if (_i2c == reinterpret_cast<I2C *>(_i2c_obj)) {
        _i2c->~I2C();
    }
}

bool SHTC3::init(I2C *i2c_obj) {
    char data[3];

    if (i2c_obj != nullptr) {
        _i2c = i2c_obj;
    }

    MBED_ASSERT(_i2c);

    tr_debug("Init");

    // wakeup
    if (!sendCmd(CMD_WAKEUP)) {
        return false;
    }

    wait_us(240);

    // request ID
    if (!sendCmd(CMD_READ_ID)) {
        return false;
    }

    // read ID
    if (!getData(data, sizeof(data))) {
        tr_error("Read ID failed");
        return false;
    }

    // check CRC
    if (!checkCRC(data, sizeof(data))) {
        return false;
    }

    // check ID type
    if ((data[0] & 0b00001000) != 0b00001000 || (data[1] & 0b00111111) != 0b00000111) {
        tr_error("ID doesn't match");
        return false;
    }

    tr_info("SHTC3 ID: %04X", data[0] | data[1]);

    return sendCmd(CMD_SLEEP);
}

bool SHTC3::read(uint16_t *temp, uint16_t *humidity, bool low_power) {
    bool ret = false;
    uint8_t i = 0;
    char data[6];

    // wakeup
    if (!sendCmd(CMD_WAKEUP)) {
        return false;
    }

    wait_us(240);

    // request measurement
    shtc3_cmd_t cmd = (low_power ? CMD_MEASUREMENT_LOW_POWER : CMD_MEASUREMENT_NORMAL);

    if (!sendCmd(cmd)) {
        goto END;
    }

    if (!low_power) {
        ThisThread::sleep_for(10ms); // "normal" measuring time is 10-12ms
    }

    // read results
    for (; i < MBED_CONF_SHTC3_TIMEOUT; i++) { // safety timeout
        if (getData(data, sizeof(data))) {
            break;

        } else {
            ThisThread::sleep_for(1ms);
        }
    }

    if (i == MBED_CONF_SHTC3_TIMEOUT) {
        tr_error("Timeout");
        goto END;
    }

    tr_debug("Measuring took %ums", i);

    // check CRC
    if (!checkCRC(data, sizeof(data))) {
        goto END;
    }

    // if we reached up to here, it's a success
    ret = true;

    if (humidity) {
        *humidity = (data[0] << 8) | data[1];
    }

    if (temp) {
        *temp = (data[3] << 8) | data[4];
    }

END:
    // sleep now
    sendCmd(CMD_SLEEP);
    return ret;
}

float SHTC3::toCelsius(uint16_t raw) {
    return 175.0 * ((float)raw / 65535.0) - 45.0;
}

float SHTC3::toFahrenheit(uint16_t raw) {
    return toCelsius(raw) * 9.0 / 5.0 + 32.0;
}

float SHTC3::toPercentage(uint16_t raw) {
    return 100.0 * ((float)raw / 65535.0);
}

void SHTC3::reset() {
    sendCmd(CMD_RESET);
    wait_us(240);
}

bool SHTC3::sendCmd(shtc3_cmd_t cmd) {
    tr_debug("Sending CMD: %04X", (uint16_t)cmd);
    char data[2];
    data[0] = (uint16_t)cmd >> 8;
    data[1] = (uint16_t)cmd & UCHAR_MAX;

    int ack = -1;

    _i2c->lock();
    ack = _i2c->write(_address, data, sizeof(data));
    _i2c->unlock();

    if (ack != 0) {
        tr_error("Write failed");
        return false;
    }

    return true;
}

bool SHTC3::getData(char *data, size_t len) {
    int ack = -1;

    _i2c->lock();
    ack = _i2c->read(_address, data, len);
    _i2c->unlock();

    if (ack != 0) {
        return false;
    }

    tr_debug("Read data[%u]: %s", len, tr_array(reinterpret_cast<uint8_t *>(data), len));

    return true;
}

bool SHTC3::checkCRC(const char *data, size_t len) {
    //MBED_STATIC_ASSERT(len % 3, "Data len must be divideable by 3");
    MbedCRC<0x31, 8> checksum(0xFF, 0x00, false, false);
    uint32_t crc = 0;

    for (size_t i = 0; i < len; i = i + 3) {
        checksum.compute(data + i, 2, &crc);

        if (data[i + 2] != crc) {
            tr_error("CRC failed");
            return false;
        }
    }

    tr_debug("Checksum OK");
    return true;
}
