# SMTP and POP3 Implementation

## Description

This is a simple SMTP and POP3 server implementation in C. The SMTP server takes a single command line argument, the port number to listen on. Similarly, the POP3 server also takes a single command line argument, the port number to listen on. The client takes three command line arguments, the server address, the SMTP port number and the POP3 port number. Note that the POP3 server and the SMTP server must be running on different ports but same server address so that te mail can be sent and received.

## SMTP Server

The server waits for a connection from a client, then waits for a HELO command. It then waits for a MAIL FROM command, a RCPT TO command, a DATA command, and finally a QUIT command. The server then closes the connection and waits for another client. It communicates with the client using the SMTP protocol and relevant codes. It can handle multiple clients at once.

The client connects to the server, sends a HELO command, a MAIL FROM command, a RCPT TO command, a DATA command, and a QUIT command. It then closes the connection.

First of all, client gives a list of 3 options to the user. Option 1 is for retrieving the mail from the server (POP3 part described later). Option 2 is for sending the mail to the server (SMTP part). Option 3 is for exiting the client. If the user chooses option 2, then the client asks for the sender's mail address, the recipient's mail address and then asks for the message to be sent. The client then sends the message to the server and closes the connection (if the message is valid else it prints an error message and closes the connection). The server then prints the message on the screen as well as in the mailbox file. The client then asks for the next option.

Note that the client does proper error checking in the message part with checks in the "From:", "To:", and "Subject:" fields. It also ensures that the mail address is valid of the form "user@domain". The termination of the message is done with a single period on a line by itself. After that, the client sends a QUIT command to the server and closes the connection.

## POP3 Server

The server waits for a connection from a client, then waits for a USER command and a PASS command. If the username and password are invalid, the server sends an error message and closes the connection. If the username and password are valid, the server sends a +OK message and waits for the next command. Now, the client first sends a STAT command to get the number of mails and valid mail numbers. Then the client sends RETR command with valid mail numbers (over a loop for all valid ones) to get the mail content and then filters it to get the sender, subject, date and time. Now client asks user for a mail to be retrieved and then sends the RETR command with the mail number to get the mail content (if -1 is entered, then the connection is closed by sending a QUIT command). The server then sends the mail content to the client if valid mail number is entered else sends an error message. The client then waits for a key press and then sends the DELE command to delete the mail from the mailbox file if the user presses `d` else nothing happens. Now, if the DELE command is sent, the server sends a +OK message and marks the mail as deleted in the mailbox file but does not actually delete it. The client then again sends the STAT command and prints the list of mails (using RETR command) until user enters -1. If the user enters -1, then the connection is closed (using a QUIT command) and the client program exits to the main menu of 3 options. Now, after the connection is closed, the server actually deletes the mails marked as deleted from the mailbox file. This is done by creating a temporary file and copying all the mails except the ones marked as deleted to the temporary file. Then the temporary file is renamed to the original mailbox file.

## Usage

### Servers

run `gcc smtpmail.c -o smtpmail` and `gcc popserver.c -o popserver` to compile the servers. Then run `./popserver <pop3_port>` and `./smtpmail <smtp_port>` to run the servers. The servers will then start listening on the given port. The servers will keep running until they are manually stopped. Note that the POP3 server and the SMTP server must be running on different ports but same server address so that te mail can be sent and received.

### Client

run `gcc mailclient.c -o mailclient` to compile the client. Then run `./mailclient <server address> <smtp_port> <pop3_port>` to run the client. Then follow the instructions on the screen to send a mail to the server. First of all the username and password are asked for POP3 authentication (if user selects option 1) and checks with the user.txt file by sending to the POP3 server.

If the user selects option 2, then the client asks for the sender's mail address, the recipient's mail address and then asks for the message to be sent. The client then sends the message to the server and closes the connection (if the message is valid else it prints an error message and closes the connection). The server then prints the message on the screen as well as in the mailbox file. The client then asks for the next option. If the user chooses option 3, then the client exits.

Note that for the receiver to get the message in their mailbox, the required mailbox file should be present inside a subdirectory with their name in the server directory. For example, if the receiver's mail address is "test@test", then the mailbox file will be saved in the file "mailbox" inside the directory "test" in the server directory. Here we have provided some sample mailbox files for testing purposes. If the user requires any other mailbox file, then they have to create it manually (mentioned in the assignment).

If the user selects option 1, first of all the username and password are checked for POP3 authentication (note that they have already been asked by client program). Then a list of mails (serial number, sender, subject, date and time) is printed on the screen. The user can then select any valid mail number to retrieve the mail. The mail is then printed on the screen if number is valid else an error message is printed and the client asks again for a valid mail number. After a mail has been printed, the client program waits on a getchar() for the user. Now if the user presses "d" then the mail is deleted from the mailbox file (not actually deleted but the mail is marked as deleted in the mailbox file and will be deleted when the connection is closed). If user selects any other key, nothing happens.

In either case, the client shows the list of mails again and asks for a mail number. If the user enters -1, then the connection is closed (using a QUIT command) and the client program exits to the main menu of 3 options. Again, if the user selects option 3, then the client exits.
