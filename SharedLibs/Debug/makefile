################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include SharedLibs/subdir.mk
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
all: libSharedLibs.so

dependents:
	-cd /home/utnso/git/so-commons-library/Debug && $(MAKE) all

# Tool invocations
libSharedLibs.so: $(OBJS) $(USER_OBJS) /home/utnso/git/so-commons-library/Debug/libcommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/git/so-commons-library/Debug" -shared -o "libSharedLibs.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(LIBRARIES) libSharedLibs.so
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/git/so-commons-library/Debug/libcommons.so:

-include ../makefile.targets
