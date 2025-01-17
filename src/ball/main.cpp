#include "pch.h"
#include "main.h"

#define WIDTH 800
#define HEIGHT 600
#define SERVER_PORT 130
#define BUFFER_SIZE 12

struct Data
{
    float x;
    float y;
    int id;
    SOCKET socket;
    sockaddr_in serverAddr;
};

struct Player
{
    sf::Color playerColor;
    sf::Color enemyColor;
    sf::Keyboard::Key top;
    sf::Keyboard::Key down;
    sf::Keyboard::Key left;
    sf::Keyboard::Key right;
    int port;
    int id;
};

static DWORD WINAPI threadFunc(void* lPtr)
{
    Data* data = static_cast<Data*>(lPtr);

    char buffer[BUFFER_SIZE];
    int id = -1;
    float x = 0.0f, y = 0.0f;

    while (true)
    {
        sockaddr_in senderAddr{};
        int senderAddrSize = sizeof(senderAddr);

        int recvSize = recvfrom(data->socket, buffer, BUFFER_SIZE, 0, (sockaddr*)&senderAddr, &senderAddrSize);
        if (recvSize > 0)
        {
            memcpy(&id, buffer, 4);
            memcpy(&x, buffer + 4, 4);
            memcpy(&y, buffer + 8, 4);

            data->id = id;
            data->x = x;
            data->y = y;
        }
    }

    return 0;
}

static DWORD WINAPI clientFunc(void* lPtr)
{
    Player* player = static_cast<Player*>(lPtr);

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Client Window");
    window.setPosition(sf::Vector2i(WIDTH * player->id, 270));
    window.setFramerateLimit(60);

    float radius = 20;

    sf::CircleShape playerShape(radius);
    playerShape.setFillColor(player->playerColor);
    playerShape.setOrigin(radius, radius);
    playerShape.setPosition(WIDTH / 2, HEIGHT / 2);

    sf::CircleShape enemyShape(radius);
    enemyShape.setFillColor(player->enemyColor);
    enemyShape.setOrigin(radius, radius);
    enemyShape.setPosition(WIDTH / 2, HEIGHT / 2);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in clientAddr{};
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(player->port);
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    bind(clientSocket, (sockaddr*)&clientAddr, sizeof(clientAddr));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr.s_addr);

    Data playerData{};
    playerData.socket = clientSocket;
    playerData.serverAddr = serverAddr;
    playerData.id = player->id;
    playerData.x = 0.0f;
    playerData.y = 0.0f;

    Data enemyData{};
    enemyData.socket = clientSocket;
    enemyData.serverAddr = serverAddr;
    enemyData.id = -1;
    enemyData.x = 0.0f;
    enemyData.y = 0.0f;

    HANDLE recvThread = CreateThread(nullptr, 0, threadFunc, &enemyData, 0, nullptr);

    const float speed = 0.2f;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(player->top))
            playerData.y -= speed;

        if (sf::Keyboard::isKeyPressed(player->down))
            playerData.y += speed;

        if (sf::Keyboard::isKeyPressed(player->left))
            playerData.x -= speed;

        if (sf::Keyboard::isKeyPressed(player->right))
            playerData.x += speed;
    

        char buffer[BUFFER_SIZE];
        memcpy(buffer, &playerData.id, 4);
        memcpy(buffer + 4, &playerData.x, 4);
        memcpy(buffer + 8, &playerData.y, 4);
        sendto(clientSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

        sf::Vector2f enemyRelativePos(enemyData.x - playerData.x, enemyData.y - playerData.y);

        playerShape.setPosition(WIDTH / 2, HEIGHT / 2);

        enemyShape.setPosition(WIDTH / 2 + enemyRelativePos.x, HEIGHT / 2 + enemyRelativePos.y);

        window.clear(sf::Color::Black);
        window.draw(playerShape);
        window.draw(enemyShape);
        window.display();
    }


    WaitForSingleObject(recvThread, INFINITE);
    CloseHandle(recvThread);

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int cmdShow)
{
    Player playerA = { sf::Color::Green, sf::Color::Yellow, sf::Keyboard::Z, sf::Keyboard::S, sf::Keyboard::Q, sf::Keyboard::D, 131, 0 };
    Player playerB = { sf::Color::Red, sf::Color::Yellow, sf::Keyboard::O, sf::Keyboard::L, sf::Keyboard::K, sf::Keyboard::M, 132, 1 };

    HANDLE clientA = CreateThread(nullptr, 0, clientFunc, &playerA, 0, nullptr);
    HANDLE clientB = CreateThread(nullptr, 0, clientFunc, &playerB, 0, nullptr);

    WaitForSingleObject(clientA, INFINITE);
    WaitForSingleObject(clientB, INFINITE);

    return 0;
}
