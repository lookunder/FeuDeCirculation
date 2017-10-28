RM := rm -rf

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
FeuDeCirculation.lss \

FLASH_IMAGE += \
FeuDeCirculation.hex \

EEPROM_IMAGE += \
FeuDeCirculation.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: FeuDeCirculation.elf FeuDeCirculation.hex

install:
	avrdude -p ATTINY85 -c linuxspi -P /dev/spidev0.0 -C ../avrdude_gpio.conf -b 10000 -U flash:w:FeuDeCirculation.hex

# Tool invocations
FeuDeCirculation.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,FeuDeCirculation.map main.c -L/usr/lib/avr/lib -mmcu=attiny85 -o "FeuDeCirculation.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

FeuDeCirculation.lss: FeuDeCirculation.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S FeuDeCirculation.elf  >"FeuDeCirculation.lss"
	@echo 'Finished building: $@'
	@echo ' '

FeuDeCirculation.hex: FeuDeCirculation.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex FeuDeCirculation.elf  "FeuDeCirculation.hex"
	@echo 'Finished building: $@'
	@echo ' '

FeuDeCirculation.eep: FeuDeCirculation.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex FeuDeCirculation.elf  "FeuDeCirculation.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: FeuDeCirculation.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=ATTINY85 FeuDeCirculation.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(EEPROM_IMAGE)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS) FeuDeCirculation.elf FeuDeCirculation.hex FeuDeCirculation.map

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:
