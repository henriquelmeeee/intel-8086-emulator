sudo afl-g++ Base.cpp Instructions.cpp DebugScreen.cpp -lSDL2 `sdl2-config --cflags --libs` -lSDL2_ttf -lboost_program_options -lX11 -lGL -lpthread -o main -g -lstdc++fs -lpng
sudo afl-fuzz -i ./AFL/input -o ./AFL/output -- ./main --master @@
