#include <stdio.h>
#include "TCP_Server.h"

static void do_retransmit(const int sock)
{
    int len;
    char rx_buffer[256];

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(SERVER_TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(SERVER_TAG, "Connection closed");
        } else {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(SERVER_TAG, "Received %d bytes: %s", len, rx_buffer);

            /**Processar comandos AT**/
            int return_index = 0;
            int error = 0;
            enum input_type num_code = checkInput(rx_buffer, len, &return_index);

            switch (num_code)
            {
            case Reverse_Cmd:
                //printf("AT+REVERSE=\n");
                ESP_LOGI(SERVER_TAG, "AT+REVERSE");
                char* output = calloc(len-11, sizeof(char));
                reverse_str(rx_buffer, return_index, len, output, &error);
                if(error)
                {
                    ESP_LOGE(SERVER_TAG, "Erro no input");
                }
                else
                {
                    ESP_LOGI(SERVER_TAG, "Entrada invertida = %s.", output);
                }
                free(output);
                break;
            case Size_Cmd:
                ESP_LOGI(SERVER_TAG, "AT+SIZE");
                size_t input_size = size(rx_buffer, return_index, len, &error);
                if(error)
                {
                    ESP_LOGE(SERVER_TAG, "Erro no input");
                }
                else
                {
                    ESP_LOGI(SERVER_TAG, "String de tamanho = %zu.\n", input_size);
                }
                break;
            case Mult_Cmd:
                ESP_LOGI(SERVER_TAG, "AT+MULT");
                int result = mult(rx_buffer, return_index, len, &error);
                if(error)
                {
                    ESP_LOGE(SERVER_TAG, "Erro no input");
                }
                else
                {
                    ESP_LOGI(SERVER_TAG, "Resultado da multiplicacao = %d.\n", result);
                }
                break;    
            default:
                ESP_LOGE(SERVER_TAG, "Erro no input");
                break;
            }

            // send() can return less bytes than supplied length.
            // Walk-around for robust implementation.
            int to_write = len;
            while (to_write > 0) {
                int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
                if (written < 0) {
                    ESP_LOGE(SERVER_TAG, "Error occurred during sending: errno %d", errno);
                }
                to_write -= written;
            }
        }
    } while (len > 0);
}

void tcp_server_task()
{
    char addr_str[128];
    int addr_family = AF_INET;//(int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }
#ifdef CONFIG_EXAMPLE_IPV6
    else if (addr_family == AF_INET6) {
        struct sockaddr_in6 *dest_addr_ip6 = (struct sockaddr_in6 *)&dest_addr;
        bzero(&dest_addr_ip6->sin6_addr.un, sizeof(dest_addr_ip6->sin6_addr.un));
        dest_addr_ip6->sin6_family = AF_INET6;
        dest_addr_ip6->sin6_port = htons(PORT);
        ip_protocol = IPPROTO_IPV6;
    }
#endif

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(SERVER_TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
#endif

    ESP_LOGI(SERVER_TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(SERVER_TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(SERVER_TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(SERVER_TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 5); //ate 5 conexoes
    if (err != 0) {
        ESP_LOGE(SERVER_TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {

        ESP_LOGI(SERVER_TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(SERVER_TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
#ifdef CONFIG_EXAMPLE_IPV6
        else if (source_addr.ss_family == PF_INET6) {
            inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
#endif
        ESP_LOGI(SERVER_TAG, "Socket accepted ip address: %s", addr_str);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}