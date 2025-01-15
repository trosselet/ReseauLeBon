#include "pch.h"
#include "main.h"

struct Data
{
    float x;
    float y;
    SOCKET socket;
    sockaddr_in addr;
};

static DWORD WINAPI threadFunc(void* lPtr)
{
    Data* data = static_cast<Data*>(lPtr);

    char message[8];
    memcpy(message, &data->x, 4);
    memcpy(message + 4, &data->y, 4);

    int ret = sendto(data->socket, message, 8, 0, reinterpret_cast<const sockaddr*>(&data->addr), sizeof(data->addr));
    if (ret <= 0)
    {
        std::cout << "Erreur envoi de données : " << WSAGetLastError() << ". Fermeture du programme.";
        return 2;
    }

    return 1;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int cmdShow)
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Client Window");
    sf::CircleShape shape(20);
    shape.setFillColor(sf::Color::Green);

    shape.setPosition(400, 300);
    sf::Vector2f shapePos = shape.getPosition();

    int dirX = 1;
    int dirY = 1;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in to;
    inet_pton(AF_INET, "127.0.0.1", &to.sin_addr.s_addr);
    to.sin_family = AF_INET;
    to.sin_port = htons(130);

    Data data;
    data.addr = to;
    data.socket = sendingSocket;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Update ball position
        shape.move(0.1f * dirX, 0.1f * dirY);
        if (shape.getPosition().x <= 0 || shape.getPosition().x >= 780) dirX *= -1;
        if (shape.getPosition().y <= 0 || shape.getPosition().y >= 580) dirY *= -1;

        // Send ball position
        data.x = shape.getPosition().x;
        data.y = shape.getPosition().y;
        CreateThread(nullptr, 0, threadFunc, &data, 0, nullptr);

        // Render
        window.clear(sf::Color::Black);
        window.draw(shape);
        window.display();
    }

    closesocket(sendingSocket);
    WSACleanup();

    return 0;
}