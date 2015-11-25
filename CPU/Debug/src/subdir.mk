################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/CPU.c \
../src/funcCPU.c \
../src/mAnsisOp.c 

OBJS += \
./src/CPU.o \
./src/funcCPU.o \
./src/mAnsisOp.o 

C_DEPS += \
./src/CPU.d \
./src/funcCPU.d \
./src/mAnsisOp.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/so-commons-library" -I"/home/utnso/git/tp-2015-2c-game-of-pointers/SharedLibs" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


