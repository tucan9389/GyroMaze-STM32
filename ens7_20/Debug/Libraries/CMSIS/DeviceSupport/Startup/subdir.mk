################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Libraries/CMSIS/DeviceSupport/Startup/startup_stm32f10x_cl.s 

OBJS += \
./Libraries/CMSIS/DeviceSupport/Startup/startup_stm32f10x_cl.o 


# Each subdirectory must supply rules for building sources it contributes
Libraries/CMSIS/DeviceSupport/Startup/%.o: ../Libraries/CMSIS/DeviceSupport/Startup/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Assembler 5'
	armasm --cpu=cortex-m3 -g -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


