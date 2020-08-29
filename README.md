# SHTC3
[![Framework Badge mbed](https://img.shields.io/badge/framework-mbed-008fbe.svg)](https://os.mbed.com/)

Mbed library for SHTC3 humidity &amp; temperature sensor

## Example
```cpp
#include "mbed.h"
#include "SHTC3.h"

SHTC3 sht(PB_9, PB_8);

int main() {
    if (!sht.init()) {
        printf("Init failed\n");
        return 0;
    }

    while (1) {
        uint16_t raw_temp, raw_humidity;

        if (sht.read(raw_temp, raw_humidity)) {
            printf("Temperature: %f*C (%u), humidity: %f%% (%u)\n", sht.toCelsius(raw_temp), raw_temp,
                   sht.toPercentage(raw_humidity), raw_humidity);

        } else {
            printf("Read failed\n");
        }

        ThisThread::sleep_for(2s);
    }
}
```

## Example passing I2C object
```cpp
#include "mbed.h"
#include "SHTC3.h"

I2C i2c(PB_9, PB_8);
SHTC3 sht;

int main() {
    if (!sht.init(&i2c)) {
        printf("Init failed\n");
        return 0;
    }

    while (1) {
        uint16_t raw_temp, raw_humidity;

        if (sht.read(raw_temp, raw_humidity)) {
            printf("Temperature: %f*C (%u), humidity: %f%% (%u)\n", sht.toCelsius(raw_temp), raw_temp,
                   sht.toPercentage(raw_humidity), raw_humidity);

        } else {
            printf("Read failed\n");
        }

        ThisThread::sleep_for(2s);
    }
}
```