CC = g++
EXEC = mysh
OUTPUT = main.o
HEADER = stdafx.h

$(EXEC): $(OUTPUT)
	$(CC) -o $(EXEC) $(OUTPUT)

main.o: main.cpp $(HEADER)
	$(CC) -c main.cpp

clear:
	rm -v -f $(OUTPUT) $(EXEC)
