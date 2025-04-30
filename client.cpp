#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 54000

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    server_addr.sin_port = htons(PORT);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::string login, password;
    std::cout << "Login: ";
    std::cin >> login;
    std::cout << "Password: ";
    std::cin >> password;

    std::string message = login + ":" + password;
    send(sock, message.c_str(), message.length(), 0);

    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        std::cout << "Server response:\n" << std::string(buffer, bytesReceived) << std::endl;
    }
    else {
        std::cout << "No response or error receiving data.\n";
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}