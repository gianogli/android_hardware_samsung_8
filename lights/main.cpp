#include <string.h>
#include <errno.h>
#include <cutils/log.h>
#include <hardware/hardware.h>
#include <hardware/lights.h>

#include <cstdlib>
#include <stdio.h>

static bool init;
static light_device_t *g_battery_dev;
static light_device_t *g_none_dev;

light_device_t* get_device(hw_module_t* module, char const* name)
{
    int err;
    hw_device_t* device;
    err = module->methods->open(module, name, &device);
    if (err == 0) {
        return (light_device_t*)device;
    } else {
        return NULL;
    }
}

void init_native(void)
{
    int err;
    hw_module_t* module;

    err = hw_get_module(LIGHTS_HARDWARE_MODULE_ID, (hw_module_t const**)&module);
    if (err) {
        ALOGE("%s: can't open lights module!", __func__);
        return;
    }

    g_battery_dev = get_device(module, LIGHT_ID_BATTERY);
    //g_none_dev = get_device(module, LIGHT_ID_ATTENTION);
    init = (g_battery_dev != NULL /*&& g_none_dev != NULL*/);
}

void setLight_native(int colorARGB, int flashMode, int onMS, int offMS, int brightnessMode,
        int brightnessLevel, int multipleLeds __unused)
{
    light_state_t state;
    int colorAlpha;

    if (!init) {
        ALOGE("%s: light device is not initialized!\n", __func__);
        return;
    }

    if (brightnessLevel > 0 && brightnessLevel <= 0xFF) {
        colorAlpha = (colorARGB & 0xFF000000) >> 24;
        if (colorAlpha == 0x00) {
            colorAlpha = 0xFF;
        }
        colorAlpha = (colorAlpha * brightnessLevel) / 0xFF;
        if (colorAlpha < 1) {
            colorAlpha = 1;
        }
        colorARGB = (colorAlpha << 24) + (colorARGB & 0x00FFFFFF);
    }

    memset(&state, 0, sizeof(light_state_t));

    state.flashMode = flashMode;
    state.flashOnMS = onMS;
    state.flashOffMS = offMS;
    state.color = colorARGB;
    state.brightnessMode = brightnessMode;
    //state.ledsModes = 0 |
    //                  (multipleLeds ? LIGHT_MODE_MULTIPLE_LEDS : 0);

     if (g_battery_dev == NULL) {
        printf("%s: BUG: g_battery_dev == NULL\n", __func__);
        return;
     }

     //if (brightnessLevel > 0)
     g_battery_dev->set_light(g_battery_dev, &state);
     //else
     //g_none_dev->set_light(g_none_dev, &state);
}

#ifndef BUILD_AS_LIBRARY

int usage()
{
    printf("%s: lightd <colorARGB> <onOff>\n", __func__);
    printf("%s: example: lightd ffffffff 1\n", __func__);
    return -EINVAL;
}

int parse_hex(char *hex)
{
    char *p;
    long n = strtol(hex, &p, 16);
    if (*p != 0) {
        printf("%s: failed to parse args!\n", __func__);
        return -EINVAL;
    }

    return n;
}

int main(int argc, char *argv[])
{
    int colorARGB;
    int flashMode;
    int onMS, offMS;
    int brightnessMode;
    int brightnessLevel;
    init = false;
    if (argc != 7)
        return usage();

    colorARGB = parse_hex(argv[1]);
    flashMode = atoi(argv[2]);
    onMS = atoi(argv[3]);
    offMS = atoi(argv[4]);
    brightnessMode = atoi(argv[5]);
    brightnessLevel = atoi(argv[6]);

    if (colorARGB < 0 || brightnessLevel < 0)
        return usage();

    init_native();
#if 0
    setLight_native(/*int colorARGB*/ 0xFF, /*int flashMode*/ LIGHT_FLASH_NONE, 0 /*int onMS*/, 0 /*int offMS*/, 0 /*int brightnessMode*/,
        0 /*int brightnessLevel*/, 0 /*int multipleLeds*/);
#endif

    setLight_native(colorARGB, LIGHT_FLASH_NONE /*flashMode*/ /*int flashMode*/ /*LIGHT_FLASH_NONE*/, onMS /*int onMS*/, offMS /*int offMS*/, brightnessMode /*int brightnessMode*/,
        brightnessLevel, 0 /*multipleLeds*/ /*int multipleLeds*/);

    return 0;
}
#endif
