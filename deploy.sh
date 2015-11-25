#! /bin/bash
# Script de Game of Pointers para Sistemas Operativos UTN

# Nos colocamos en el directorio de nuestro usuario
cd ~

# Agrego la linea al final del archivo de la consola
echo "# seteo el library path GAME OF POINTERS
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/git/tp-2015-2c-game-of-pointers/SharedLibs/Debug/" >> .bashrc

# Imprimimos por pantalla msje

echo "Library Path de GAME OF POINTERS agregada"


