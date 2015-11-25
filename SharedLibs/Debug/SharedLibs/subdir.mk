################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SharedLibs/comun.c \
../SharedLibs/libreriaCliente.c \
../SharedLibs/libreriaServidor.c \
../SharedLibs/manejoListas.c 

OBJS += \
./SharedLibs/comun.o \
./SharedLibs/libreriaCliente.o \
./SharedLibs/libreriaServidor.o \
./SharedLibs/manejoListas.o 

C_DEPS += \
./SharedLibs/comun.d \
./SharedLibs/libreriaCliente.d \
./SharedLibs/libreriaServidor.d \
./SharedLibs/manejoListas.d 


# Each subdirectory must supply rules for building sources it contributes
SharedLibs/%.o: ../SharedLibs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/so-commons-library" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


