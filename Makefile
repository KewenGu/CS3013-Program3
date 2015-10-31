all: client server

client: client/client
	gcc client/client.c -o client/client


server: server/server
	gcc server/server.c -o server/server


clean:
	rm client/client
	rm server/server