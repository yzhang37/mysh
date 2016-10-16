CC = g++
EXEC = mysh
OUTPUT = main.o
HEADER = stdafx.h

$(EXEC): $(OUTPUT)
	$(CC) -o $(EXEC) $(OUTPUT) -g

main.o: main.cpp $(HEADER)
	$(CC) -c main.cpp -g

clear:
	rm -v -f $(OUTPUT) $(EXEC)

re:
	make clear
	make
