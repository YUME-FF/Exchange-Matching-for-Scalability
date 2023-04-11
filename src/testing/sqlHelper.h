#ifndef __SQLHELPER_H__
#define __SQLHELPER_H__

#include <pqxx/pqxx>
#include <string>
#include <vector>

void createTable(const std::string & filename, pqxx::connection * C);
void dropTable(const std::string & tablename, pqxx::connection * C);
void cleanTable(const std::string & tablename, pqxx::connection * C);
bool isNumeric(const std::string & str);
bool isAlphanumeric(std::string str);
int findCharInSubStr(const std::vector<char> & buffer, const std::vector<char> & target);
std::vector<char> addHeader(std::vector<char> buffer);
std::vector<char> strToVec(std::string str);

#endif
