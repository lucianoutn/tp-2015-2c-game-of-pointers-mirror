-include ../makefile.init

-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk
RM := rm -rf

all:
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/Swap/Debug && $(MAKE) make all
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/Memoria/Debug && $(MAKE) make all
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/Planificador/Debug && $(MAKE) make all
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/CPU/Debug && $(MAKE) make all

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/git/tp-2015-2c-game-of-pointers/SharedLibs/Debug/libSharedLibs.so:
/home/utnso/git/tp-2015-2c-game-of-pointers/commons/Debug/libcommons.so:

-include ../makefile.targets
