#ifndef __CLIENTSOCKET_H__
#define __CLIENTSOCKET_H__
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>

#include "myexception.h"
#include "sqlHelper.h"

class Client {
 public:
  int socket_fd;

  Client() {}

  /**
    * init socket status, bind socket, listen on socket as a server(to accept connection from browser)
    * @param port
    */
  void initSocket(const char * hostname, const char * port) {
    int status;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, port, &host_info, &host_info_list);

    if (status != 0) {
      std::string msg = "Error: cannot get address info for host\n";
      msg = msg + "  (" + "" + "," + port + ")";
      throw myException(msg);
    }

    if (port == "") {
      // OS will assign a port
      struct sockaddr_in * addr_in = (struct sockaddr_in *)(host_info_list->ai_addr);
      addr_in->sin_port = 0;
    }

    socket_fd = socket(host_info_list->ai_family,
                       host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
      std::cerr << "Error: cannot create socket" << std::endl;
      exit(EXIT_FAILURE);
    }  //if

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
      std::cerr << "Error: cannot connect to socket" << std::endl;
      exit(EXIT_FAILURE);
    }  //if
    freeaddrinfo(host_info_list);
  }

  std::vector<char> receiveData(int fd, int len) {
    std::vector<char> buffer(len);
    int curLen = 0;
    while (curLen < len) {
      int bufferLen = recv(fd, &(buffer.data()[curLen]), len - curLen, 0);
      if (bufferLen < 0) {
        throw myException("Fail to receive client request");
      }
      curLen += bufferLen;
    }
    return buffer;
  }

  std::pair<int, std::vector<char> > toReceiveXML() {
    std::vector<char> buffer(1024);
    int bufferLen = recv(socket_fd, buffer.data(), buffer.size(), 0);
    if (bufferLen == -1) {
      throw myException("Fail to receive response");
    }
    std::string tmp = "\r\n";
    int idx = findCharInSubStr(buffer, std::vector<char>(tmp.begin(), tmp.end()));
    std::string lenStr = std::string(buffer.begin(), buffer.begin() + idx);
    int targetLen = stoi(lenStr);
    while (idx < buffer.size() && (buffer[idx] == '\r' || buffer[idx] == '\n')) {
      ++idx;
    }
    targetLen = targetLen + idx;
    buffer.resize(targetLen);
    std::vector<char> data = receiveData(socket_fd, targetLen - bufferLen);
    std::copy(data.begin(), data.end(), buffer.begin() + bufferLen);
    return std::make_pair(buffer.size(), buffer);
  }

  void sendResponse(std::vector<char> response) {
    int status = send(socket_fd, response.data(), response.size(), 0);
    if (status == -1) {
      throw myException("Fail to send response");
    }
  }
};
#endif