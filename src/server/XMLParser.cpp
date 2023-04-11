#include "XMLParser.h"

XMLParser::XMLParser(std::vector<char> input, Query_funcs q) {
  cout << "start init XMLParser" << endl;
  query_funcs = q;
  input.emplace_back('\0');
  pugi::xml_parse_result result = request.load_string(input.data());
  if (!result) {
    throw myException("Error parsing XML");
  }
  resNode = response.append_child(RESULTS);
  cout << "end init XMLParser" << endl;
}

std::string XMLParser::getRequest() {
  std::stringstream ss;
  request.save(ss, "    ");
  return ss.str();
}

std::string XMLParser::getResponse() {
  std::stringstream ss;
  response.save(ss, "    ");
  return ss.str();
}

void XMLParser::processOrder(pugi::xml_node orderChild, string account_id) {
  cout << "start process order" << endl;
  std::string sym = orderChild.attribute(SYM).value();
  double amount = stod(orderChild.attribute(AMOUNT).value());
  double limit = stod(orderChild.attribute(LIMIT).value());
  cout << sym << " " << amount << " " << limit << endl;
  // std::string msg = query_funcs.
  std::unordered_map<string, vector<string> > xmlMsgs =
      query_funcs.order(account_id, sym, amount, limit);
  if (xmlMsgs.count(ERROR)) {
    pugi::xml_node errNode = resNode.append_child(ERROR);
    errNode.append_attribute(SYM).set_value(sym.c_str());
    errNode.append_attribute(AMOUNT).set_value(to_string(amount).c_str());
    errNode.append_attribute(LIMIT).set_value(to_string(limit).c_str());
    errNode.text().set(xmlMsgs[ERROR][0].c_str());
  }
  else {
    pugi::xml_node openedNode = resNode.append_child(OPENED);
    openedNode.append_attribute(SYM).set_value(sym.c_str());
    openedNode.append_attribute(AMOUNT).set_value(to_string(amount).c_str());
    openedNode.append_attribute(LIMIT).set_value(to_string(limit).c_str());
    openedNode.append_attribute(ID).set_value(xmlMsgs["unique_id"][0].c_str());
  }
  cout << "end process order" << endl;
}
void XMLParser::processCancel(pugi::xml_node cancelChild, string account_id) {
  std::string unique_id = cancelChild.attribute(ID).value();
  std::unordered_map<string, vector<string> > xmlMsgs = query_funcs.cancel(unique_id);
  if (xmlMsgs.count(ERROR)) {
    pugi::xml_node errNode = resNode.append_child(ERROR);
    errNode.append_attribute(ID).set_value(unique_id.c_str());
    errNode.text().set(xmlMsgs[ERROR][0].c_str());
  }
  else {
    pugi::xml_node canceledRootNode = resNode.append_child(CANCELED);
    canceledRootNode.append_attribute(ID).set_value(unique_id.c_str());
    if (xmlMsgs.count(CANCELED)) {
      pugi::xml_node canceledNode = canceledRootNode.append_child(CANCELED);
      canceledNode.append_attribute(SHARES).set_value(xmlMsgs[CANCELED][0].c_str());
      canceledNode.append_attribute(TIME).set_value(xmlMsgs[CANCELED][1].c_str());
    }
    else if (xmlMsgs.count(EXECUTED)) {
      pugi::xml_node executedNode = canceledRootNode.append_child(EXECUTED);
      executedNode.append_attribute(SHARES).set_value(xmlMsgs[EXECUTED][0].c_str());
      executedNode.append_attribute(PRICE).set_value(xmlMsgs[EXECUTED][1].c_str());
      executedNode.append_attribute(TIME).set_value(xmlMsgs[EXECUTED][2].c_str());
    }
  }
  cout << "start process order" << endl;
}
void XMLParser::processQuery(pugi::xml_node queryChild, string account_id) {
  cout << "startQuery" << endl;
  std::string unique_id = queryChild.attribute(ID).value();

  std::unordered_map<string, vector<string> > xmlMsgs = query_funcs.query(unique_id);
  if (xmlMsgs.count(ERROR)) {
    pugi::xml_node errNode = resNode.append_child(ERROR);
    errNode.append_attribute(ID).set_value(unique_id.c_str());
    errNode.text().set(xmlMsgs[ERROR][0].c_str());
  }
  else {
    pugi::xml_node statusNode = resNode.append_child(STATUS);
    statusNode.append_attribute(ID).set_value(unique_id.c_str());
    if (xmlMsgs.count(OPEN)) {
      pugi::xml_node openNode = statusNode.append_child(OPEN);
      openNode.append_attribute(SHARES).set_value(xmlMsgs[OPEN][0].c_str());
    }
    if (xmlMsgs.count(CANCELED)) {
      pugi::xml_node canceledNode = statusNode.append_child(CANCELED);
      canceledNode.append_attribute(SHARES).set_value(xmlMsgs[CANCELED][0].c_str());
      canceledNode.append_attribute(TIME).set_value(xmlMsgs[CANCELED][1].c_str());
    }
    if (xmlMsgs.count(EXECUTED)) {
      pugi::xml_node executedNode = statusNode.append_child(EXECUTED);
      executedNode.append_attribute(SHARES).set_value(xmlMsgs[EXECUTED][0].c_str());
      executedNode.append_attribute(PRICE).set_value(xmlMsgs[EXECUTED][1].c_str());
      executedNode.append_attribute(TIME).set_value(xmlMsgs[EXECUTED][2].c_str());
    }
  }
  cout << "end query" << endl;
}

