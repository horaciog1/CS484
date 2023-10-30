/* 
    Made by: Horacio Gonzalez
    CS 484 Computer Networks I
    This program uses network sockets to connect to an SMTP server and send an email
    
    *note: the body of the email will change if you change it in the txt file, but the headers wont.
        Usage: ./email_sender <SMTP ADDR IPv4> <DEST Email ADDR> <Email Filename>"
        The email cointained in the <Email Filename> will be send to the <DEST Email ADDR> you put

    References: https://stackoverflow.com/questions/16486361/creating-a-basic-c-c-tcp-socket-writer
                https://www.geeksforgeeks.org/check-if-given-email-address-is-valid-or-not-in-cpp/
                https://www.w3schools.com/c/c_files_read.php
                https://www.geeksforgeeks.org/socket-programming-cc/#
                https://en.wikipedia.org/wiki/List_of_SMTP_server_return_codes
                https://www.tutorialspoint.com/cprogramming/c_command_line_arguments.htm
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <regex.h>
#include <string.h>

#define FROM "horaciog@nmsu.edu"
#define TO "horaciog@nmsu.edu" 
#define EMAIL_SUBJECT "CS484 - I can program with sockets!"

int readResponse(int ssocket, char *buff, int buffsize) {
    int totalBytesRead = 0;
    do {
        memset(buff, 0, buffsize);
        int bytesRead = read(ssocket, buff, buffsize);
        
        if (bytesRead <= 0) {
            printf("Server is not responding!\n");
            return -1;
        }

        totalBytesRead += bytesRead;
        printf("Server Response: %s", buff); // Print each line as it's received

        // Check if this is the last line of the response
        if (bytesRead >= 4 && buff[3] == ' ')
            break;
        
    } while (totalBytesRead < buffsize - 1);

    return totalBytesRead;
} // end readResponse()

// Error condition to check for SMTP commands rejection (not implemented)
int checkSMTPResponse(int clientSocket, char *responseBuffer, const char *command) {
    if (readResponse(clientSocket, responseBuffer, sizeof(responseBuffer) - 1) == -1) {
        fprintf(stderr, "Failed to get the response from the server.\n");
        return 0; // Indicate failure
    }

    // Check the response for SMTP command rejection
    if (strncmp(responseBuffer, "5", 1) == 0) {
        // Handle permanent error (e.g., "500" series response)
        fprintf(stderr, "SMTP server rejected the command '%s': %s", command, responseBuffer);
        return 0; // Indicate failure
    } else if (strncmp(responseBuffer, "4", 1) == 0) {
        // Handle temporary error (e.g., "400" series response)
        fprintf(stderr, "SMTP server temporarily rejected the command '%s': %s", command, responseBuffer);
        // You can decide whether to retry the command or handle it as needed
        return 0; // Indicate failure
    }

    return 1; // Indicate success
}

int sendToTerminal(int ssocket, const char *command) {
    return write(ssocket, command, strlen(command));
} // end sendToTerminal()

char* getEmail(char *emailFileName) {
    // Check if the file exists
    FILE* email = fopen(emailFileName, "r");
    if (email == NULL) {
        fprintf(stderr, "Invalid or nonexistent email file specified: %s\n", emailFileName);
        exit(1);
    }

    // Navigate to the end of the file to get its size
    fseek(email, 0, SEEK_END);
    long emailSize = ftell(email);

    if (emailSize <= 0) {
        fprintf(stderr, "Email file is empty or invalid: %s\n", emailFileName);
        fclose(email);
        exit(1);
    }

    // Allocate memory for the email content, adding one byte for the null-terminator
    char* data = (char*)malloc(emailSize + 1);

    // Navigate back to the beginning of the file to read its contents
    fseek(email, 0, SEEK_SET);

    // Read the email content into the allocated memory
    fread(data, 1, emailSize, email);

    // Null-terminate the loaded data
    data[emailSize] = '\0';

    // Close the email file
    fclose(email);

    return data;
} // end getEmail()


// char* getLine(char *emailFileName, int lineNumber) {
//     FILE *email = fopen(emailFileName, "r");

//     char *line = NULL;
//     size_t len = 0;
//     ssize_t read;
//     int currentLine = 0;

//     // Read and find the desired line
//     while ((read = getline(&line, &len, email) != -1) ){
//         currentLine++;
//         if (currentLine == lineNumber) {
//             break;  // Found the desired line
//         }
//     }

//     // Close the file
//     fclose(email);

//     if (line) {
//         return line;
//     } else {
//         return NULL; // Line not found
//     }
// }

// Error condition for email address
int emailIsValid(const char *email) {
    regex_t regex;

    /*
    This line compiles the regular expression and returns an error code in the variable ret. 
    It uses regcomp to compile the regex pattern ^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$ 
    with extended syntax (REG_EXTENDED). If compilation fails, ret will hold a non-zero error code.
    */
    int ret = regcomp(&regex, "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$", REG_EXTENDED);

    if (ret) {
        regfree(&regex);
        return 0;  // Compilation of regex failed, return false
    }

    ret = regexec(&regex, email, 0, NULL, 0);

    //Frees the memory used by the compiled regular expression pattern to avoid memory leaks
    regfree(&regex);

    return (ret == 0); // Return true if regex match was successful, false otherwise
} // end emailIsValid

