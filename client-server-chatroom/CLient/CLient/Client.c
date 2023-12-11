// client code
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

SOCKET Connection;

void ClientHandler() {
    int msg_size;
    while (1) {
        // Receive the username size
        int username_size;
        recv(Connection, (char*)&username_size, sizeof(int), 0);

        // Receive the username
        char* username = (char*)malloc((username_size + 1) * sizeof(char));
        username[username_size] = '\0';
        recv(Connection, username, username_size, 0);

        // Receive the message size
        int msg_size; 
        recv(Connection, (char*)&msg_size, sizeof(int), 0); 

        // Receive the message
        char* msg = (char*)malloc((msg_size + 1) * sizeof(char)); 
        msg[msg_size] = '\0'; 
        recv(Connection, msg, msg_size, 0); 

        // Print the username and the message
        printf("%s%s\n", username, msg);

        free(username);
        free(msg);
    }
}

int main(int argc, char* argv[]) {
    // WSAStartup
    WSADATA wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        printf("Error\n");
        exit(1);
    }

    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(1111);
    addr.sin_family = AF_INET;

    Connection = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
        printf("Error: failed connect to server.\n");
        return 1;
    }
    printf("Connected!\n");

    // Prompt the user for a username
    char username[100];
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    // Clear the input buffer
    fflush(stdin);

    // Send the username to the server
    int username_size = strlen(username);
    send(Connection, (char*)&username_size, sizeof(int), 0);
    send(Connection, username, username_size, 0);

    // Receive the welcome message from the server
    int welcome_size;
    recv(Connection, (char*)&welcome_size, sizeof(int), 0);
    char* welcome_msg = (char*)malloc((welcome_size + 1) * sizeof(char));
    welcome_msg[welcome_size] = '\0';
    recv(Connection, welcome_msg, welcome_size, 0);
    printf("%s\n", welcome_msg);
    free(welcome_msg);

    // Create a separate thread to handle receiving messages from the server
    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

    char msg1[100];
    while (1) {
        fgets(msg1, sizeof(msg1), stdin);
        msg1[strcspn(msg1, "\n")] = '\0';
        int msg_size = strlen(msg1);
        send(Connection, (char*)&msg_size, sizeof(int), 0);
        send(Connection, msg1, msg_size, 0);
        Sleep(10);
    }

    system("pause");
    return 0;
}
