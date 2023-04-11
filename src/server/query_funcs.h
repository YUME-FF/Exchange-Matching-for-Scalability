#ifndef __QUERY_FUNCS_H__
#define __QUERY_FUNCS_H__

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <pqxx/pqxx>
#include <unordered_map>

#include "myexception.h"
#include "sqlHelper.h"
#define ACC_EXIST "Account already exist"
#define ACC_NOT_EXIST "Account does not exist"
#define ERR_ACC_FORMAT "Account id is empty or format error"
#define ERR_SYM_FORMAT "Symbol is empty or format error"

using namespace std;
using namespace pqxx;

class Query_funcs {
 private:
  connection * C;
  void add_account(string id, double balance);
  void add_position(string id, string sym, string amount);

  void add_order(string account_id, string sym, double amount, double limit);

  void add_canceled(string unique_id,
                    string account_id,
                    string sym,
                    double amount,
                    double limit);

  void add_executed(string unique_id,
                    string account_id,
                    string sym,
                    double amount,
                    double price);
  void remove_position(string id, string sym);

  void remove_order(string id);
  void update_account(string id, double balance);
  void update_order(string unique_id, double amount);

  void update_position(string id, string sym, double amount);

  void process_order(string buyer_id,
                     string seller_id,
                     double amount,
                     string sym,
                     double price);
  bool accountifExists(string id);

  result getAccount(string id);

  double getBalance(string id);
  double getAmount(string id, string sym);
  result getPosition(string id, string sym);
  result getOrders(string id);
  result getOrdersBySym(string sym);
  result getCanceled(string id);
  result getExecuted(string id);
  std::time_t getCurrTime();

 public:
  Query_funcs() {
    try {
      //Establish a connection to the database
      //Parameters: database name, user name, user password
      C = new connection(
          "dbname=exchange user=postgres password=passw0rd host=postgres port=5432");
      if (C->is_open()) {
        cout << "Opened database successfully: " << C->dbname() << endl;
      }
      else {
        throw myException("Can't open database");
      }
    }
    catch (const std::exception & e) {
      cerr << e.what() << std::endl;
    }
    // dropTable("ACCOUNT", C);
    // dropTable("POSITION", C);
    // dropTable("ORDERS", C);
    // dropTable("EXECUTED", C);
    // dropTable("CANCELED", C);
  }
  string add_const(string id, double balance);
  connection * getConnection();

  std::string tryAddAccount(string id, double balance);

  std::string tryAddPosition(string id, string sym, string amount);
  //ORDERS (UNIQUE_ID, ACCOUNT_ID, SYMBOL, AMOUNT, LIMIT, TIME)
  //CANCELED (UNIQUE_ID, ACCOUNT_ID, SYM, AMOUNT, TIME)
  //EXECUTED (UNIQUE_ID, ACCOUNT_ID, SYMBOL, AMOUNT, PRICE, TIME)
  std::unordered_map<string, vector<string> > query(string id);
  std::unordered_map<string, vector<string> > order(string account_id,
                                                    string sym,
                                                    double amount,
                                                    double limit);
  std::unordered_map<string, vector<string> > cancel(string id);
};

#endif  //_QUERY_FUNCS_