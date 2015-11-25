################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Swap.c \
../src/funcSwap.c 

OBJS += \
./src/Swap.o \
./src/funcSwap.o 

C_DEPS += \
./src/Swap.d \
./src/funcSwap.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/so-commons-library" -I"/home/utnso/git/tp-2015-2c-game-of-pointers/SharedLibs" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


