################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../NixieOne.c \
../booster.c \
../buttons.c \
../ds1307.c \
../ds18b20.c \
../i2c.c \
../nixie.c \
../onewire.c \
../photoresistor.c 

OBJS += \
./NixieOne.o \
./booster.o \
./buttons.o \
./ds1307.o \
./ds18b20.o \
./i2c.o \
./nixie.o \
./onewire.o \
./photoresistor.o 

C_DEPS += \
./NixieOne.d \
./booster.d \
./buttons.d \
./ds1307.d \
./ds18b20.d \
./i2c.d \
./nixie.d \
./onewire.d \
./photoresistor.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega8 -DF_CPU=4000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


