#include "exchangeMatching.h"

size_t ExchangeMatching::threadpoolsize = 5;

ExchangeMatching::ExchangeMatching(Query_funcs q, Server s) :
    query_funcs(q), serverSocket(s) {
}

void ExchangeMatching::run() {
  initTables();
  serverSocket.initListenfd(PORT);
  //accept request from client
  ctpl::thread_pool p(threadpoolsize);

  while (true) {
    serverSocket.acceptConnection();
    p.push(handleRequest, serverSocket.client_connection_fd, this->query_funcs);
  }
}

void ExchangeMatching::initTables() {
  connection * C = query_funcs.getConnection();
  dropTable("CANCELED", C);
  dropTable("EXECUTED", C);
  dropTable("ORDERS", C);
  dropTable("POSITION", C);
  dropTable("ACCOUNT", C);
  createTable(SQL_FILE, C);
}

void ExchangeMatching::handleRequest(int i, int client_fd, Query_funcs query_funcs) {
  vector<char> buffer;
  while (true) {
    try {
      buffer = Server::getXML(client_fd);
      if (buffer.empty()) {
        cout << "buffer recived is empty" << endl;
        return;
      }
      size_t idx = findCharInSubStr(buffer, strToVec("\r\n"));
      while (idx < buffer.size() && (buffer[idx] == '\r' || buffer[idx] == '\n')) {
        ++idx;
      }

      XMLParser xmlParser(vector<char>(buffer.begin() + idx, buffer.end()), query_funcs);
      cout << "start Parser" << endl;
      xmlParser.parse();
      // cout << "XML parse: \n" + xmlParser.getResponse() << endl;
      Server::sendResponse(client_fd, addHeader(strToVec(xmlParser.getResponse())));
    }
    catch (myException & e) {
      std::cout << e.what();
      break;
    }
  }
}