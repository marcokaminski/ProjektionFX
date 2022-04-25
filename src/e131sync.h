#if !defined(__PROJECTIONFX_E131SYNC_H__)
#define __PROJECTIONFX_E131SYNC_H__

#include "e131/E131.h"
#include <FastLED.h>
#include "settings.h"
E131 e131;
class E131Sync
{
private:
    uint16_t universeTX = 1;
    uint8_t DMXsequence = 78;

public:
    void setup()
    {
    }
    void loop()
    {
        if (DMXsequence > 0xFF)
            DMXsequence = 1;
        e131.setSequenceNumber(DMXsequence++);
        e131.setSourceName("ProjektioNFX");

        for(int i = 0; i < NUM_LEDS; i++){
            CRGB led = leds[i];
            e131.setRGB(i * 3, led.r, led.g, led.b); // start, R, G, B
        }
        e131.sendPacket(universeTX);
    }
};

static E131Sync e131sync;

#endif // __PROJECTIONFX_E131SYNC_H__