################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ScreenModule/Screens/testScreen.c 

OBJS += \
./Core/Src/ScreenModule/Screens/testScreen.o 

C_DEPS += \
./Core/Src/ScreenModule/Screens/testScreen.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/ScreenModule/Screens/%.o Core/Src/ScreenModule/Screens/%.su Core/Src/ScreenModule/Screens/%.cyclo: ../Core/Src/ScreenModule/Screens/%.c Core/Src/ScreenModule/Screens/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-ScreenModule-2f-Screens

clean-Core-2f-Src-2f-ScreenModule-2f-Screens:
	-$(RM) ./Core/Src/ScreenModule/Screens/testScreen.cyclo ./Core/Src/ScreenModule/Screens/testScreen.d ./Core/Src/ScreenModule/Screens/testScreen.o ./Core/Src/ScreenModule/Screens/testScreen.su

.PHONY: clean-Core-2f-Src-2f-ScreenModule-2f-Screens

