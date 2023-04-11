#include "query_funcs.h"
using namespace std;
using namespace pqxx;
std::mutex mtx;

std::time_t Query_funcs::getCurrTime() {
  //from 1970.1.1 to current UTC time
  auto now = std::chrono::system_clock::now();
  auto seconds_since_epoch =
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
  return seconds_since_epoch;
}

int uni_id = 0;
std::string Query_funcs::add_const(string id, double balance) {
  if (!accountifExists(id)) {
    if (id.empty() || !isNumeric(id)) {
      return ERR_ACC_FORMAT;
    }
    add_account(id, balance);
    return "";
  }
  else {
    return ACC_EXIST;
  }
}
connection * Query_funcs::getConnection() {
  return C;
}
string Query_funcs::tryAddAccount(string id, double balance) {
  std::lock_guard<std::mutex> lck(mtx);
  if (!accountifExists(id)) {
    if (id.empty() || !isNumeric(id)) {
      return ERR_ACC_FORMAT;
    }
    add_account(id, balance);
    return "";
  }
  else {
    return ACC_EXIST;
  }
}

string Query_funcs::tryAddPosition(string id, string sym, string amount) {
  std::lock_guard<std::mutex> lck(mtx);
  if (accountifExists(id)) {
    if (sym.empty() || !isAlphanumeric(sym)) {
      return ERR_SYM_FORMAT;
    }
    result position = getPosition(id, sym);
    if (position.empty()) {
      add_position(id, sym, amount);
    }
    else {
      update_position(id, sym, stoi(amount) + position[0][2].as<double>());
    }
    return "";
  }
  else {
    return ACC_NOT_EXIST;
  }
}
std::unordered_map<string, vector<string> > Query_funcs::query(string id) {
  std::lock_guard<std::mutex> lck(mtx);
  //get ORDERS, CANCELED, EXECUTED table rows by transaction UNIQUE_ID
  result orderResult = getOrders(id);
  result canceledResult = getCanceled(id);
  result executedResult = getExecuted(id);
  std::unordered_map<string, vector<string> > xmlMsgs;
  //generate response XML message
  for (auto order = orderResult.begin(); order != orderResult.end(); ++order) {
    string amount = order[3].as<string>();
    xmlMsgs["open"].push_back(amount);
  }
  for (auto canceled = canceledResult.begin(); canceled != canceledResult.end();
       ++canceled) {
    string amount = canceled[3].as<string>();
    string time = canceled[4].as<string>();
    xmlMsgs["canceled"].push_back(amount);
    xmlMsgs["canceled"].push_back(time);
  }
  cout << "finish executed" << endl;
  for (auto executed = executedResult.begin(); executed != executedResult.end();
       ++executed) {
    string amount = executed[3].as<string>();
    string price = executed[4].as<string>();
    string time = executed[5].as<string>();
    xmlMsgs["executed"].push_back(amount);
    xmlMsgs["executed"].push_back(price);
    xmlMsgs["executed"].push_back(time);
  }
  if (xmlMsgs.empty()) {
    xmlMsgs["error"].push_back("Unique ID not exists!");
  }
  return xmlMsgs;
}

std::unordered_map<string, vector<string> > Query_funcs::cancel(string id) {
  std::lock_guard<std::mutex> lck(mtx);
  cout << "cancel start" << endl;
  //get ORDERS, EXECUTED table rows by transaction UNIQUE_ID
  result orderResult = getOrders(id);
  result executedResult = getExecuted(id);
  std::unordered_map<string, vector<string> > xmlMsgs;
  //cancel orders that are still open
  cout << "start cancel order" << endl;
  for (auto order = orderResult.begin(); order != orderResult.end(); ++order) {
    string u_id = order[0].as<string>();
    string a_id = order[1].as<string>();
    string sym = order[2].as<string>();
    double amount = order[3].as<double>();
    double limit = order[4].as<double>();
    remove_order(u_id);
    add_canceled(u_id, a_id, sym, amount, limit);
    if (amount > 0) {
      update_account(a_id, getBalance(a_id) + amount * limit);
    }
    xmlMsgs["canceled"].push_back(to_string(amount));
    xmlMsgs["canceled"].push_back(to_string(getCurrTime()));
  }
  cout << "end cancel order" << endl;
  cout << "start executed" << endl;
  for (auto executed = executedResult.begin(); executed != executedResult.end();
       ++executed) {
    string amount = executed[3].as<string>();
    string price = executed[4].as<string>();
    string time = executed[5].as<string>();
    xmlMsgs["executed"].push_back(amount);
    xmlMsgs["executed"].push_back(price);
    xmlMsgs["executed"].push_back(time);
  }
  cout << "end executed" << endl;
  if (xmlMsgs.empty()) {
    xmlMsgs["error"].push_back("Unique ID not exists!");
  }
  cout << "cancel finished" << endl;
  return xmlMsgs;
}

