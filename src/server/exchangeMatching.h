#ifndef __EXCHANGEMATCHING_H__
#define __EXCHANGEMATCHING_H__
#include <memory>
#include <thread>

#include "XMLParser.h"
#include "ctpl/ctpl_stl.h"
#include "query_funcs.h"
#include "server.h"

#define PORT "12345"
#define SQL_FILE "createTables.sql"

using namespace std;

class ExchangeMatching {
 private:
  Query_funcs query_funcs;
  Server serverSocket;

 public:
  static size_t threadpoolsize;
  ExchangeMatching(Query_funcs q, Server s);
  void run();

 private:
  void initTables();
  static void handleRequest(int i, int client_fd, Query_funcs q);
};
#endif