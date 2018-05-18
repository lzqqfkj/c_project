all: server client
server:server.c socket.c socket.h
		gcc -o server server.c socket.c socket.h

client: client.c socket.c socket.h
		gcc -o client client.c socket.c socket.h
clean:
		rm server client
