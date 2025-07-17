// server.cpp (Linux)
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

vector<int> clients;
mutex clients_mutex;

void broadcast(const string& message, int senderSocket) {
    lock_guard<mutex> lock(clients_mutex);
    for (int client : clients) {
        if (client != senderSocket) {
            send(client, message.c_str(), message.length(), 0);
        }
    }
}

void handleClient(int clientSocket) {
    char buffer[1024];
    ssize_t bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        string msg = buffer;

        cout << "Received: " << msg;
        broadcast(msg, clientSocket);
    }

    // Remove client
    {
        lock_guard<mutex> lock(clients_mutex);
        clients.erase(remove(clients.begin(), clients.end(), clientSocket), clients.end());
    }

    close(clientSocket);
    cout << "Client disconnected\n";
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Failed to create socket\n";
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Bind failed\n";
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) < 0) {
        cerr << "Listen failed\n";
        return 1;
    }

    cout << "Server running on port 54000...\n";

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientSize = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket == -1) {
            cerr << "Failed to accept connection\n";
            continue;
        }

        {
            lock_guard<mutex> lock(clients_mutex);
            clients.push_back(clientSocket);
        }

        thread(handleClient, clientSocket).detach();
        cout << "New client connected\n";
    }

    close(serverSocket);
    return 0;
}
