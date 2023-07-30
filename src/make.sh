rm source
rm debug

g++ -g -o main main.cpp
nasm -f bin -o source tester.asm
