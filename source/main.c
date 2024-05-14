// SPDX-License-Identifier: CC0-1.0
//
// SPDX-FileContributor: Antonio Niño Díaz, 2024

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <nds.h>
#include <dswifi9.h>

// This function sends an HTTP request to the specified URL and prints the
// response from the server.
// void getHttp(const char *url, const char *path)
// {
//     static char request_text[1024];

//     snprintf(request_text, sizeof(request_text),
//         "GET %s HTTP/1.1\r\n"
//         "Host: %s\r\n"
//         "User-Agent: Nintendo DS\r\n\r\n",
//         path, url);

//     // Get host information, including the IP address
//     struct hostent *myhost = gethostbyname(url);
//     if (myhost == NULL)
//     {
//         perror("gethostbyname()");
//         return;
//     }

//     unsigned long ip = *((unsigned long *)(myhost->h_addr_list[0]));

//     printf("IP Address: %ld.%ld.%ld.%ld\n", (ip >> 0) & 0xFF, (ip >> 8) & 0xFF,
//            (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);

//     // Create a TCP socket

//     printf("Created Socket!\n");

//     printf("Connected to server!\n");

//     // send our request
//     if (send(my_socket, request_text, strlen(request_text), 0) == -1)
//     {
//         perror("send()");
//         close(my_socket);
//         return;
//     }

//     printf("Sent our request!\n");

//     // Print incoming data
//     printf("Printing incoming data:\n");

//     // Put the socket in non-blocking mode:
//     int opt = 1;
//     int rc = ioctl(my_socket, FIONBIO, (char *)&opt);
//     if (rc < 0)
//     {
//         perror("ioctl()");
//         close(my_socket);
//         return;
//     }

//     // 512 KB buffer for the received website
//     static char response_buffer[512 * 1024];
//     response_buffer[0] = '\0';
//     int response_buffer_ptr = 0;

//     // Used to know when to stop receiving data
//     int content_length = -1;
//     int content_end_ptr = -1;

//     // Read at most 100 bytes per call so that we can exercise the logic.
//     const int chunk_size = 100;

//     while (1)
//     {
//         int recvd_len = recv(my_socket, &(response_buffer[response_buffer_ptr]),
//                              chunk_size, 0);

//         if (recvd_len > 0)
//         {
//             printf("Received %d bytes\n", recvd_len);
//             // Some data has been received
//             response_buffer_ptr += recvd_len;
//             response_buffer[response_buffer_ptr] = '\0'; // NULL-terminate
//         }
//         else if (recvd_len == 0)
//         {
//             // The socket has been closed.
//             printf("Other side closed connection!\n");
//             break;
//         }

//         // Try to determine the length of the response
//         if (content_length == -1)
//         {
//             const char *searchstr = "Content-Length: ";
//             char *start = strstr(response_buffer, searchstr);
//             if (start != NULL)
//             {
//                 start += strlen(searchstr);
//                 int tmp = -1;
//                 if (sscanf(start, "%d\r\n", &tmp) == 1)
//                 {
//                     content_length = tmp;
//                     printf("Read %s%d\n", searchstr, content_length);
//                 }
//             }
//         }

//         // Try to determine the end of the content
//         if ((content_length != -1) && (content_end_ptr == -1))
//         {
//             // The start of the content comes after an empty line
//             const char *searchstr = "\r\n\r\n";
//             char *start = strstr(response_buffer, searchstr);
//             if (start != NULL)
//             {
//                 int content_start_ptr = start - &response_buffer[0] + strlen(searchstr);
//                 content_end_ptr = content_start_ptr + content_length;
//                 printf("Determined end of the content\n");
//             }
//         }

//         // If we know the end of the message, check if we have reached it
//         if (content_end_ptr != -1)
//         {
//             if (content_end_ptr <= response_buffer_ptr)
//             {
//                 printf("Reached end of the message\n");
//                 break;
//             }
//         }

//         if (response_buffer_ptr + chunk_size >= sizeof(response_buffer))
//         {
//             printf("Website too big!\n");
//             break;
//         }
//     }

//     // It's good practice to shutdown the socket.
//     if (shutdown(my_socket, 0) != 0)
//     {
//         perror("shutdown()");
//     }

//     // Remove the socket.
//     if (closesocket(my_socket) != 0)
//     {
//         perror("closesocket()");
//     }

//     // Print response (clamp it if it's too long)
//     printf("----------\n");
//     if (response_buffer_ptr > 500)
//         printf("%.*s [...]\n", 500, response_buffer);
//     else
//         printf("%s\n", response_buffer);
//     printf("---------- (%d bytes)\n", response_buffer_ptr);

//     printf("Website: %s%s", url, path);
// }

// dumps recv values
void dumpSockOutput(int s)
{
    char* junkbuf[128];
    recv(s, junkbuf, sizeof(junkbuf), 0);
}

int initOBD()
{

    // init wifi with default settings
    if (!Wifi_InitDefault(WFC_CONNECT))
    {
        printf("Can't connect to WiFi!\n");
        return -1;
    }

    // create tcp socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        printf("failed to create socket.");
        return -1;
    }

    // Tell the socket to connect to the IP address of the OBD reader
    struct sockaddr_in sain;
    sain.sin_family = AF_INET;
    sain.sin_port = htons(35000);
    // this is the ip 192.168.0.10, its in this format u8.u8.u8.u8, it is reversed too, C0 is 192.
    sain.sin_addr.s_addr = 0x0A00A8C0UL;

    // print out the ip
    unsigned long ip = 0x0A00A8C0UL;
    printf("IP Address: %ld.%ld.%ld.%ld\n", (ip >> 0) & 0xFF, (ip >> 8) & 0xFF,
                                            (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);

    if (connect(s, (struct sockaddr *)&sain, sizeof(sain)) == -1)
    {
        printf("failed to connect socket.");
        close(s);
        return -1;
    }

    

    // reset the OBD reader
    send(s, "ATZ\r", 5, 0);

    // wait for 3 secs for it to reset
    for (size_t i = 0; i < 60 * 3; i++)
    {
        swiWaitForVBlank();
    }

    // buffer for junk, output to this should not be used
    dumpSockOutput(s);

    // set elm327 to default settings
    send(s, "ATD\r", 5, 0);
    dumpSockOutput(s);

    send(s, "ATE0\r", 6, 0);
    dumpSockOutput(s);

    send(s, "ATAL\r", 6, 0);
    dumpSockOutput(s);

    send(s, "ATSP0\r", 7, 0);
    dumpSockOutput(s);

    send(s, "0100\r", 6, 0);
    dumpSockOutput(s);

    return s;
}



int main(int argc, char *argv[])
{
    consoleDemoInit();

    printf("Connecting to WiFi using\n");
    printf("firmware settings...\n");

    int s = initOBD();

    if (s == -1)
    {
        printf("failed to connect to OBD");
        return -1;
    }

    printf("\n");
    printf("Press START to exit");

    while (1)
    {
        swiWaitForVBlank();
        scanKeys();
        if (keysHeld() & KEY_START)
            break;
    }

    return 0;
}
