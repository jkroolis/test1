#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include "base64.h" // 需要libb64库

#pragma comment(lib, "ws2_32.lib")  // 链接 ws2_32.lib 库，用于使用 Winsock 相关函数

#define SERVER "127.0.0.1"  // 定义服务器 IP 地址
#define PORT 8080

// 初始化 Winsock
void initializeWinsock() {
    WSADATA wsa;
    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        exit(1);
    }
}

// 创建客户端套接字
void createClientSocket(SOCKET *s) {
    if ((*s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        exit(1);
    }
    printf("Socket created.\n");
}

void connectToServer(SOCKET *s) {
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(SERVER);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(*s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connection error\n");
        exit(1);
    }
    printf("Connected to server\n");
}

void checkVersion(SOCKET *s) {
    char *message = "VERSION_CHECK";
    char server_reply[2000];
    int recv_size;

    send(*s, message, strlen(message), 0);
    recv_size = recv(*s, server_reply, 2000, 0);
    server_reply[recv_size] = '\0';

    if (strcmp(server_reply, "1.0") != 0) {
        printf("New version available: %s. Downloading...\n", server_reply);
        // 下载新版本逻辑可以放在这里
    } else {
        printf("Client is up-to-date.\n");
    }
}

// 上传数据的函数，将数据进行Base64编码后发送给服务器
void uploadData(SOCKET *s, const char *data) {
    char encodedData[2000];
    Base64encode(encodedData, data, strlen(data));

    send(*s, encodedData, strlen(encodedData), 0);

    char server_reply[2000];
    int recv_size = recv(*s, server_reply, 2000, 0);
    server_reply[recv_size] = '\0';

    printf("Server reply: %s\n", server_reply);
}

// 工作线程函数，用于处理上传数据的任务
unsigned __stdcall workerThread(void *param) {
    SOCKET *s = (SOCKET *)param;
    char data[] = "This is the data to upload";

    uploadData(s, data);

    return 0;
}

// 启动客户端线程的函数
void startClientThreads() {
    SOCKET s;
    initializeWinsock();
    createClientSocket(&s);
    connectToServer(&s);

    checkVersion(&s);

    HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, &workerThread, &s, 0, NULL);
    if (thread == 0) {
        printf("Thread creation failed\n");
    } else {
        printf("Thread created\n");
    }

    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);

    closesocket(s);
    WSACleanup();
}

// 定义窗口过程函数，用于处理窗口消息
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                startClientThreads();
            }
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// 主函数，程序入口点
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(      // 创建窗口
        0,
        CLASS_NAME,
        L"Client Application",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    HWND hwndButton = CreateWindow(         // 创建按钮控件
        L"BUTTON",
        L"Start Client",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10,
        10,
        150,
        30,
        hwnd,
        (HMENU)1,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL
    );

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;   //程序正常结束
}
