server : compiler_server
	./server

compiler_server : server.c
	gcc -w server.c -o server

client : compiler_client
	./client

compiler_client : client.c
	gcc -w client.c -o client