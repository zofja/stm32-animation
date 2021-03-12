CC       = arm-eabi-gcc
OBJCOPY  = arm-eabi-objcopy
FLAGS    = -mthumb -mcpu=cortex-m4
CPPFLAGS = -DSTM32F411xE
CFLAGS   = $(FLAGS) -Wall -g \
					-O0 -ffunction-sections -fdata-sections \
					-I/opt/arm/stm32/inc \
					-I/opt/arm/stm32/CMSIS/Include \
					-I/opt/arm/stm32/CMSIS/Device/ST/STM32F4xx/Include \
					-I$(CURDIR)/src
LDFLAGS = $(FLAGS) -Wl,--gc-sections -nostartfiles \
				   -L/opt/arm/stm32/lds -Tstm32f411re.lds

vpath %.c /opt/arm/stm32/src
vpath %.c src
vpath %.c src/spi
vpath %.c src/animation
vpath %.c src/timer
vpath %.c src/uart

OBJECTS = startup_stm32.o gpio.o delay.o \
main.o spi_config.o spi_write.o tim_config.o screen.o encoder.o frames.o
TARGET = zadanie3

.SECONDARY: $(TARGET).elf $(OBJECTS)

all: $(TARGET).bin

%.elf : $(OBJECTS)
		$(CC) $(LDFLAGS) $^ -o $@

%.bin : %.elf
		$(OBJCOPY) $< $@ -O binary

clean :
		rm -f *.bin *.elf *.hex *.d *.o *.bak *~