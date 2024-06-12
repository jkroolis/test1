#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "base64.h"

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void store_data(const char* data) {
    // 实现数据存储逻辑，这里只是一个简单示例
    printf("Storing data: %s\n", data);
    // 这里可以根据需要将数据存储到文件或SQLite数据库中
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    // 创建TCP套接字
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    // 绑定地址
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听连接
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", SERVER_PORT);

    // 接受客户端连接
    client_addr_len = sizeof(client_addr);
    client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_socket == -1) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // 接收数据并存储
    int bytes_received;
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        // 解码Base64数据
        // 这里需要根据具体的Base64编码库进行解码
        // 假设base64_decode函数是解码函数
        char* decoded_data = base64_decode(buffer);
        if (decoded_data != NULL) {
            // 存储数据
            store_data(decoded_data);
            free(decoded_data);
        }
        else {
            printf("Base64 decoding failed\n");
        }
    }

    if (bytes_received == -1) {
        perror("Receive failed");
        exit(EXIT_FAILURE);
    }

    // 关闭套接字
    close(client_socket);
    close(server_socket);

    return 0;
}
