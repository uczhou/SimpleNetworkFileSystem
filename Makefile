CC   = gcc
OPTS = -Wall

all: server libmfs.so

libmfs.so: mfs.o cNetwork.o udp.o
	$(CC) $(CFLAGS) -shared -o libmfs.so mfs.o cNetwork.o udp.o

server: server.o sNetwork.o udp.o diskfs.o utils.o
	$(CC) -o server server.o sNetwork.o udp.o diskfs.o utils.o

%.o: %.c
	$(CC) $(OPTS) -c -fPIC $< -o $@
	
clean:
	rm -rf *.o *.so server

