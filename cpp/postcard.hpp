#include <cstdint>
#include <iostream>
#include <cstring>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <netinet/tcp.h>
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <string>
#include <unistd.h> /* for close() */
#include <sys/time.h>
#include <algorithm>
#include <opencv2/opencv.hpp>

#define COMMAND_SIZE 32
typedef uint8_t byte;

namespace postcard
{

struct MessageHeader
{
    uint8_t CRC[4];
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t byte_per_element;
    char command[COMMAND_SIZE];

    MessageHeader()
    {
        memset(&command[0], 0, COMMAND_SIZE);
    }

    MessageHeader(std::string cmd, int w, int h, int d, int byte_per_element)
    {
        //Control Code
        this->CRC[0] = 6;
        this->CRC[1] = 66;
        this->CRC[2] = 166;
        this->CRC[3] = 6;

        //Payload Size
        this->width = w;
        this->height = h;
        this->depth = d;
        this->byte_per_element = byte_per_element;

        //Command field
        memset(&command[0], 0, COMMAND_SIZE);
        cmd.copy(this->command, std::min(COMMAND_SIZE, int(cmd.length())));
    }

    long getPayloadSize()
    {
        return width * height * depth * byte_per_element;
    }

    std::string getCommand()
    {
        return std::string(command);
    }

    void print()
    {
        /* Header out */
        std::cout << "Header: " << std::endl;
        std::cout << " W:" << width << std::endl;
        std::cout << " H:" << height << std::endl;
        std::cout << " D:" << depth << std::endl;
        std::cout << " Bytes Per Eleemnt:" << byte_per_element << std::endl;
        std::cout << " Command:" << command << std::endl;
    }
};

class PostcardClient
{

protected:
    /* Socket parameters */
    int sock;                         /* socket handle */
    struct sockaddr_in echoServAddr;  /* server address */
    unsigned short serverPort = 8000; /* server port */
    std::string server_address;       /* Server IP address (dotted quad) */
    unsigned short port;              /* Input Image */

public:
    PostcardClient(std::string server_address, unsigned short port)
    {

        this->server_address = server_address;
        this->port = port;

        /* TCP Socket Creation */
        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            die("socket() initialization failed");

        /* Server Address Structure */
        memset(&echoServAddr, 0, sizeof(echoServAddr));                         /* Zero out structure */
        echoServAddr.sin_family = AF_INET;                                      /* Internet address family */
        echoServAddr.sin_addr.s_addr = inet_addr(this->server_address.c_str()); /* Server IP address */
        echoServAddr.sin_port = htons(this->port);                              /* Server port */

        /* Connection */
        if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
            die("connect() failed");

        /* TCP FAST NO DELAY mode */
        int enable_no_delay = 1;
        if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &enable_no_delay, sizeof(int)) < 0)
            die("TCP_NODELAY failed");

        /* Handshake wait... */
        usleep(10000);
    }

    /**
     * Sends and HEADER through the socket
     */
    void sendHeader(int sock, MessageHeader &header)
    {
        if (send(sock, &header, sizeof(MessageHeader), 0) != sizeof(MessageHeader))
            die("Send Header fails!");
    }

    /**
     * Sends an opencv IMAGE through the socket
     */
    void sendImage(cv::Mat &image, std::string command)
    {

        int size = image.total() * image.elemSize();
        byte *bytes = new byte[size];
        std::memcpy(bytes, image.data, size * sizeof(byte));

        MessageHeader header(command, image.cols, image.rows, image.channels(), 1);

        sendHeader(sock, header);

        if (send(sock, &bytes[0], size, 0) != size)
            die("Send Payload fails!");

        delete[] bytes;
    }

    /**
     * Receives an HEADER through the socket
     */
    void receiveHeader(MessageHeader &header)
    {
        if (recv(sock, &header, sizeof(MessageHeader), 0) != sizeof(MessageHeader))
            die("Receive Data Fails!");
    }

    /**
     * Receives a Generic Payload through the socket.
     */
    void receiveData(int expected_size, byte **output)
    {

        *output = new byte[expected_size];
        int received_size = 0;
        while (received_size < expected_size)
        {
            int remains = expected_size - received_size;
            int chunk_size = recv(sock, &(*output)[received_size], remains, 0);
            received_size += chunk_size;
        }
        std::cout << "Received: " << received_size << std::endl;
    }

    /**
     * Receives Image given the Header
     */
    cv::Mat receiveImage(MessageHeader header)
    {
        byte *received_data;
        this->receiveData(header.getPayloadSize(), &received_data);

        /* Builds an Image with the received bytes */
        cv::Mat response_image(
            header.height,
            header.width,
            header.depth == 1 ? CV_8UC1 : CV_8UC3,
            received_data);
        return response_image;
    }

    /**
     * DIEs
     */
    void die(std::string errorMessage)
    {
        std::cout << errorMessage << std::endl;
        exit(1);
    }
};

}; // namespace postcard
