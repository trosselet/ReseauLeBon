#include "pch.h"
#include "main.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Server Window");
    sf::CircleShape shape(20);
    shape.setFillColor(sf::Color::Red);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET recvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in recvAddr;
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(130);
    recvAddr.sin_addr.s_addr = INADDR_ANY;

    bind(recvSocket, (sockaddr*)&recvAddr, sizeof(recvAddr));

    char buffer[8];
    sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);
    float x = 0.0f, y = 0.0f;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        int recvSize = recvfrom(recvSocket, buffer, 8, 0, (sockaddr*)&senderAddr, &senderAddrSize);
        if (recvSize > 0)
        {
            memcpy(&x, buffer, 4);
            memcpy(&y, buffer + 4, 4);
        }

        shape.setPosition(x, y);

        window.clear(sf::Color::Black);
        window.draw(shape);
        window.display();
    }

    closesocket(recvSocket);
    WSACleanup();

    return 0;
}