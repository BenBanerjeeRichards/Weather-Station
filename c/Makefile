FLAGS = -Wall -Wno-unused-variable -no-integrated-as
COMPILER = clang

out: main.o ws.o station.o ws_store.o config.o
	$(COMPILER) main.o ws.o station.o  ws_store.o  config.o $(FLAGS) -o out -lusb-1.0 -lsqlite3 -lm

main.o: main.c
	$(COMPILER) -c -g main.c $(FLAGS)
	
station.o: station.c
	$(COMPILER) -c -g station.c -o station.o $(FLAGS)

ws.o: ws.c
	$(COMPILER) -c -g ws.c $(FLAGS)

ws_store.o: ws_store.c
	$(COMPILER) -c -g ws_store.c $(FLAGS)

config.o: config.c
	$(COMPILER) -c -g config.c $(FLAGS)