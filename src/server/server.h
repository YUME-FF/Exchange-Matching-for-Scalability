#ifndef __SERVER_H__
#define __SERVER_H__
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>

#include "myexception.h"
#include "sqlHelper.h"

class Server {
 public:
  int socket_fd;             // this is the listen socket_fd
  int client_connection_fd;  // socket fd on the server side after accept

  Server() {}
  Server(int socket_fd, int client_connection_fd) :
      socket_fd(socket_fd), client_connection_fd(client_connection_fd) {}

  /**
    * init socket status, bind socket, listen on socket as a server(to accept connection from browser)
    * @param port
    */
  void initListenfd(const char * port) {
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

    socket_fd = socket(host_info_list->ai_family,
                       host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
      std::string msg = "Error: cannot create socket\n";
      msg = msg + "  (" + "" + "," + port + ")";
      throw myException(msg);
    }

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
      std::string msg = "Error: cannot bind socket\n";
      msg = msg + "  (" + "" + "," + port + ")";
      throw myException(msg);
    }  // if

    status = listen(socket_fd, 100);
    if (status == -1) {
      std::string msg = "Error: cannot listen on socket\n";
      msg = msg + "  (" + "" + "," + port + ")";
      throw myException(msg);
    }  // if

    freeaddrinfo(host_info_list);
  }

  /**
    * accept connection on socket, using IPv4 only
    * @param ip
    */
  void acceptConnection() {
    struct sockaddr_storage socket_addr;
    char str[INET_ADDRSTRLEN];
    socklen_t socket_addr_len = sizeof(socket_addr);
    client_connection_fd =
        accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
      std::string msg = "Error: cannot accept connection on socket";
      throw myException(msg);
    }  // if

    // only use IPv4
    struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
    inet_ntop(socket_addr.ss_family,
              &(((struct sockaddr_in *)addr)->sin_addr),
              str,
              INET_ADDRSTRLEN);
  }

  static std::vector<char> getXML(int client_fd) {
    std::vector<char> buffer(1024);
    int bufferLen = recv(client_fd, buffer.data(), buffer.size(), 0);
    if (bufferLen < 0) {
      throw myException("fail to receive client request");
    }
    else if (bufferLen == 0) {
      return std::vector<char>{};
    }

    size_t idx = findCharInSubStr(buffer, strToVec("\r\n"));
    std::string lenStr = std::string(buffer.begin(), buffer.begin() + idx);

    int targetLen = stoi(lenStr);
    while (idx < buffer.size() && (buffer[idx] == '\r' || buffer[idx] == '\n')) {
      ++idx;
    }
    targetLen = idx + targetLen;
    buffer.resize(targetLen);
    int curLen = bufferLen;

    while (curLen < targetLen) {
      bufferLen = recv(client_fd, &(buffer.data()[curLen]), targetLen - curLen, 0);
      if (bufferLen < 0) {
        throw myException("fail to receive client request");
      }
      curLen += bufferLen;
    }
    return buffer;
  }

  static void sendResponse(int client_fd, std::vector<char> response) {
    int status = send(client_fd, response.data(), response.size(), 0);
    if (status == -1) {
      throw myException("Fail to send response");
    }
  }
};
#endif