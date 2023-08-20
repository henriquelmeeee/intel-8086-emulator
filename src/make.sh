rm source
rm debug

g++ main.cpp Base.cpp Instructions.cpp -lSDL2 `sdl2-config --cflags --libs`  -lSDL2_ttf  -lboost_program_options -g -o main
nasm -f bin -o source tester.asm
nasm -f bin -o handle_cpu_fault handle_invalid_opcode.asm
sleep 1
sh run.sh
