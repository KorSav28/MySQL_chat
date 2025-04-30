#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mysql.h>
#include <cstring>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

#define PORT 54000

MYSQL* conn;

void initDatabase() {
    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", "root", "password", "chatdb", 0, NULL, 0);

    const char* createUsers = "CREATE TABLE IF NOT EXISTS users (login VARCHAR(50) PRIMARY KEY, password VARCHAR(50))";
    const char* createMsgs = "CREATE TABLE IF NOT EXISTS messages (sender VARCHAR(50), content TEXT)";

    mysql_query(conn, createUsers);
    mysql_query(conn, createMsgs);
}

bool registerUser(const std::string& login, const std::string& password) {
    std::string query = "INSERT IGNORE INTO users (login, password) VALUES ('" + login + "', '" + password + "')";
    return mysql_query(conn, query.c_str()) == 0;
}

std::string getUsers() {
    std::stringstream ss;
    mysql_query(conn, "SELECT login FROM users");
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        ss << row[0] << "\n";
    }
    mysql_free_result(res);
    return ss.str();
}

std::string getMessages() {
    std::stringstream ss;
    mysql_query(conn, "SELECT sender, content FROM messages");
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        ss << row[0] << ": " << row[1] << "\n";
    }
    mysql_free_result(res);
    return ss.str();
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    initDatabase();

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{}, client_addr{};
    int addr_size = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);
    std::cout << "Server started on port " << PORT << std::endl;

    while (true) {
        SOCKET client = accept(server_fd, (sockaddr*)&client_addr, &addr_size);
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        recv(client, buffer, sizeof(buffer), 0);

        std::string creds(buffer);
        std::string login = creds.substr(0, creds.find(':'));
        std::string password = creds.substr(creds.find(':') + 1);

        if (registerUser(login, password)) {
            std::string response = "Users:\n" + getUsers() + "\nMessages:\n" + getMessages();
            send(client, response.c_str(), response.length(), 0);
        }
        else {
            std::string msg = "Registration failed.";
            send(client, msg.c_str(), msg.length(), 0);
        }

        closesocket(client);
    }

    mysql_close(conn);
    WSACleanup();
    return 0;
}