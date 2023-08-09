rm source
rm debug

g++ main.cpp Instructions.cpp -lSDL2 -lSDL2_ttf -g -o main
nasm -f bin -o source tester.asm
sleep 1
sh run.sh
