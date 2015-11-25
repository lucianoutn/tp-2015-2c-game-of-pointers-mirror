################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Planificador.c \
../src/consola.c \
../src/libreriaPlanificador.c 

OBJS += \
./src/Planificador.o \
./src/consola.o \
./src/libreriaPlanificador.o 

C_DEPS += \
./src/Planificador.d \
./src/consola.d \
./src/libreriaPlanificador.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/so-commons-library" -I"/home/utnso/git/tp-2015-2c-game-of-pointers/SharedLibs" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


