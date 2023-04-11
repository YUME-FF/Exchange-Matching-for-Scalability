#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "clientSocket.h"
#include "sqlHelper.h"

int main(int argc, char * argv[]) {
  std::vector<std::string> xmls;
  for (int i = 2; i < argc; ++i) {
    std::ifstream ifs(argv[i]);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    xmls.emplace_back(buffer.str());
  }
  Client client;
  client.initSocket(argv[1], "12345");
  for (int i = 2; i < argc; ++i) {
    int contentLen = xmls[i - 2].length();
    std::string tmp = std::to_string(contentLen) + "\n" + xmls[i - 2];
    std::cout << "Client send:\n" + tmp << std::endl;
    client.sendResponse(strToVec(tmp));
    auto response = client.toReceiveXML();
    size_t idx = findCharInSubStr(response.second, strToVec("\r\n"));
    while (idx < response.second.size() &&
           (response.second[idx] == '\r' || response.second[idx] == '\n')) {
      ++idx;
    }
    std::vector<char> tmpRes =
        std::vector<char>(response.second.begin() + idx, response.second.end());
    for (size_t i = 0; i < tmpRes.size(); ++i) {
      std::cout << tmpRes[i];
    }
  }
}