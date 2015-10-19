all: server.x client.x test_bolas.x test_jugador.x test_cartones.x

server.x : bingo_server.o bolas.o cartones.o jugador.o partida.o
	gcc -g -o server.x bingo_server.o bolas.o cartones.o jugador.o partida.o

client.x : bingo_client.o bolas.o cartones.o jugador.o partida.o
	gcc -g -o client.x bingo_client.o bolas.o cartones.o jugador.o partida.o

test_bolas.x : test_bolas.o bolas.o
	gcc -lcunit -g -o test_bolas.x test_bolas.o bolas.o

test_jugador.x : test_jugador.o jugador.o
	gcc -lcunit -g -o test_jugador.x test_jugador.o jugador.o

test_cartones.x : test_cartones.o cartones.o bolas.o
	gcc -lcunit -g -o test_cartones.x test_cartones.o cartones.o bolas.o

bingo_client.o : bingo_client.c
	gcc -g -c bingo_client.c

test_bolas.o : test_bolas.c
	gcc -g -c test_bolas.c -lcunit -g

test_jugador.o : test_jugador.c
	gcc -g -c test_jugador.c -lcunit -g

bingo_server.o : bingo_server.c
	gcc -g -c bingo_server.c partida.h jugador.h cartones.h

bolas.o : bolas.c bolas.h
	gcc -g -c bolas.c bolas.h

cartones.o : cartones.c cartones.h bolas.h
	gcc -g -c cartones.c cartones.h bolas.h

jugador.o : jugador.c jugador.h bolas.h
	gcc -g -c jugador.c jugador.h bolas.h

partida.o : partida.c partida.h bolas.h jugador.h cartones.h
	gcc -g -c partida.c partida.h bolas.h jugador.h cartones.h

clean :
	rm *.o *.x *.*~ *.*.gch