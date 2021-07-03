#ifndef _DB_H
#define _DB_H

#include<iostream>
#include<mysql/mysql.h>
#include<string>

using std::string;

class MyDB{
public:
    MyDB();
    ~MyDB();
    bool initDb(string host,string user,string password,string db_name);
    /*打印表单的所有数据，仅作测试用*/
    bool execSql(string sql);

    /*查询，可以考虑返回查询结果，找不到或错误*/
    bool findSql(string usr, string pwd, string& res);
    bool findKey(const string& usr);
    bool changePwd(const string& usr, const string& pwd);
    bool insertSql(const string& usr, const string& pwd);
private:
    MYSQL* conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
};

#endif
