################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Startup/subdir.mk
-include Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
ce2812_wk03_lab.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
ce2812_wk03_lab.list \


# All Target
all: ce2812_wk03_lab.elf secondary-outputs

# Tool invocations
ce2812_wk03_lab.elf: $(OBJS) $(USER_OBJS) C:\Users\wilkg\Documents\MSOE\2019_2020\q2_winter\ce2812_embedded_systems_2\ce2812_wk03_lab\STM32F446RETX_FLASH.ld
	arm-none-eabi-gcc -o "ce2812_wk03_lab.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\wilkg\Documents\MSOE\2019_2020\q2_winter\ce2812_embedded_systems_2\ce2812_wk03_lab\STM32F446RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="ce2812_wk03_lab.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

ce2812_wk03_lab.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "ce2812_wk03_lab.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
