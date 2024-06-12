#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// Base64编码表
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// 对数据进行Base64编码
char* base64_encode(const unsigned char* data, size_t input_length) {
    // 计算编码后数据的长度
    size_t output_length = 4 * ((input_length + 2) / 3);

    // 分配内存存储编码后的数据，记得释放内存
    char* encoded_data = (char*)malloc(output_length + 1);
    if (encoded_data == NULL) return NULL;

    // 进行编码
    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = base64_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 0 * 6) & 0x3F];
    }

    // 补充'='
    for (size_t i = 0; i < (3 - input_length % 3) % 3; i++) {
        encoded_data[output_length - 1 - i] = '=';
    }

    encoded_data[output_length] = '\0';
    return encoded_data;
}

// 对Base64编码的数据进行解码
unsigned char* base64_decode(const char* data) {
    // 计算解码后数据的长度
    size_t input_length = strlen(data);
    if (input_length % 4 != 0) return NULL;

    size_t output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') output_length--;
    if (data[input_length - 2] == '=') output_length--;

    // 分配内存存储解码后的数据，记得释放内存
    unsigned char* decoded_data = (unsigned char*)malloc(output_length + 1);
    if (decoded_data == NULL) return NULL;

    // 进行解码
    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : strchr(base64_table, data[i++]) - base64_table;
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : strchr(base64_table, data[i++]) - base64_table;
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : strchr(base64_table, data[i++]) - base64_table;
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : strchr(base64_table, data[i++]) - base64_table;

        uint32_t triple = (sextet_a << 3 * 6)
                        + (sextet_b << 2 * 6)
                        + (sextet_c << 1 * 6)
                        + (sextet_d << 0 * 6);

        if (j < output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    decoded_data[output_length] = '\0';
    return decoded_data;
}
