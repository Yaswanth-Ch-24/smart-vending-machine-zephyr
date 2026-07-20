cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(smart_vending_machine)

target_sources(app PRIVATE
    src/main.c
    src/keypad.c
    src/servo_dispense.c
    src/ultrasonic_verify.c
    src/wifi_inventory.c
    src/lcd_i2c.c
)

target_include_directories(app PRIVATE src)
