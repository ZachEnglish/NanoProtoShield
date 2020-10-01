#ifndef ONE_WIRE_WITH_PULLUP
#define ONE_WIRE_WITH_PULLUP

#include <OneWire.h>
#include "util/OneWire_direct_gpio.h"

class OneWireWithPullup : public OneWire
{
    uint8_t OneWireWithPullup::reset(void)
    {
        IO_REG_TYPE mask IO_REG_MASK_ATTR = bitmask;
        volatile IO_REG_TYPE *reg IO_REG_BASE_ATTR = baseReg;
        uint8_t r;
        uint8_t retries = 125;

        noInterrupts();
        DIRECT_MODE_INPUT(reg, mask);
        interrupts();
        // wait until the wire is high... just in case
        do {
            if (--retries == 0) return 0;
            delayMicroseconds(2);
        } while ( !DIRECT_READ(reg, mask));

        noInterrupts();
        DIRECT_WRITE_LOW(reg, mask);
        DIRECT_MODE_OUTPUT(reg, mask);	// drive output low
        interrupts();
        delayMicroseconds(480);
        noInterrupts();
        DIRECT_MODE_INPUT(reg, mask);	// allow it to float
        DIRECT_WRITE_HIGH( reg , mask ); // enable pull-up resistor
        delayMicroseconds(70);
        r = !DIRECT_READ(reg, mask);
        interrupts();
        delayMicroseconds(410);
        return r;
    }

    uint8_t OneWireWithPullup::read_bit(void)
    {
        IO_REG_TYPE mask IO_REG_MASK_ATTR = bitmask;
        volatile IO_REG_TYPE *reg IO_REG_BASE_ATTR = baseReg;
        uint8_t r;

        noInterrupts();
        DIRECT_MODE_OUTPUT(reg, mask);
        DIRECT_WRITE_LOW(reg, mask);
        delayMicroseconds(3);
        DIRECT_MODE_INPUT(reg, mask);	// let pin float, pull up will raise
        DIRECT_WRITE_HIGH( reg , mask ); // enable pull-up resistor
        delayMicroseconds(10);
        r = DIRECT_READ(reg, mask);
        interrupts();
        delayMicroseconds(53);
        return r;
    }

    void OneWireWithPullup::write(uint8_t v, uint8_t power /* = 0 */)
    {
        uint8_t bitMask;

        for (bitMask = 0x01; bitMask; bitMask <<= 1) {
        OneWire::write_bit( (bitMask & v)?1:0);
        }
        if ( !power) {
        noInterrupts();
        DIRECT_MODE_INPUT(baseReg, bitmask);
        //DIRECT_WRITE_LOW(baseReg, bitmask);
        interrupts();
        }
    }

    void OneWireWithPullup::write_bytes(const uint8_t *buf, uint16_t count, bool power /* = 0 */)
    {
        for (uint16_t i = 0 ; i < count ; i++)
            write(buf[i]);
        if (!power) {
            noInterrupts();
            DIRECT_MODE_INPUT(baseReg, bitmask);
            //DIRECT_WRITE_LOW(baseReg, bitmask);
            interrupts();
        }
    }
}

#endif