void XMLParser::parse() {
  cout << "start parse" << endl;
  pugi::xml_node createNode = request.child(CREATE);
  if (!createNode.empty()) {
    cout << "create exists" << endl;
    create(createNode);
    return;
  }
  pugi::xml_node transactionsNode = request.child(TRANSACTIONS);
  if (!transactionsNode.empty()) {
    transactions(transactionsNode);
    return;
  }
  throw myException("Error parsing XML");
}

/**
 * Iterates over the child nodes of the "CREATE" node. If a child node is an "ACCOUNT" node,
 * it calls "createAccount". If a child node is a "SYMBOL" node, it retrieves the symbol 
 * and calls "createPosition" for each child node of the "SYMBOL" node.
 * 
 * @param createNode - The "CREATE" node of the XML request
*/
void XMLParser::create(pugi::xml_node createNode) {
  cout << "create" << endl;
  for (pugi::xml_node createChild : createNode) {
    if (strcmp(createChild.name(), ACCOUNT) == 0) {
      createAccount(createChild);
    }
    else if (strcmp(createChild.name(), SYMBOL) == 0) {
      std::string sym = createChild.attribute(SYM).value();
      for (pugi::xml_node symbolChild : createChild) {
        if (strcmp(symbolChild.name(), ACCOUNT) == 0) {
          createPosition(sym, symbolChild);
        }
      }
    }
  }
  cout << "end create" << endl;
}

void XMLParser::transactions(pugi::xml_node transactionsNode) {
  cout << "transactions" << endl;
  std::string accountId = transactionsNode.attribute(ID).value();
  for (pugi::xml_node transaction : transactionsNode) {
    if (strcmp(transaction.name(), ORDER) == 0) {
      processOrder(transaction, accountId);
    }
    else if (strcmp(transaction.name(), CANCEL) == 0) {
      processCancel(transaction, accountId);
    }
    else if (strcmp(transaction.name(), QUERY) == 0) {
      processQuery(transaction, accountId);
    }
  }
  cout << "end transactions" << endl;
}

/**
 * creates a new account with the given unique ID and balance 
 * (in USD). The account has no positions. Attempting to create 
 * an account that already exists is an error.
 * 
 * @param accountNode
 */
void XMLParser::createAccount(pugi::xml_node accountNode) {
  cout << "createAccount" << endl;
  std::string accountId = accountNode.attribute(ID).value();
  double balance = accountNode.attribute(BALANCE).as_double();
  cout << accountId + " " << balance << endl;
  std::string msg = query_funcs.tryAddAccount(accountId, balance);
  cout << msg << endl;
  if (msg.empty()) {
    pugi::xml_node createdNode = resNode.append_child(CREATED);
    createdNode.append_attribute(ID).set_value(accountId.c_str());
  }
  else {
    pugi::xml_node errNode = resNode.append_child(ERROR);
    errNode.append_attribute(ID).set_value(accountId.c_str());
    errNode.text().set(msg.c_str());
  }
  cout << "end createAccount" << endl;
}

/**
 * creates or update a position with the given symbol.
 * 
 * @param sym
 * @param symbolChild
 */
void XMLParser::createPosition(std::string sym, pugi::xml_node symbolChild) {
  cout << "createPosition" << endl;
  std::string accountId = symbolChild.attribute(ID).value();
  std::string amount = symbolChild.child_value();
  cout << accountId << " " << amount << endl;
  std::string msg = query_funcs.tryAddPosition(accountId, sym, amount);
  cout << msg << endl;
  if (msg.empty()) {
    pugi::xml_node createdNode = resNode.append_child(CREATED);
    createdNode.append_attribute(SYM).set_value(sym.c_str());
    createdNode.append_attribute(ID).set_value(accountId.c_str());
  }
  else {
    pugi::xml_node errNode = resNode.append_child(ERROR);
    errNode.append_attribute(SYM).set_value(sym.c_str());
    errNode.append_attribute(ID).set_value(accountId.c_str());
    errNode.text().set(msg.c_str());
  }
  cout << "end createPosition" << endl;
}
