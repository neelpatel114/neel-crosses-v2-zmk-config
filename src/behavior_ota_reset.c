/*
 * Reboot into the Adafruit nRF52 bootloader's over-the-air DFU mode.
 *
 * ZMK's own &bootloader behavior goes through Zephyr's boot-mode retention, which can only
 * request the USB/UF2 mode (GPREGRET 0x57). The bootloader enters BLE DFU when GPREGRET
 * holds 0xA8 instead, so this behavior writes that register directly and resets.
 */
#define DT_DRV_COMPAT zmk_behavior_ota_reset

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>
#include <nrfx.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define ADAFRUIT_BOOTLOADER_OTA_MAGIC 0xA8

static int on_pressed(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    LOG_INF("rebooting into OTA DFU mode");
    NRF_POWER->GPREGRET = ADAFRUIT_BOOTLOADER_OTA_MAGIC;
    sys_reboot(SYS_REBOOT_COLD);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_released(struct zmk_behavior_binding *binding,
                       struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_ota_reset_driver_api = {
    .binding_pressed = on_pressed,
    .binding_released = on_released,
    .locality = BEHAVIOR_LOCALITY_EVENT_SOURCE,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define OTA_RESET_INST(n)                                                                      \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                            \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                \
                            &behavior_ota_reset_driver_api);

DT_INST_FOREACH_STATUS_OKAY(OTA_RESET_INST)
