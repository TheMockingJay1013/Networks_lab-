# Networks-Assignment-5

Emulating End-to-End Reliable Flow Control over Unreliable Communication Channels

## Introduction

In this assignment, we have built a reliable data transfer protocol along with proper flow control over a UDP connection (not really a connection, but a communication channel). We have created a simple protocol MTP (My Transfer Protocol) which ensures reliable data transfer and is built on top of UDP. The exact protocol details are present in the assignment PDF.

## Files

The necessary functions and structures are defined in the files `msocket.c` and `msocket.h` respectively. We also provide a simple static library `libmsocket.a` which can be linked with the user program to use the MTP protocol. The user program should include the header file `msocket.h` and link the library `libmsocket.a` to use the MTP protocol.

Now, the `msocket.c` functions simply put the messages in a shared buffer but do not send them, similarly they read messages from buffer but do not receive them. There is another file `initmsocket.c` which does the actual sending of the messages. Along with creating the shared memory buffers and other variables (including semaphores) required for the protocol, it also creates two separate threads for sending and receiving messages. The sending thread sends the messages from the shared buffer (if any) and the receiving thread receives the messages and puts them in the shared buffer. Any retransmissions are also handled by the sending thread. While the receiver thread handles the sending as well as processing of ACKs as well. There is also a garbage collector thread which cleans up the respective socket parameters and shared memory buffers periodically after the user process has terminated.

## Usage

To use the MTP protocol, the user program should include the header file `msocket.h` and link the library `libmsocket.a`. The program `initmsocket.c` should be compiled and run before the user program. We have provided a makefile for the same.

Run `make libmsocket.a` to create the static library `libmsocket.a`, then run `make initmsocket` to create the executable `initmsocket`. Finally, run `make user1` to create the user program executable `user1`. (Same for `user2`)

Otherwise you can run `make all` to create all the executables at once. Note that running will also run `./initmsocket`, so you dont need to run it separately.

Now, run `./initmsocket` to start the MTP protocol. Then run `./user1` to start the user program on a separate terminal. user1, after creating the mtp socket and binding, will send the [Romeo and Juliet Book File](https://www.gutenberg.org/cache/epub/1513/pg1513.txt) to the receiver (which is user2). After the file is sent , we send an additional terminal message `##########` so that the receiver can know that the file transfer is over. The sendto of user1 keeps returning -1 when it cannot put the message into the buffer so we put in a while loop where it keeps trying to send the message until it is successful. Also in this busy wait it makes a wait of around 10000 microseconds so ot that the CPU does not busy wait. After it has sent the terminal message, it waits for 60 seconds so that sender thread can complete the transmission of the file, after which the average transimission count is printed.

Note that the user1 program will keep on printing "sendto error" but its not an error, its just that the buffer is full and it is trying to send the message again.

User2 can be run using the command `./user2` and it will receive the file that the sender sends and will write in a file called `juliet.txt`. It also faces the same issue of busy waiting, so we have done a similar busy wait management as in user1. After receiving the file, it gets the terminal message which is not written in the file and then it exits.

Note that the user2 program will keep on printing "recvfrom error" but its not an error, its just that the buffer is empty and it is trying to receive the message again.

Also if we run `time ./user2`, we can see that the time taken to run the user2 and receive the whole file.Albeit this need not be very accurate due to system load and other factors. But it gives a rough idea of the time taken to receive the file.

## Other Files

The `documentation.txt` has description of all the structure and the functions defined in msocket.h and msocket.c respectively. We have tested senting the large file with multiple probability values and the observations found are specified in the above text file.

`romeo.txt` and `juliet.txt` are files that are used for testing the sending and receiving.

One single makefile has been provided which can be used to compile all the files, make the static library and for generating/running the executables.

## Conclusion

We have successfully implemented the MTP protocol which ensures reliable data transfer over an unreliable communication channel (UDP) along with proper flow control. We have tested the protocol with different probability values and have observed the behaviour of the protocol. The protocol is able to send large files reliably over the unreliable channel. We have tested with the Romeo and Juliet book file which is around 166KB in size and a diffchecker shows that the file received is exactly the same as the file sent.
