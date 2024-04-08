# Networks-Assignment-6

Implementing a Custom Protocol using Raw Sockets
git 
## How to run

Execute `make all` to compile the `simDNSServer.c` and `simDNSClient.c` files.

Run the server using `./server` and the client using `./client` on two different terminals. Note that you need sudo permissions to run the executables.

### Server

The server will first of all ask the user the interface name to bind to and the MAC address. We can get the list of interfaces as well as their MAC addresses by running `ifconfig` on the terminal. Please note that the interface name and the MAC address should be entered in the same format as shown by the `ifconfig` command. After this, the server asks the user to enter the IP address of the server. The format to enter all the above fields is clearly mentioned in the prompt.

The server will then wait for the client to send a request. On receiving the request, the server will send a response back to the client. It first runs `gethostbyname` to get the IP addresses associated with the queries (with proper error handling). It also gets the IP address and the MAC address of the client from the request packet and sends it back to the client in the response packet. The server may drop packets (the ones that are actual queries) randomly with a probability of `P` defined in the `simDNSServer.c` file and can be changed as per your need (in the `#define P` statement in `simDNSServer.c`).

Note that server drops all the packets with wrong IP and/or Protocol. *(IP Protocol 254 is used for Experimentation and Testing, as this was mentioned to be found out in the assignment statement)*. The server will keep running until the user stops it by pressing `Ctrl+C` or `Ctrl+Z`, upon which it just closes the socket and exits.

### Client

The client will first of all ask the user the interface name to bind to and the MAC address. We can get the list of interfaces as well as their MAC addresses by running `ifconfig` on the terminal. Please note that the interface name and the MAC address should be entered in the same format as shown by the `ifconfig` command. After this, the client asks the user to enter the IP address of the client. The format to enter all the above fields is clearly mentioned in the prompt.

The client also needs the MAC address and the IP address of the server to send the request. This should match exactly with the server's MAC address and IP address else communication will not be possible. The client will then wait on a prompt where the user can enter the query to be sent to the server.

The proper format to enter the query is also mentioned in the prompt. If the user tries to enter a query in the wrong format, the client will print the corresponding error message and ask the user to enter the query again. If the query is in the correct format, the client will send the request to the server and wait for the response. To match the request and response, the client will put in a identifier in the request packet and the server will send the same identifier back in the response packet. If the identifier in the response packet does not match the identifier in the request packet, the client will print Invalid Response and wait for the next response.

Note that the client waits on a select on both the socket and the stdin. This is done so that the client can take input from the user and also receive the response from the server. All the packets with wrong IP and/or Protocol are also dropped by the client. On receiving the response, the client will first check if the identifier in the response packet matches the identifier it has stored in the table. If it matches, the client will print the IP addresses along with the domain names we put had put in the queries. If the identifier does not match, the client will print Invalid Response and wait for the next response.

The client will keep running until the user stops it by pressing `Ctrl+C` or `Ctrl+Z`, upon which it just closes the socket and exits. The user can also stop the client by entering `EXIT` in the prompt which also closes the socket and exits normally.
