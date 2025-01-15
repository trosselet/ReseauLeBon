#include "pch.h"
#include "main.h"

struct ThreadArgs
{
    SOCKET listenSocket;
    SOCKET* clientSocket;
};

static DWORD WINAPI threadFunc(void* lPtr)
{
    ThreadArgs* acceptArgs = static_cast<ThreadArgs*>(lPtr);
    SOCKET listenSocket = acceptArgs->listenSocket;
    SOCKET* clientSocket = acceptArgs->clientSocket;

    sockaddr_in client_address;
    int client_address_len = sizeof(client_address);

    std::cout << "Waiting for a client to connect..." << std::endl;

    *clientSocket = accept(listenSocket, (sockaddr*)&client_address, &client_address_len);

    if (*clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Client rejected : " << WSAGetLastError() << std::endl;
        return 1;
    }

    std::cout << "Client connected!" << std::endl;
    return 0;
}

int main()
{
    std::cout << "SERVEUR" << std::endl;

    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);

    sockaddr_in listen_address;
    listen_address.sin_family = AF_INET;
    listen_address.sin_port = htons(130);
    listen_address.sin_addr.S_un.S_addr = INADDR_ANY;


    SOCKET listen_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    bind(listen_socket, (sockaddr*)&listen_address, sizeof(listen_address));
    listen(listen_socket, SOMAXCONN);

    SOCKET client_socket = INVALID_SOCKET;

    ThreadArgs acceptArgs = { listen_socket, &client_socket };

    /*HANDLE _Thread = CreateThread(nullptr, 0, threadFunc, &acceptArgs, 0, nullptr);*/

    sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    char buffer[1024];
    int iResult;

    while (true)
    {
        
        int iResult = recvfrom(listen_socket, buffer, 1023, 0, reinterpret_cast<sockaddr*>(&from), &fromlen);

        if (iResult > 0)
        {
            buffer[iResult] = '\0';
            std::cout << "Received: " << buffer << std::endl;

            std::string response = "Message received: " + std::string(buffer);

        }
        else if (iResult == 0)
        {
            std::cout << "Client disconnected." << std::endl;
            break;
        }
        else
        {
            std::cout << "Erreur réception de données : " << WSAGetLastError() << ". Fermeture du programme.";
            break;
        }
    }

    closesocket(client_socket);

    closesocket(listen_socket);

    WSACleanup();

    return 0;
}