std::unordered_map<string, vector<string> > Query_funcs::order(string account_id,
                                                               string sym,
                                                               double amount,
                                                               double limit) {
  std::lock_guard<std::mutex> lck(mtx);
  std::unordered_map<string, vector<string> > msgs;
  //input error checking
  if (accountifExists(account_id)) {
    double balance = getBalance(account_id);
    if (amount > 0 && balance < amount * limit) {
      //send error msg; balance not enough
      msgs["error"].push_back("Balance is not enough!");
      return msgs;
    }
  }
  else {
    //send error msg; account not exist
    cout << account_id << endl;
    msgs["error"].push_back("Account not exists");
    return msgs;
  }
  result posRes = getPosition(account_id, sym);
  if (posRes.empty()) {
    msgs["error"].push_back("Transaction position not exists");
    return msgs;
  }
  else {
    double curr_amount = getAmount(account_id, sym);
    if (curr_amount + amount < 0) {
      //send error msg; share amount not enough
      msgs["error"].push_back("Share amount is not enough");
      return msgs;
    }
  }
  //lock
  if (amount > 0) {
    update_account(account_id, getBalance(account_id) - amount * limit);
  }
  msgs["unique_id"].push_back(to_string(uni_id));
  result res = getOrdersBySym(sym);
  for (pqxx::result::const_iterator order = res.begin(); order != res.end(); ++order) {
    string u_id = order[0].as<string>();
    string a_id = order[1].as<string>();
    string sym_d = order[2].as<string>();
    double amount_d = order[3].as<double>();
    double limit_d = order[4].as<double>();
    //amount positive for buy, negative for sale
    if (amount * amount_d < 0) {
      //buyer's limit>= seller's limit
      //buy
      if (amount > 0 && limit >= limit_d) {
        if (amount + amount_d >= 0) {
          //update account & position
          process_order(account_id, a_id, -amount_d, sym, limit_d);
          remove_order(u_id);
          add_executed(u_id, a_id, sym_d, amount_d, limit_d);
          add_executed(to_string(uni_id), account_id, sym, -amount_d, limit_d);
          amount += amount_d;
          if (amount == 0) {
            break;
          }
        }
        else {
          process_order(account_id, a_id, amount, sym, limit_d);
          update_order(u_id, amount_d + amount);
          add_executed(u_id, a_id, sym, -amount, limit_d);
          add_executed(to_string(uni_id), account_id, sym, amount, limit_d);
          amount = 0;
          break;
        }
      }

      //sell
      if (amount < 0 && limit <= limit_d) {
        if (amount + amount_d <= 0) {
          process_order(a_id, account_id, amount_d, sym, limit_d);
          remove_order(u_id);
          add_executed(u_id, a_id, sym_d, amount_d, limit_d);
          add_executed(to_string(uni_id), account_id, sym, -amount_d, limit_d);
          amount += amount_d;
          if (amount == 0) {
            break;
          }
        }
        else {
          process_order(a_id, account_id, -amount, sym, limit_d);
          update_order(u_id, amount_d + amount);
          add_executed(u_id, a_id, sym, -amount, limit_d);
          add_executed(to_string(uni_id), account_id, sym, amount, limit_d);
          amount = 0;
          break;
        }
      }
    }
  }
  if (amount != 0) {
    add_order(account_id, sym, amount, limit);
  }
  //unlock
  return msgs;
}

void Query_funcs::add_account(string id, double balance) {
  work work(*C);
  stringstream instr;
  instr << "INSERT INTO ACCOUNT (ID, BALANCE) "
           "VALUES(\'"
        << id << "\'," << balance << ");";
  work.exec(instr.str());
  work.commit();
}
void Query_funcs::add_position(string id, string sym, string amount) {
  work work(*C);
  stringstream instr;
  instr << "INSERT INTO POSITION (ID, SYMBOL, AMOUNT) "
           "VALUES(\'"
        << id << "\',\'" << sym << "\'," << amount + ");";
  work.exec(instr.str());
  work.commit();
}

void Query_funcs::add_order(string account_id, string sym, double amount, double limit) {
  work work(*C);
  stringstream instr;
  instr << "INSERT INTO ORDERS (UNIQUE_ID, ACCOUNT_ID, SYMBOL, AMOUNT, LLIMIT) "
           "VALUES("
        << uni_id++ << ",\'" << account_id << "\',"
        << "\'" << sym << "\'," << amount << "," << limit << ");";
  work.exec(instr.str());
  work.commit();
}

void Query_funcs::add_canceled(string unique_id,
                               string account_id,
                               string sym,
                               double amount,
                               double limit) {
  work work(*C);
  stringstream instr;
  instr << "INSERT INTO CANCELED (UNIQUE_ID, ACCOUNT_ID, SYMBOL, AMOUNT, TTIME) "
           "VALUES("
        << unique_id << ",\'" << account_id << "\',"
        << "\'" << sym << "\'," << amount << ","
        << "\'" << getCurrTime() << "\');";
  work.exec(instr.str());
  work.commit();
}

