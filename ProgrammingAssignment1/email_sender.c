/* 
    Made by: Horacio Gonzalez
    CS 484 Computer Networks I
    This program uses network sockets to connect to an SMTP server and send an email
*/ 



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <regex.h>
#include <string.h>

int main(){

    /*
        Copies all received data from the TCP socket buffer to <buff>.
        This is a blocking call.
        Suggested buffer length for SMTP: 1024 or more bytes.
        Returns the number of bytes read
    */
    // int socket_receive(int ssocket, char* buff, int buffsize) {
    //     return read(ssocket,buff, buffsize);
    //     //https://man7.org/linux/man-pages/man2/read.2.html
    // }


    /*
        Sends data out over a TCP socket.

        Warning: this function will work fine for sending SMTP messages line by line
        (because they are short), but in general you many need to handle the case were
        only part of your message is sent at a time.

        Returns the number of bytes sent
    */ 
    // int socket_send(int ssocket, char* msg, int msglen) {
    // return write(ssocket,msg,msglen);
    // //https://man7.org/linux/man-pages/man2/write.2.html
    // }
    return 1;
} // end main

