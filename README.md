# Animation

## Hardware

- NUCLEO-F411RE chip

## File structure

- `animation` module:
    1. `encoder` handles encoder interrupts
    2. `frames` stores buffer with animation frames
    2. `screen` configures LCD screen
- `spi` module:
    1. `spi_config` configures SPI3
    2. `spi_write` provides `SPIwrite(...)` function to initialize SPI3 transfer
- `timer` module:
    1. `tim_config` configures TIM3 for encoder
