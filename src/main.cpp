#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <netinet/in.h>

// This is used to determine the content type based on the file extension.
std::string getContentType(const std::string& path) {
    if (path.size() >= 5 && path.compare(path.size() - 5, 5, ".html") == 0) {
        return "text/html";
    }
    else if (path.size() >= 4 && path.compare(path.size() - 4, 4, ".css") == 0) {
        return "text/css";
    }
    else if (path.size() >= 3 && path.compare(path.size() - 3, 3, ".js") == 0) {
        return "application/javascript";
    }
    else if (path.size() >= 5 && path.compare(path.size() - 5, 5, ".json") == 0) {
        return "application/json";
    }
    else if (path.size() >= 4 && path.compare(path.size() - 4, 4, ".txt") == 0) {
        return "text/plain";
    }

    return "application/octet-stream";
}

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
        std::string method;

        if (bytesReceived > 0) {
            std::cout << "\nReceived request:\n";
            std::cout << buffer << "\n";

            std::istringstream requestStream(buffer);

            std::string requestedPath;
            std::string version;

            requestStream >> method >> requestedPath >> version;

            path = requestedPath;

            std::cout << "Method: " << method << "\n";
            std::cout << "Path: " << path << "\n";

            if (method != "GET" && method != "POST") {
                std::string response =
                    "HTTP/1.1 405 Method Not Allowed\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 22\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "Method Not Allowed";

                send(
                    clientSocket,
                    response.c_str(),
                    response.length(),
                    0
                );

                close(clientSocket);
                continue;
            }
        }

        // Create an HTTP response
        std::string body;
        std::string status = "200 OK";
        std::string contentType = "text/html";

        if (path == "/api/status") {
            contentType = "application/json";

            body = R"({
            "status": "Server is running",
            "message": "Hello from the C++ web server!"
        })";
        }

        else if (path == "/api/message" && method == "POST") {
            contentType = "text/html";

            // Find the beginning of the form data
            std::string request(buffer);
            size_t bodyStart = request.find("\r\n\r\n");

            if (bodyStart != std::string::npos) {
                std::string formData = request.substr(bodyStart + 4);

                // Look for "message="
                size_t messageStart = formData.find("message=");

                if (messageStart != std::string::npos) {
                    std::string message = formData.substr(messageStart + 8);

                    // Basic form decoding
                    size_t position = 0;

                    while ((position = message.find('+', position)) != std::string::npos) {
                        message.replace(position, 1, " ");
                        position++;
                    }

                    body =
                        "<!DOCTYPE html>"
                        "<html>"
                        "<head>"
                        "<title>Message Received</title>"
                        "<link rel=\"stylesheet\" href=\"/style.css\">"
                        "</head>"
                        "<body>"
                        "<h1>Message Received</h1>"
                        "<p>Your message was:</p>"
                        "<p>" + message + "</p>"
                        "<a href=\"/\">Back to Home</a>"
                        "</body>"
                        "</html>";
                }
                else {
                    status = "400 Bad Request";
                    body = "Message field was not found.";
                }
            }
            else {
                status = "400 Bad Request";
                body = "Could not read request body.";
            }
        }

        else {
            // Convert "/" into "/index.html"
            if (path == "/") {
                path = "/index.html";
            }

            // Build the path to the requested file
            std::string filePath = "public" + path;

            std::ifstream file(filePath);

            if (file) {
                std::stringstream contents;
                contents << file.rdbuf();
                body = contents.str();

                contentType = getContentType(filePath);
            }
            else {
                status = "404 Not Found";

                std::ifstream notFoundFile("public/404.html");

                if (notFoundFile) {
                    std::stringstream contents;
                    contents << notFoundFile.rdbuf();
                    body = contents.str();
                }
                else {
                    body = "404 Not Found";
                }
            }
        }

        // Construct the full HTTP response
        std::string response =
            "HTTP/1.1 " + status + "\r\n"
            "Content-Type: " + contentType + "\r\n"
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