void Query_funcs::add_executed(string unique_id,
                               string account_id,
                               string sym,
                               double amount,
                               double price) {
  work work(*C);
  stringstream instr;
  instr << "INSERT INTO EXECUTED (UNIQUE_ID, ACCOUNT_ID, SYMBOL, AMOUNT, PRICE, TTIME) "
           "VALUES("
        << unique_id << ",\'" << account_id << "\',"
        << "\'" << sym << "\'," << amount << "," << price << ","
        << "\'" << getCurrTime() << "\');";
  work.exec(instr.str());
  work.commit();
}
void Query_funcs::remove_position(string id, string sym) {
  work work(*C);
  stringstream instr;
  instr << "DELETE FROM POSITION WHERE ID = \'" << id << "\' AND SYMBOL = \'" << sym
        << "\';";
  work.exec(instr.str());
  work.commit();
}

void Query_funcs::remove_order(string id) {
  work work(*C);
  stringstream instr;
  instr << "DELETE FROM ORDERS WHERE UNIQUE_ID = \'" << id << "\';";
  work.exec(instr.str());
  work.commit();
}
void Query_funcs::update_account(string id, double balance) {
  work work(*C);
  stringstream instr;
  instr << "UPDATE ACCOUNT SET BALANCE = " << balance << " WHERE ID = \'" << id << "\';";
  work.exec(instr.str());
  work.commit();
}
void Query_funcs::update_order(string unique_id, double amount) {
  work work(*C);
  stringstream instr;
  instr << "UPDATE ORDERS SET AMOUNT = " << amount << " WHERE UNIQUE_ID = \'" << unique_id
        << "\';";
  work.exec(instr.str());
  work.commit();
}

void Query_funcs::update_position(string id, string sym, double amount) {
  work work(*C);
  stringstream instr;
  instr << "UPDATE POSITION SET AMOUNT = " << amount << " WHERE ID = \'" << id
        << "\' AND SYMBOL = \'" << sym << "\';";
  work.exec(instr.str());
  work.commit();
}

void Query_funcs::process_order(string buyer_id,
                                string seller_id,
                                double amount,
                                string sym,
                                double price) {
  //lock
  //buyer update
  update_position(buyer_id, sym, getAmount(buyer_id, sym) + amount);
  //seller update
  update_account(seller_id, getBalance(seller_id) + amount * price);
  double curr_amount = getAmount(seller_id, sym);
  if (curr_amount > amount) {
    update_position(seller_id, sym, curr_amount - amount);
  }
  else {
    remove_position(seller_id, sym);
  }
  //unlock
}
bool Query_funcs::accountifExists(string id) {
  nontransaction N(*C);
  stringstream instr;
  instr << "SELECT * FROM ACCOUNT WHERE ID = \'" << id << "\';";
  result res(N.exec(instr.str()));
  if (!res.empty()) {
    return true;
  }
  else {
    return false;
  }
}

result Query_funcs::getAccount(string id) {
  nontransaction N(*C);
  stringstream instr;
  instr << "SELECT * FROM ACCOUNT WHERE ID = \'" << id << "\';";
  return N.exec(instr.str());
}

double Query_funcs::getBalance(string id) {
  return getAccount(id).begin()[1].as<double>();
}
double Query_funcs::getAmount(string id, string sym) {
  return getPosition(id, sym).begin()[2].as<double>();
}
result Query_funcs::getPosition(string id, string sym) {
  nontransaction N(*C);
  stringstream instr;
  instr << "SELECT * FROM POSITION WHERE ID = \'" << id << "\' AND SYMBOL = \'" << sym
        << "\';";
  return N.exec(instr.str());
}
result Query_funcs::getOrders(string id) {
  nontransaction N(*C);
  stringstream instr;
  instr << "SELECT * FROM ORDERS WHERE UNIQUE_ID = " << id << ";";
  return N.exec(instr.str());
}
result Query_funcs::getOrdersBySym(string sym) {
  nontransaction N(*C);
  stringstream instr;
  instr << "SELECT * FROM ORDERS WHERE SYMBOL = \'" << sym << "\';";
  return N.exec(instr.str());
}
result Query_funcs::getCanceled(string id) {
  nontransaction N(*C);
  stringstream instr;
  instr << "SELECT * FROM CANCELED WHERE UNIQUE_ID = " << id << "ORDER BY TTIME ;";
  return N.exec(instr.str());
}
result Query_funcs::getExecuted(string id) {
  nontransaction N(*C);
  stringstream instr;
  instr << "SELECT * FROM EXECUTED WHERE UNIQUE_ID = " << id << "ORDER BY TTIME ;";
  return N.exec(instr.str());
}
