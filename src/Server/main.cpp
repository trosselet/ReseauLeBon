#include "pch.h"
#include "main.h"

#define SERVER_PORT 130
#define BUFFER_SIZE 12
#define MAX_CLIENTS 2

struct ClientData {
    int id;
    float x, y;
    sockaddr_in addr;
};

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    ClientData clients[MAX_CLIENTS] = { 0 };
    int clientCount = 0;

    char buffer[BUFFER_SIZE];
    sockaddr_in clientAddr{};
    int clientAddrSize = sizeof(clientAddr);

    std::cout << "Serveur en attente des connexions..." << std::endl;

    while (true) {
        int recvSize = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrSize);
        if (recvSize > 0) {
            int id;
            float x, y;
            memcpy(&id, buffer, 4);
            memcpy(&x, buffer + 4, 4);
            memcpy(&y, buffer + 8, 4);

            bool clientExists = false;
            for (int i = 0; i < clientCount; ++i) {
                if (clients[i].id == id) {
                    clients[i].x = x;
                    clients[i].y = y;
                    clientExists = true;
                    break;
                }
            }

            if (!clientExists && clientCount < MAX_CLIENTS) {
                clients[clientCount].id = id;
                clients[clientCount].x = x;
                clients[clientCount].y = y;
                clients[clientCount].addr = clientAddr;
                clientCount++;
                std::cout << "Nouveau client connecté : ID = " << id << std::endl;
            }

            char sendBuffer[BUFFER_SIZE * MAX_CLIENTS] = { 0 };
            for (int i = 0; i < clientCount; ++i) {
                memcpy(sendBuffer + i * BUFFER_SIZE, &clients[i].id, 4);
                memcpy(sendBuffer + i * BUFFER_SIZE + 4, &clients[i].x, 4);
                memcpy(sendBuffer + i * BUFFER_SIZE + 8, &clients[i].y, 4);
            }

            for (int i = 0; i < clientCount; ++i) {
                sendto(serverSocket, sendBuffer, BUFFER_SIZE * clientCount, 0,
                    (sockaddr*)&clients[i].addr, sizeof(clients[i].addr));
            }
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
