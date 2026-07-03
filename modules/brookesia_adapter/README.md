# Brookesia Adapter

This module is the project boundary for ESP-Brookesia integration.

The existing board bring-up stays where it already works:

- LCD/backlight drivers remain in `drivers/`
- Board pins and configuration remain in `configs/`
- LCD/touch interfaces remain in `interfaces/`
- LVGL tasking, locking, input and display flush remain in `frameworks/bsp_lvgl_framework`

`brookesia_adapter_start()` is called by `main` after hardware, LVGL, display and
touch are initialized. If the adapter is disabled, it returns `false` and the
existing UI continues to run unchanged.

Next integration steps:

1. Add the required Brookesia registry components in this module's dependency
   boundary, not directly in `main`.
2. Map the existing BSP interfaces to Brookesia HAL/display/touch abstractions.
3. Replace the local LVGL shell with the real Brookesia app/system entry point.
4. Add services one by one, starting with storage or Wi-Fi before audio/AI.
