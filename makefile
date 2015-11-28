-include ../makefile.init

-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk
RM := rm -rf

all:
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/SharedLibs/Debug && $(MAKE) clean
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/SharedLibs/Debug && $(MAKE) all
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/Swap/Debug && $(MAKE) clean
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/Memoria/Debug && $(MAKE) clean
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/Planificador/Debug && $(MAKE) clean
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/CPU/Debug && $(MAKE) clean
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/Swap/Debug && $(MAKE) all
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/Memoria/Debug && $(MAKE) all
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/Planificador/Debug && $(MAKE) all
	-cd /home/utnso/git/tp-2015-2c-game-of-pointers/CPU/Debug && $(MAKE) all

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/git/tp-2015-2c-game-of-pointers/SharedLibs/Debug/libSharedLibs.so:
/home/utnso/git/tp-2015-2c-game-of-pointers/commons/Debug/libcommons.so:

-include ../makefile.targets
