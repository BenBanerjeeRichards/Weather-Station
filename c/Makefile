FLAGS = -Wall -Wno-unused-variable
COMPILER = clang

out: main.o ws.o
	$(COMPILER) main.o ws.o $(FLAGS) -o out -lusb-1.0

main.o: main.c
	$(COMPILER) -c -g main.c $(FLAGS)
	
ws.o: ws.c
	$(COMPILER) -c -g ws.c $(FLAGS)
