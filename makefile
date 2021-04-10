TARGET = worker.exe
COMPILER = g++-10 -std=c++11 -Wall -g
OBJECTS = definition.o game.o json_value.o json_writer.o json_reader.o



target : $(TARGET)

parser: $(OBJECTS) parser.o
	$(COMPILER) -o parser $(OBJECTS) parser.o

definition.o : game/definition.cpp game/definition.h
	$(COMPILER) -c game/definition.cpp -o definition.o

game.o : game/game.cpp game/game.h definition.o
	$(COMPILER) -c game/game.cpp -o game.o

main.o : main.cpp
	$(COMPILER) -c main.cpp -o main.o

json_value.o : lib_json/json_value.cpp
	$(COMPILER) -c lib_json/json_value.cpp -o json_value.o

json_writer.o : lib_json/json_writer.cpp
	$(COMPILER) -c lib_json/json_writer.cpp -o json_writer.o

json_reader.o : lib_json/json_reader.cpp
	$(COMPILER) -c lib_json/json_reader.cpp -o json_reader.o

parser.o: parse/parser.cpp game.o
	$(COMPILER) -c parse/parser.cpp -o parser.o

$(TARGET) : $(OBJECTS) main.o
	$(COMPILER) -o $(TARGET) $(OBJECTS) main.o


clean :
	rm -f $(TARGET) definition.o game.o main.o json_value.o json_writer.o json_reader.o