#include "obd.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include "nds.h"
#include <dswifi9.h>
#include "string.h"
#include "stdio.h"
#include <sys/types.h>

int send_command(OBDSocket socket_fd, const char *cmd)
{
    snprintf(OBD_BUF, sizeof(OBD_BUF), "%s\r", cmd); // Append carriage return
    int len = strlen(OBD_BUF);
    int sent = send(socket_fd, OBD_BUF, len, 0);
    if (sent != len)
    {
        perror("send");
        return -1;
    }
    return 0;
}

int receive_response(OBDSocket socket_fd, char *buffer, int buffer_size)
{
    int total_received = 0;
    while (total_received < BUF_LEN - 1)
    {
        int received = recv(socket_fd, buffer + total_received, BUF_LEN - 1 - total_received, 0);
        if (received < 0)
        {
            perror("recv");
            return -1;
        }
        else if (received == 0)
        {
            // Connection closed
            break;
        }
        total_received += received;
        // Check for the ELM327 prompt '>'
        if (buffer[total_received - 1] == '>')
        {
            break;
        }
        // yield_thread();  // Yield control to other threads
    }
    buffer[total_received] = '\0'; // Null-terminate the string
    return total_received;
}

void waitFrames(int frames)
{
    for (size_t i = 0; i < frames; i++)
    {
        swiWaitForVBlank();
    }
}

int initELM(OBDSocket socket_fd)
{
    int ret = send_command(socket_fd, "AT Z");
    if (ret < 0)
    {
        fprintf(stderr, "Failed to send AT Z command\n");
        return -1;
    }
    ret = receive_response(socket_fd, OBD_BUF, BUF_LEN);
    if (ret < 0)
    {
        fprintf(stderr, "Failed to receive response for AT Z\n");
        return -1;
    }

    // set all to default
    ret = send_command(socket_fd, "AT D");
    // yield_thread();
    ret = receive_response(socket_fd, OBD_BUF, BUF_LEN);
    printf("Response to AT D: %s\n", OBD_BUF);



    ret = send_command(socket_fd, "AT E0");
    // yield_thread();
    ret = receive_response(socket_fd, OBD_BUF, BUF_LEN);
    printf("Response to AT E0: %s\n", OBD_BUF);

    // Turn off linefeeds
    ret = send_command(socket_fd, "AT L0");
    // yield_thread();
    ret = receive_response(socket_fd, OBD_BUF, BUF_LEN);
    printf("Response to AT L0: %s\n", OBD_BUF);

    // Turn on long messages
    ret = send_command(socket_fd, "AT L0");
    // yield_thread();
    ret = receive_response(socket_fd, OBD_BUF, BUF_LEN);
    printf("Response to AT L0: %s\n", OBD_BUF);

    // Turn on headers to get full responses
    ret = send_command(socket_fd, "AT H1");
    // yield_thread();
    ret = receive_response(socket_fd, OBD_BUF, BUF_LEN);
    printf("Response to AT H1: %s\n", OBD_BUF);

    // Set protocol to automatic search
    ret = send_command(socket_fd, "AT SP 0");
    // yield_thread();
    ret = receive_response(socket_fd, OBD_BUF, BUF_LEN);
    printf("Response to AT SP 0: %s\n", OBD_BUF);

    // Send an OBD-II command to retrieve data (e.g., engine RPM)
    ret = send_command(socket_fd, "01 0C");
    // yield_thread();
    ret = receive_response(socket_fd, OBD_BUF, BUF_LEN);
    printf("Response to 01 0C: %s\n", OBD_BUF);

    // Handle 'SEARCHING...' response
    if (strstr(OBD_BUF, "SEARCHING"))
    {
        // Continue receiving until '>' prompt is detected
        while (!strchr(OBD_BUF, '>'))
        {
            // yield_thread();
            ret = receive_response(socket_fd, OBD_BUF + strlen(OBD_BUF), sizeof(OBD_BUF) - strlen(OBD_BUF));
            if (ret < 0)
            {
                fprintf(stderr, "Failed to receive response during protocol search\n");
                return -1;
            }
        }
        printf("Complete response after searching: %s\n", OBD_BUF);
    }

    return 0;
}

// reads vehicle rpm and prints it
void testELM(OBDSocket socket_fd, char *buffer)
{

    int ret = send_command(socket_fd, "01 0C");
    // yield_thread();
    ret = receive_response(socket_fd, buffer, sizeof(buffer));

    printf("Data recieved: %d\n", ret);
    printf("%s\n", buffer);



    if (strstr(buffer, "NO DATA"))
    {
        fprintf(stderr, "No data received for 01 0C command\n");
    }
    else
    {
        // Process the response to extract RPM value
        // Expected response format: "41 0C A B"
        char *line = strtok(buffer, "\r\n");
        while (line != NULL)
        {
            if (strstr(line, "41 0C"))
            {
                // Parse the response
                char *tokens[5];
                int i = 0;
                char *token = strtok(line, " ");
                while (token != NULL && i < 5)
                {
                    tokens[i++] = token;
                    token = strtok(NULL, " ");
                }
                if (i >= 4)
                {
                    // tokens[2] and tokens[3] are data bytes A and B
                    int A = (int)strtol(tokens[2], NULL, 16);
                    int B = (int)strtol(tokens[3], NULL, 16);
                    int rpm = ((A * 256) + B) / 4;
                    printf("Engine RPM: %d\n", rpm);
                }
                else
                {
                    fprintf(stderr, "Incomplete data received\n");
                }
                break;
            }
            line = strtok(NULL, "\r\n");
        }
    }
}

// Function to initialize the ELM327 Connection.
OBDSocket initConnection()
{

    // init wifi with default settings
    if (!Wifi_InitDefault(WFC_CONNECT))
    {
        fprintf(stderr, "Can't connect to WiFi!\n");
        return -1;
    }

    // create tcp socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        fprintf(stderr, "failed to create socket.");
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
        fprintf(stderr, "failed to connect socket.");
        close(s);
        return -1;
    }

    return s;
}