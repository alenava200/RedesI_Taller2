
all: server client

server: svr_s.c
	gcc -o server svr_s.c -lpthread

client: svr_c.c
	gcc -o client svr_c.c