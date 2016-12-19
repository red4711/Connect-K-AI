FLAGS = -std=c++11 -ggdb

bin_folder=./AI/bin
src_folder=./AI/src

src=$(src_folder)/ConnectK.cpp $(src_folder)/Move.cpp $(src_folder)/AIShell.cpp

all: myAI

myAI: $(src)
	g++ $(FLAGS) $(src) -o $(bin_folder)/AI
run: all
	java -jar ConnectK-Start.jar cpp:$(bin_folder)/AI
clean:
	rm -rf $(bin_folder)/*

