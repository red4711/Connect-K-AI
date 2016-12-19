team_name = CodingCatsAI
FLAGS = -std=c++11 -ggdb

bin_folder=./Chu_10473576_CodingCatsAI/bin
src_folder=./Chu_10473576_CodingCatsAI/src

src=$(src_folder)/ConnectK.cpp $(src_folder)/Move.cpp $(src_folder)/AIShell.cpp

all: myAI

myAI: $(src)
	g++ $(FLAGS) $(src) -o $(bin_folder)/$(team_name)

test: all
	java -jar ConnectK-Start.jar cpp:$(bin_folder)/$(team_name)

testPoor: all
	java -jar ConnectK-Start.jar cpp:$(bin_folder)/$(team_name) ./ConnectKSource_java/SampleAI/PoorAI/PoorAI.class

testGood: all
	java -jar ConnectK-Start.jar cpp:$(bin_folder)/$(team_name) ./ConnectKSource_java/SampleAI/GoodAI/GoodAI.class

testAverage: all
	java -jar ConnectK-Start.jar cpp:$(bin_folder)/$(team_name) ./ConnectKSource_java/SampleAI/AverageAI/AverageAI.class
clean:
	rm -rf $(bin_folder)/*

