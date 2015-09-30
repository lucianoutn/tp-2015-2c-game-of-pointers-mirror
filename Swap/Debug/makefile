################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Swap

dependents:
	-cd /home/utnso/workspace/tp-2015-2c-game-of-pointers/SharedLibs/Debug && $(MAKE) all
	-cd /home/utnso/workspace/tp-2015-2c-game-of-pointers/commons/Debug && $(MAKE) all

# Tool invocations
Swap: $(OBJS) $(USER_OBJS) /home/utnso/workspace/tp-2015-2c-game-of-pointers/SharedLibs/Debug/libSharedLibs.so /home/utnso/workspace/tp-2015-2c-game-of-pointers/commons/Debug/libcommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspace/tp-2015-2c-game-of-pointers/commons/Debug" -L"/home/utnso/workspace/tp-2015-2c-game-of-pointers/SharedLibs/Debug" -o "Swap" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Swap
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/workspace/tp-2015-2c-game-of-pointers/SharedLibs/Debug/libSharedLibs.so:
/home/utnso/workspace/tp-2015-2c-game-of-pointers/commons/Debug/libcommons.so:

-include ../makefile.targets
