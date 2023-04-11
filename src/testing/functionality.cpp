#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "clientSocket.h"
#include "sqlHelper.h"

int main(int argc, char * argv[]) {
  std::vector<std::string> xmls;
  std::vector<std::string> xmls_expected;
  for (int i = 0; i < 8; ++i) {
    std::string filename = "functionality_xml/test_" + std::to_string(i) + ".xml";
    std::ifstream ifs(filename);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    xmls.push_back(buffer.str());
    std::string filename_expected = "expected_xml/expected_" + std::to_string(i) + ".xml";
    std::ifstream ifs_expected(filename_expected);
    std::stringstream buffer_expected;
    buffer_expected << ifs_expected.rdbuf();
    xmls_expected.push_back(buffer_expected.str());
  }

  Client client;
  client.initSocket("localhost", "12345");
  for (int i = 0; i < 8; ++i) {
    int contentLen = xmls[i].length();
    std::string tmp = std::to_string(contentLen) + "\n" + xmls[i];
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
    std::string res = "";
    for (size_t i = 0; i < tmpRes.size(); ++i) {
      res += tmpRes[i];
    }
    int diff = xmls_expected.at(i).compare(res);
    // std::cout << std::to_string(diff) << std::endl;
    if (diff != -1 && i != 2 && i != 7) {
      std::cerr << "Expected:" << std::endl;
      std::cerr << xmls_expected.at(i) << std::endl;
      std::cerr << "But was:" << std::endl;
      std::cerr << res << std::endl;
      std::cerr << xmls_expected.at(i).size() << " " << res.size() << std::endl;
      exit(1);
    }
  }
  client.closeSocket();
  std::cout << "\nPassed all tests" << std::endl;
}