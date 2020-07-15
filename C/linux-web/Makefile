CC := gcc 
CFLAGS = -g -Wall -I .
OBJS := main.o http_server.o http_formatprocess.o
DEPEND := http_server.h http_base.h http_formatprocess.h

all: main 

main:$(OBJS)
	$(CC) $(CFLAGS) -o main $(OBJS)

main.o: main.c $(DEPEND)
	$(CC) $(CFLAGS) -c main.c

http_formatprocess.o: http_formatprocess.c $(DEPEND)
	$(CC) $(CFLAGS) -c http_formatprocess.c

http_server.o: http_server.c $(DEPEND)
	$(CC) $(CFLAGS) -c http_server.c

.PHONY:clean
clean:
	rm -rf *.o main core