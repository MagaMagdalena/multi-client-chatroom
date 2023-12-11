//Server code
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

#define MAX_CLIENTS 100
#define MAX_MSG_SIZE 1000

struct Client {
    int connection;
    char username[100];
};

struct Client clients[MAX_CLIENTS];
int num_clients = 0;

void ClientHandler(void* clientIndex) {
    int index = *(int*)clientIndex;
    int msg_size;
    while (1) {
        //Receiving client message
        recv(clients[index].connection, (char*)&msg_size, sizeof(int), 0);
        char* msg = (char*)malloc((msg_size + 1) * sizeof(char));
        msg[msg_size] = '\0';
        recv(clients[index].connection, msg, msg_size, 0);

        // Prepend the sender's username before the message
        char full_msg[MAX_MSG_SIZE];
        sprintf(full_msg, "%s: %s", clients[index].username, msg);

        // Print the message to the server terminal
        printf("%s\n", full_msg);

        // Open the file in append mode and write the message
        FILE* file = fopen("conversation.txt", "a");
        if (file != NULL) {
            fprintf(file, "%s\n", full_msg);
            fclose(file);
        }

        for (int i = 0; i < num_clients; i++) {
            if (i == index) {
                continue;
            }

            send(clients[i].connection, (char*)&msg_size, sizeof(int), 0);
            send(clients[i].connection, full_msg, msg_size, 0);
        }
        free(msg);
    }
}

int main(int argc, char* argv[]) {
    // Initialize Winsock
    WSADATA wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        printf("Error initializing Winsock\n");
        exit(1);
    }

    // Create socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Error creating socket\n");
        WSACleanup();
        exit(1);
    }

    // Bind socket to address
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(1111);
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("Error binding socket\n");
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    // Listen for connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Error listening for connections\n");
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }

    // Accept client connections
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    SOCKET clientSocket;
    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);

        if (clientSocket == INVALID_SOCKET) {
            printf("Error accepting client connection\n");
        }
        else {
            printf("Client Connected!\n");

            // Receive the username from the client
            int username_size;
            recv(clientSocket, (char*)&username_size, sizeof(int), 0);
            char* username = (char*)malloc((username_size + 1) * sizeof(char));
            username[username_size] = '\0';
            recv(clientSocket, username, username_size, 0);

            // Create a new client and store the connection and username
            struct Client client;
            client.connection = clientSocket;
            strcpy(client.username, username);
            clients[num_clients++] = client;

            // Send a welcome message to the client
            char welcome_msg[MAX_MSG_SIZE];
            sprintf(welcome_msg, "Welcome, %s!", client.username);
            int msg_size = strlen(welcome_msg);
            send(clientSocket, (char*)&msg_size, sizeof(int), 0);
            send(clientSocket, welcome_msg, msg_size, 0);

           

            // Create a new thread to handle the client
            int clientIndex = num_clients - 1;
            _beginthread(ClientHandler, 0, (void*)&clientIndex);

        }
    }

    // Cleanup and exit
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

