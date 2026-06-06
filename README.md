# bthomev2-zephyr-module

An out-of-tree Zephyr module for building BTHome V2 BLE advertising payloads
([specification](https://bthome.io/format/)).

## Features

- Platform-independent encoder: no nRF52 or ESP32 peripheral register accesses
- Queue-based design: accumulate measurements → sort → encode → advertise
- Automatic object-ID sorting (required by the BTHome V2 spec)
- Full set of object-ID constants and type-safe add helpers
- `struct bt_data` output for direct use with `bt_le_adv_start()`
- All features controlled via `CONFIG_BTHOME_*` Kconfig symbols
- `trigger_based` device flag support (bit 2 of device-info byte)
- Optional AES-CCM encryption flag + crypto backend selection via Kconfig

## Module layout

```
CMakeLists.txt          # Zephyr module CMake entry point
zephyr/
  module.yml            # West module manifest
  Kconfig               # CONFIG_BTHOME_* options
include/
  bthome/
    bthome.h            # Public API and object-ID constants
src/
  bthome.c              # Encoder implementation
```

## Quick start

### 1. Register the module in your west workspace

Add to your `west.yml`:

```yaml
manifest:
  projects:
    - name: bthomev2-zephyr-module
      url: https://github.com/the78mole/bthomev2-zephyr-module
      path: modules/bthome
```

### 2. Enable the module in `prj.conf`

```conf
CONFIG_BT=y
CONFIG_BT_BROADCASTER=y
CONFIG_BTHOME=y
```

### 3. Use the API

```c
#include <bthome/bthome.h>

static struct bthome_ctx bthome;
static struct bt_data ad[2];

static const struct bt_le_adv_param adv_param =
    BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_USE_IDENTITY,
                         BT_GAP_ADV_SLOW_INT_MIN,
                         BT_GAP_ADV_SLOW_INT_MAX,
                         NULL);

void advertise(void)
{
    bthome_init(&bthome, false, false);
    bthome_add_temperature(&bthome, 2350);   /* 23.50 °C */
    bthome_add_humidity(&bthome, 5500);       /* 55.00 %  */
    bthome_encode(&bthome);

    ad[0] = (struct bt_data)BT_DATA_BYTES(BT_DATA_FLAGS,
                 BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR);
    bthome_get_bt_data(&bthome, &ad[1]);

    bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
}
```

## Unit conventions

| Sensor       | API unit     | Wire factor | Example                  |
|-------------|--------------|-------------|--------------------------|
| temperature | 0.01 °C      | × 0.01      | `2350` → 23.50 °C        |
| humidity    | 0.01 %       | × 0.01      | `5500` → 55.00 %         |
| pressure    | 0.01 hPa     | × 0.01      | `101325` → 1013.25 hPa   |
| illuminance | 0.01 lx      | × 0.01      | `50000` → 500.00 lx      |
| voltage     | 0.001 V      | × 0.001     | `3300` → 3.300 V         |
| acceleration| 0.001 m/s²   | × 0.001     | `9810` → 9.810 m/s²      |
| gyroscope   | 0.001 °/s    | × 0.001     | `1000` → 1.000 °/s       |

## Kconfig reference

| Symbol                          | Description                         |
|---------------------------------|-------------------------------------|
| `CONFIG_BTHOME`                 | Enable the module                   |
| `CONFIG_BTHOME_ENCRYPTION`      | Enable AES-CCM encryption flag      |
| `CONFIG_BTHOME_CRYPTO_BACKEND_MBEDTLS` | Use Mbed TLS backend         |
| `CONFIG_BTHOME_CRYPTO_BACKEND_TINYCRYPT` | Use TinyCrypt backend      |
| `CONFIG_BTHOME_LOG_LEVEL`       | Log level for the bthome module     |
