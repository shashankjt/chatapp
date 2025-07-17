// client.cpp
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

int clientSocket;

void receiveMessages() {
    char buffer[1024];
    ssize_t bytesReceived;
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        cout << "\r[Server] " << buffer << "\n> " << flush;
    }
    cout << "\nDisconnected from server.\n";
}

int main() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Socket creation failed.\n";
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Connection to server failed.\n";
        return 1;
    }

    cout << "Connected to server. Type messages below.\n";

    thread receiver(receiveMessages);

    string message;
    while (true) {
        cout << "> ";
        getline(cin, message);
        if (message == "exit") break;
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    close(clientSocket);
    receiver.join();
    return 0;
}
