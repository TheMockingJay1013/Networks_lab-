server : compiler_server
	./server

compiler_server : fileserver.c
	gcc -w fileserver.c -o server

client : compiler_client
	./client

compiler_client : fileclient.c
	gcc -w fileclient.c -o client