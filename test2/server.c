#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <sqlite3.h>
#include "base64.h" // 需要libb64库

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define VERSION "1.0"
#define DB_PATH "server.db"

// 初始化Winsock
void initializeWinsock() {
    WSADATA wsa;
    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        exit(1);
    }
}


// 创建客户端套接字
void createServerSocket(SOCKET *s) {
    if ((*s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        exit(1);
    }
    printf("Socket created.\n");
}


void bindAndListen(SOCKET *s) {
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(*s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code: %d\n", WSAGetLastError());
        exit(1);
    }
    printf("Bind done.\n");

    listen(*s, 3);
    printf("Waiting for incoming connections...\n");
}

void createDatabase() {
    sqlite3 *db;
    char *err_msg = 0;

    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char *sql = "CREATE TABLE IF NOT EXISTS Data(Id INTEGER PRIMARY KEY, Info TEXT);";
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(1);
    }

    sqlite3_close(db);
}

void storeData(const char *encodedData) {
    sqlite3 *db;
    sqlite3_open(DB_PATH, &db);

    char *err_msg = 0;
    char *decodedData = (char *)malloc(Base64decode_len(encodedData));
    Base64decode(decodedData, encodedData);

    char sql[512];
    snprintf(sql, sizeof(sql), "INSERT INTO Data(Info) VALUES('%s');", decodedData);

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    sqlite3_close(db);
    free(decodedData);
}

// 处理客户端连接的函数
void handleClient(SOCKET *new_socket) {
    char client_message[2000];
    int recv_size;

    // 版本检查
    recv_size = recv(*new_socket, client_message, 2000, 0);
    if (recv_size == SOCKET_ERROR) {
        printf("recv failed\n");
    }
    client_message[recv_size] = '\0';

    if (strcmp(client_message, "VERSION_CHECK") == 0) {
        send(*new_socket, VERSION, strlen(VERSION), 0);
    } else {
        storeData(client_message);
        send(*new_socket, "Data received and stored", 25, 0);
    }
    closesocket(*new_socket);
}

// 接受客户端连接的函数
void acceptConnections(SOCKET *s) {
    struct sockaddr_in client;
    int c = sizeof(struct sockaddr_in);
    SOCKET new_socket;

    while ((new_socket = accept(*s, (struct sockaddr *)&client, &c)) != INVALID_SOCKET) {
        printf("Connection accepted.\n");
        handleClient(&new_socket);
    }

    if (new_socket == INVALID_SOCKET) {
        printf("accept failed with error code: %d\n", WSAGetLastError());
    }
}

int main() {
    SOCKET s;  // 定义一个套接字变量s
    initializeWinsock();  // 初始化Winsock库
    createServerSocket(&s);
    bindAndListen(&s);
    createDatabase();
    acceptConnections(&s);

    closesocket(s);
    WSACleanup();

    return 0;
}
