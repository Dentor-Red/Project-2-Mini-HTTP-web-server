#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sstream>
#include <fstream>



int main() {
    // Create a TCP socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); 

    if (serverSocket < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // Configure the server address
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET; // 
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    // Bind the socket to port 8080
    if (bind(
            serverSocket,
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress)) < 0) {

        std::cerr << "Failed to bind socket\n";
        close(serverSocket);
        return 1;
    }

    // Start listening for connections
    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Failed to listen on socket\n";
        close(serverSocket);
        return 1;
    }

    std::cout << "Server running on http://localhost:8080\n";

    // Keep accepting clients
    while (true) {
        sockaddr_in clientAddress{};
        socklen_t clientAddressSize = sizeof(clientAddress);

        int clientSocket = accept(
            serverSocket,
            reinterpret_cast<sockaddr*>(&clientAddress),
            &clientAddressSize
        );

        if (clientSocket < 0) {
            std::cerr << "Failed to accept client connection\n";
            close(serverSocket);
            return 1;
        }

        // Read the HTTP request from the client
        char buffer[4096] = {0};

        ssize_t bytesReceived = recv(
            clientSocket,
            buffer,
            sizeof(buffer) - 1,
            0
        );

        std::string path = "/";

        if (bytesReceived > 0) {
            std::cout << "\nReceived request:\n";
            std::cout << buffer << "\n";

            std::istringstream requestStream(buffer);

            std::string method;
            std::string requestedPath;
            std::string version;

            requestStream >> method >> requestedPath >> version;

            path = requestedPath;

            std::cout << "Method: " << method << "\n";
            std::cout << "Path: " << path << "\n";
        }

        // Create an HTTP response
        std::string body;

        if (path == "/") {
            std::ifstream file("public/index.html");

            if (file) {
                std::stringstream contents;
                contents << file.rdbuf();
                body = contents.str();
            }
            else {
                body = "Could not open index.html";
            }
        }
        else if (path == "/about") {
            body = "This is my C++ web server project.";
        }
        else if (path == "/test") {
            body = "Test page works!";
        }
        else {
            body = "404 Page Not Found";
        }

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(body.length()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" +
            body;

        // Send response
        send(
            clientSocket,
            response.c_str(),
            response.length(),
            0
        );

        close(clientSocket);
    }

    close(serverSocket);

    return 0;
}