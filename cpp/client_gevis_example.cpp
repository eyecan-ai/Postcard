#include <algorithm>
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/tcp.h>
#include <opencv2/opencv.hpp>
#include <stdio.h>  /* for printf() and fprintf() */
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <string>
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <sys/time.h>
#include <unistd.h> /* for close() */

#include "postcard.hpp"

void die(std::string errorMessage)
{
  std::cout << errorMessage << std::endl;
  exit(1);
}

int main(int argc, char *argv[])
{

  /* Socket parameters */
  int sock;                         /* Socket descriptor */
  struct sockaddr_in echoServAddr;  /* Echo server address */
  unsigned short serverPort = 8000; /* Echo server port */

  if (argc < 4)
  {
    std::cout << "Not enought arguments..." << std::endl;
    exit(1);
  }

  postcard::PostcardClient client(argv[1], atoi(argv[2]));

  /* Display window */
  cv::namedWindow("image", cv::WINDOW_NORMAL);
  cv::namedWindow("response", cv::WINDOW_NORMAL);

  /* loading images and merge channels */
  std::cout << "Loading Images:" << std::endl;

  cv::Mat img0 = cv::imread(std::string(argv[3]) + "/" + argv[4] + "_imgFrontale.bmp", cv::IMREAD_GRAYSCALE);
  cv::Mat img1 = cv::imread(std::string(argv[3]) + "/" + argv[4] + "_imgRadente1.bmp", cv::IMREAD_GRAYSCALE);
  cv::Mat img2 = cv::imread(std::string(argv[3]) + "/" + argv[4] + "_imgRadente2.bmp", cv::IMREAD_GRAYSCALE);

  cv::Mat img;
  std::vector<cv::Mat> channels;
  channels.push_back(img0);
  channels.push_back(img1);
  channels.push_back(img2);
  cv::merge(channels, img);

  /* resize to match CNN input */
  cv::resize(img, img, cv::Size(512, 1315));

  cv::imshow("image", img);
  cv::waitKey(0);

  /* Sends the image to the server  */
  std::cout << "Sending data..." << std::endl;
  client.sendImage(img, "sample_command");

  /* Wait&Receive a response Header from the server */
  postcard::MessageHeader responseHeader;
  client.receiveHeader(responseHeader);

  /* Header out */
  std::cout << "Received Header: " << std::endl;
  responseHeader.print();

  /* Receive 'payload_size' bytes from the server */
  if (responseHeader.getPayloadSize() > 0)
  {
    std::cout << "Waiting for " << responseHeader.getPayloadSize()
              << " bytes..." << std::endl;
    cv::Mat response_image = client.receiveImage(responseHeader);

    float alpha = 0.5;
    float beta = 1.0 - alpha;
    cv::Mat blend;
    cv::addWeighted(img, alpha, response_image, beta, 0.0, blend);

    std::cout << response_image.rows << "X" << response_image.cols << "\n";
    cv::imshow("response", response_image);
    cv::imshow("blend", blend);
    cv::waitKey(0);
    img = response_image;

    close(sock);
    exit(0);
  }
}