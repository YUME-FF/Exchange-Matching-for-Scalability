#ifndef __XMLPARSER_H__
#define __XMLPARSER_H__
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>

#include "myexception.h"
#include "pugixml/pugixml.hpp"
#include "query_funcs.h"

/*==When top-level node is create==*/
#define CREATE "create"
#define ACCOUNT "account"
#define ID "id"
#define BALANCE "balance"
#define SYMBOL "symbol"
#define SYM "sym"
/*===============end===============*/

/*==When top-level node is transaction==*/
#define TRANSACTIONS "transactions"
#define ORDER "order"
#define QUERY "query"
#define CANCEL "cancel"
#define AMOUNT "amount"
#define LIMIT "limit"
/*==================end=================*/

/*=======When root node is result=======*/
#define RESULTS "results"
#define CREATED "created"
#define ERROR "error"
#define OPENED "opened"
#define STATUS "status"
#define OPEN "open"
#define CANCELED "canceled"
#define EXECUTED "executed"
#define SHARES "shares"
#define PRICE "price"
#define TIME "time"
/*==================end=================*/

class XMLParser {
 private:
  Query_funcs query_funcs;
  pugi::xml_document request;
  pugi::xml_document response;
  pugi::xml_node resNode;

 public:
  XMLParser(std::vector<char> input, Query_funcs q);
  void parse();
  std::string getRequest();
  std::string getResponse();

 private:
  void create(pugi::xml_node createNode);
  void createAccount(pugi::xml_node accountNode);
  void createPosition(std::string sym, pugi::xml_node symbolChild);
  void processOrder(pugi::xml_node orderChild,string account_id);
  void processCancel(pugi::xml_node cancelChild,string account_id);
  void processQuery(pugi::xml_node queryChild,string account_id);
  void transactions(pugi::xml_node transactionsNode);
};

#endif