
all: svr_s svr_c

svr_s: svr_s.c
	gcc -o svr_s svr_s.c -lpthread header.h

svr_c: svr_c.c
	gcc -o svr_c svr_c.c header.h