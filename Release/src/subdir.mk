################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Blinkovi.cpp \
../src/GasSensor.cpp \
../src/UartMux.cpp \
../src/app.cpp 

CPP_DEPS += \
./src/Blinkovi.d \
./src/GasSensor.d \
./src/UartMux.d \
./src/app.d 

OBJS += \
./src/Blinkovi.o \
./src/GasSensor.o \
./src/UartMux.o \
./src/app.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/Blinkovi.d ./src/Blinkovi.o ./src/GasSensor.d ./src/GasSensor.o ./src/UartMux.d ./src/UartMux.o ./src/app.d ./src/app.o

.PHONY: clean-src

