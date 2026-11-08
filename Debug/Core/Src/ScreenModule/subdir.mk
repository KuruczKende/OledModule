################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ScreenModule/DispItf.c \
../Core/Src/ScreenModule/IoExpander.c \
../Core/Src/ScreenModule/SH1106.c \
../Core/Src/ScreenModule/Screens.c \
../Core/Src/ScreenModule/fonts.c 

OBJS += \
./Core/Src/ScreenModule/DispItf.o \
./Core/Src/ScreenModule/IoExpander.o \
./Core/Src/ScreenModule/SH1106.o \
./Core/Src/ScreenModule/Screens.o \
./Core/Src/ScreenModule/fonts.o 

C_DEPS += \
./Core/Src/ScreenModule/DispItf.d \
./Core/Src/ScreenModule/IoExpander.d \
./Core/Src/ScreenModule/SH1106.d \
./Core/Src/ScreenModule/Screens.d \
./Core/Src/ScreenModule/fonts.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/ScreenModule/%.o Core/Src/ScreenModule/%.su Core/Src/ScreenModule/%.cyclo: ../Core/Src/ScreenModule/%.c Core/Src/ScreenModule/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-ScreenModule

clean-Core-2f-Src-2f-ScreenModule:
	-$(RM) ./Core/Src/ScreenModule/DispItf.cyclo ./Core/Src/ScreenModule/DispItf.d ./Core/Src/ScreenModule/DispItf.o ./Core/Src/ScreenModule/DispItf.su ./Core/Src/ScreenModule/IoExpander.cyclo ./Core/Src/ScreenModule/IoExpander.d ./Core/Src/ScreenModule/IoExpander.o ./Core/Src/ScreenModule/IoExpander.su ./Core/Src/ScreenModule/SH1106.cyclo ./Core/Src/ScreenModule/SH1106.d ./Core/Src/ScreenModule/SH1106.o ./Core/Src/ScreenModule/SH1106.su ./Core/Src/ScreenModule/Screens.cyclo ./Core/Src/ScreenModule/Screens.d ./Core/Src/ScreenModule/Screens.o ./Core/Src/ScreenModule/Screens.su ./Core/Src/ScreenModule/fonts.cyclo ./Core/Src/ScreenModule/fonts.d ./Core/Src/ScreenModule/fonts.o ./Core/Src/ScreenModule/fonts.su

.PHONY: clean-Core-2f-Src-2f-ScreenModule

