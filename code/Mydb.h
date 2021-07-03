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
    /*��ӡ�����������ݣ�����������*/
    bool execSql(string sql);

    /*��ѯ�����Կ��Ƿ��ز�ѯ������Ҳ��������*/
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
