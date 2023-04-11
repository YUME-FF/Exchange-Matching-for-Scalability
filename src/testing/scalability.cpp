#include <pthread.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>

#include "clientSocket.h"
#include "sqlHelper.h"

typedef struct {
  std::vector<std::string> xmls_files;
  int epoch;
  const char * hostname;
  const char * port;
} scalability_args;

void * run(void * args) {
  scalability_args * myArgs = (scalability_args *)args;
  std::vector<std::string> xmls = myArgs->xmls_files;
  int epoch = myArgs->epoch;
  const char * hostname = myArgs->hostname;
  const char * port = myArgs->port;
  Client client;
  client.initSocket(hostname, port);
  for (int i = 0; i < epoch; ++i) {
    for (std::size_t j = 0; j < xmls.size(); ++j) {
      int contentLen = xmls[j].length();
      std::string tmp = std::to_string(contentLen) + "\n" + xmls[j];
      // std::cout << "Client send:\n" + tmp << std::endl;
      client.sendResponse(strToVec(tmp));

      auto response = client.toReceiveXML();
      // size_t idx = findCharInSubStr(response.second, strToVec("\r\n"));
      // while (idx < response.second.size() &&
      //        (response.second[idx] == '\r' || response.second[idx] == '\n')) {
      //   ++idx;
      // }
      // std::vector<char> tmpRes =
      //     std::vector<char>(response.second.begin() + idx, response.second.end());
      // for (size_t i = 0; i < tmpRes.size(); ++i) {
      //   std::cout << tmpRes[i];
      // }
    }
  }
  client.closeSocket();
  pthread_exit(nullptr);
}

int main(int argc, char * argv[]) {
  if (argc != 5) {
    std::cerr
        << "Usage: ./scalability_test.out <#Num of thread> <#epoch> <hostname> <port>"
        << std::endl;
    return EXIT_FAILURE;
  }
  int num_threads = std::stoi(argv[1]);
  int epoch = std::stoi(argv[2]);

  const char * hostname = argv[3];
  const char * port = argv[4];

  std::vector<std::string> xmls;
  for (int i = 0; i < 1; ++i) {
    std::string filename = "xml/test_" + std::to_string(i) + ".xml";
    std::ifstream ifs(filename);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    xmls.emplace_back(buffer.str());
    std::cout << "Get file: " + filename << std::endl;
  }

  scalability_args args = {xmls, epoch, hostname, port};

  std::vector<pthread_t> threads(num_threads, 0);

  float total_time = 0;
  float total_time_ms = 0;
  int num_runs = 1;  // change this to the number of times you want to run the loop
  for (int j = 0; j < num_runs; ++j) {
    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_threads; ++i) {
      std::cout << "Creating thread: " + i << std::endl;
      int rc = pthread_create(&threads[i], nullptr, run, (void *)&args);
      if (rc != 0) {
        std::cerr << "failed to create thread!" << std::endl;
        exit(EXIT_FAILURE);
      }
    }

    std::cout << "Joining thread" << std::endl;
    for (int i = 0; i < num_threads; ++i) {
      pthread_join(threads[i], nullptr);
    }
    std::cout << "After Joining thread" << std::endl;

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> fs = end_time - start_time;
    std::chrono::milliseconds d =
        std::chrono::duration_cast<std::chrono::milliseconds>(fs);
    std::cout << "Seconds: " << fs.count() << "s" << std::endl;
    std::cout << "Or in ms: " << d.count() << "ms" << std::endl;
    total_time += fs.count();
    total_time_ms += d.count();
  }

  float avg_time = total_time / num_runs;
  float avg_time_ms = total_time_ms / num_runs;
  std::cout << "Average time taken: " << avg_time << "s" << std::endl;
  std::cout << "Average time taken in ms: " << avg_time_ms << "ms" << std::endl;
  std::cout << "Average Throughput: " << num_threads * epoch / avg_time_ms
            << " requests/ms" << std::endl;
}