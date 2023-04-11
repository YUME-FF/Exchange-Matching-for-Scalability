#include "sqlHelper.h"

#include <algorithm>
#include <fstream>
#include <iostream>

using namespace pqxx;
using namespace std;

void createTable(const string & filename, connection * C) {
  work work(*C);
  ifstream sql_file(filename);
  string sql_commands((istreambuf_iterator<char>(sql_file)), istreambuf_iterator<char>());
  work.exec(sql_commands);
  work.commit();
}

void dropTable(const string & tablename, connection * C) {
  work work(*C);
  string sql_commands = "DROP TABLE IF EXISTS " + tablename + " CASCADE;";
  work.exec(sql_commands);
  work.commit();
}

void cleanTable(const string & tablename, connection * C) {
  work work(*C);
  string sql_commands = "DELETE FROM " + tablename;
  work.exec(sql_commands);
  work.commit();
}

bool isNumeric(const string & str) {
  return all_of(str.begin(), str.end(), [](unsigned char c) { return isdigit(c); });
}

bool isAlphanumeric(string str) {
  return all_of(str.begin(), str.end(), [](char c) { return isalnum(c); });
}

int findCharInSubStr(const vector<char> & buffer, const vector<char> & target) {
  auto it = find_first_of(buffer.begin(), buffer.end(), target.begin(), target.end());
  return (it != buffer.end()) ? distance(buffer.begin(), it) : -1;
}

vector<char> addHeader(vector<char> buffer) {
  ostringstream oss;
  oss << buffer.size() << "\n";
  string header = oss.str();

  vector<char> res(header.length() + buffer.size());
  copy(header.begin(), header.end(), res.begin());
  copy(buffer.begin(), buffer.end(), res.begin() + header.length());

  return res;
}

vector<char> strToVec(string str) {
  return vector<char>(str.begin(), str.end());
}
