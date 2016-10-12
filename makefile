CC = g++
EXEC = mysh
OUTPUT = main.o

$(EXEC): $(OUTPUT)
	$(CC) -o $(EXEC) $(OUTPUT)

main.o: main.cpp
	$(CC) -c main.cpp

clear:
	rm -v -f $(OUTPUT) $(EXEC)
