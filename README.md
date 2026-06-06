# bthomev2-zephyr-module
A Zephyr out-of-tree module for building BTHome V2 BLE advertising payloads.

## Features
- Platform-independent payload builder API (`struct bt_data` output)
- Feature toggles via Kconfig (`CONFIG_BTHOME_*`)
- Zephyr module integration via `zephyr/module.yml`

## Layout
- `/CMakeLists.txt`
- `/zephyr/module.yml`
- `/zephyr/Kconfig`
- `/include/bthome/bthome.h`
- `/src/bthome.c`