int main(int argc, char *argv[]) {
    char responseBuffer[1024];
    // char *fromLine = getLine(argv[4], 1);
    // char *toLine = getLine(argv[4], 2);
    // char *subjectLine = getLine(argv[4], 3);


    //Check for arguments
    if (argc != 4) {
        printf("Usage: %s <SMTP ADDR IPv4> <DEST Email ADDR> <Email Filename>\n", argv[0]);
        exit(1);
    }

    // <SMTP ADDR IPv4>
    char *addressIP = argv[1];
    // <DEST Email ADDR>
    char *dest_email = argv[2];
    // <Email Filename>
    char *email_filename = argv[3];
    char serverResponse[1024];
    
    if (!emailIsValid(dest_email)) {
        fprintf(stderr, "Invalid email address specified\n");
        exit(1);
    }

    // Error condition for socket creation failed
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(25);
    server_addr.sin_addr.s_addr = inet_addr(addressIP);

    // Error condition for connection to SMTP server failed
    if (connect(clientSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(clientSocket);
        exit(1);
    }

    //check if the IP Address is valid
    if (server_addr.sin_addr.s_addr == INADDR_NONE) {
        fprintf(stderr, "Invalid IP address.\n");
        exit(1);
    }


    if (readResponse(clientSocket, serverResponse, sizeof(serverResponse) - 1) == -1) {
        printf("Failed to get response from the server.\n");
        close(clientSocket);
        exit(1);
    }
    printf("Server Response: %s", serverResponse);
    
    char rcptToCommand[256];
    
   // Concatenate "RCPT TO:" and dest_email into rcptToCommand
    sprintf(rcptToCommand, "RCPT TO: %s\r\n", dest_email);

    // EHLO does the same as HELO but it contains more information of the response
    sendToTerminal(clientSocket, "EHLO example.com\r\n");
    sendToTerminal(clientSocket, "MAIL FROM: " FROM "\r\n");
    sendToTerminal(clientSocket, rcptToCommand);
    sendToTerminal(clientSocket, "DATA\r\n");

    // Body of email
    sendToTerminal(clientSocket, "Subject: " EMAIL_SUBJECT "\r\n");
    sendToTerminal(clientSocket, "From: " FROM "\r\n");
    sendToTerminal(clientSocket, "To: " TO "\r\n");
    sendToTerminal(clientSocket, "\r\n");  // Separation

    // Read the email from file and send it
    char *emailContent = getEmail(email_filename);
    if(emailContent == NULL) {
        perror("Error reading email content");
        close(clientSocket);
        exit(1);
    }
      
    // Send the email content
    sendToTerminal(clientSocket, emailContent);

    //free the memory
    free(emailContent);

    sendToTerminal(clientSocket, ".\r\n");
 
    sendToTerminal(clientSocket, "QUIT\r\n");
    
    // Error check for sending email too often
    if (strncmp(responseBuffer, "452", 3) == 0) {
        fprintf(stderr, "SMTP server is blocking you from sending emails too often: %s", responseBuffer);
        close(clientSocket);
        exit(1);
    }


    if (readResponse(clientSocket, serverResponse, sizeof(serverResponse) - 1) == -1) {
        printf("Failed to get final response from server.\n");
        close(clientSocket);
        exit(1);
    }

    printf("Server Response: %s", serverResponse);

    close(clientSocket);
    return 0;
} // end main()