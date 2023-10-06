cd src
rm source
rm debug

g++ main.cpp Base.cpp Instructions.cpp Video/MainVideo.cpp DebugScreen.cpp -lSDL2 `sdl2-config --cflags --libs`  -lSDL2_ttf  -lboost_program_options -lX11 -lGL -lpthread -o main -g -lstdc++fs -lpng
nasm -f bin -o source tester.asm
nasm -f bin -o handle_cpu_fault handle_invalid_opcode.asm
sh run.sh
cd ..
