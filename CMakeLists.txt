cmake_minimum_required(VERSION 3.20.0)

set(BOARD nucleo_f446re)

find_package(Zephyr REQUIRED)

project(blinky_rtos LANGUAGES C)
target_sources(app PRIVATE src/tx_rx.c
                   PRIVATE src/blink.c
                   PRIVATE src/common.c)