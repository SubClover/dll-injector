@echo off
gcc -shared -o mydll.dll mydll.c -Wall
gcc -o injector.exe injector.c -Wall
