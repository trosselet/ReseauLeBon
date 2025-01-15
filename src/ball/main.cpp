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
    
    int ret = sendto(data->socket, message, 15, 0, reinterpret_cast<const sockaddr*>(&data->addr), sizeof(data->addr));
    if (ret <= 0)
    {
        std::cout << "Erreur envoi de données : " << WSAGetLastError() << ". Fermeture du programme.";
        return 2;
    }

    return 1;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int cmdShow)
{
    //Init
    sf::RenderWindow window(sf::VideoMode(800, 600), "My window");

    sf::CircleShape shape(20);
    shape.setFillColor(sf::Color::Green);

    shape.setPosition(sf::Vector2f(400, 300));
    sf::Vector2f shapePos = shape.getPosition();

    int multipleX = 1;
    int multipleY = 1;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in ReceiverAddr;
    ReceiverAddr.sin_family = AF_INET;
    ReceiverAddr.sin_port = htons(130);
    ReceiverAddr.sin_addr.s_addr = INADDR_ANY;

    unsigned short portDst = 130;
    sockaddr_in to = { 0 };
    inet_pton(AF_INET, "127.0.0.1", &to.sin_addr.s_addr);
    to.sin_family = AF_INET;
    to.sin_port = htons(portDst);

    Data _data;

    _data.addr = to;
    _data.socket = sendingSocket;

    HANDLE hThread = nullptr;


    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        shape.setPosition(sf::Vector2f(shape.getPosition().x + 0.1 * multipleX, 300));

        if (shape.getPosition().x >= 780 || shape.getPosition().x <= 0)
        {
            multipleX *= -1;
        }
        
        if (shape.getPosition().y > 600 || shape.getPosition().y < 0)
        {
            multipleX *= -1;
        }

        _data.x = shape.getPosition().x;
        _data.y = shape.getPosition().y;

        hThread = CreateThread(NULL, 0, threadFunc, &_data, 0, 0);

        window.clear(sf::Color::Black);

        window.draw(shape);

        window.display();
    }

    closesocket(sendingSocket);

    WSACleanup();

    return 0;
}