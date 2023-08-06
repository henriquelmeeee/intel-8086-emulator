rm source
rm debug

g++ -g -o main main.cpp Instructions.cpp
nasm -f bin -o source tester.asm
sleep 1
sh run.sh
