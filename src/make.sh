rm source
rm debug

g++ main.cpp Instructions.cpp -lSDL2 -lSDL2_ttf -lSDL2_image -lboost_program_options -g -o main
nasm -f bin -o source tester.asm
nasm -f bin -o handle_cpu_fault handle_invalid_opcode.asm
sleep 1
sh run.sh
