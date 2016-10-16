CC = g++
EXEC = mysh
OUTPUT = main.o command.o
HEADER = command.h stdafx.h

$(EXEC): $(OUTPUT)
	$(CC) -o $(EXEC) $(OUTPUT)

main.o: main.cpp $(HEADER)
	$(CC) -c main.cpp

command.o: command.cpp $(HEADER)
	$(CC) -c command.cpp

clear:
	rm -v -f $(OUTPUT) $(EXEC)
