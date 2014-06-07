FLAGS = -Wall -Wno-unused-variable -no-integrated-as
COMPILER = clang

out: main.o ws.o station.o
	$(COMPILER) main.o ws.o station.o $(FLAGS) -o out -lusb-1.0 -lm

main.o: main.c
	$(COMPILER) -c -g main.c $(FLAGS)
	
station.o: station.c
	$(COMPILER) -c -g station.c -o station.o $(FLAGS)

ws.o: ws.c
	$(COMPILER) -c -g ws.c $(FLAGS)


