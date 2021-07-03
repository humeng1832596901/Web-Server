#include "Mydb.h"
#include <iostream>
#include <string>
#include <cstring>

using std::string;

MyDB::MyDB(){
    conn = mysql_init(NULL);
    if(conn==NULL){
        std::cout << "error occurs in MyDB() " << mysql_error(conn) << std::endl;
        exit(1);
    }
}


MyDB::~MyDB(){
    if(conn!=NULL){
        mysql_close(conn);
    }
}

bool MyDB::initDb(string host, string user, string password, string db_name){

    conn = mysql_real_connect(conn,nullptr,user.c_str(),password.c_str(),db_name.c_str(),0,NULL,0);
    if(conn==NULL){
        std::cout << "error occurs in initDb " << mysql_error(conn) << std::endl;
        exit(1);
    }
    return true;
}

/*execSql½ö×÷²âÊÔÓÃ*/
bool MyDB::execSql(string sql){
    
    if(mysql_query(conn,sql.c_str())){
        std::cout << "query error in execSql " << mysql_error(conn)<< std::endl;
        return false;
    }else{
        result = mysql_use_result(conn);
        if(result){
            int num_fields = mysql_num_fields(result);
            int num_rows = mysql_field_count(conn);
            for(int i=0; i<num_rows; i++){
                row = mysql_fetch_row(result);
                if(row<0) 
                    break;
                for(int j=0; j<num_fields; j++){
                    std::cout << row[j] << '\t';
                }

                std::cout << std::endl;
            }
        }
        mysql_free_result(result);
    }
    return false;
}

bool MyDB::findSql(string usr, string pwd, string& res){
    
    string sql = "select * from data where usr=" + usr;
    mysql_real_query(conn,sql.c_str(),sql.size());
    result = mysql_use_result(conn);
    row = mysql_fetch_row(result);
    if(!row){
        mysql_free_result(result);
        res = "ÎÞ´ËÕËºÅ";
        return false;
    }
    if(row[1]==pwd){
        mysql_free_result(result);
        return true;
    }else{
        res = "ÃÜÂë´íÎó";
        mysql_free_result(result);
        return false;
    }
}

bool MyDB::findKey(const string& usr){
    
    string sql = "select * from data where usr=" + usr;
    mysql_real_query(conn,sql.c_str(),sql.size());
    result = mysql_use_result(conn);
    row = mysql_fetch_row(result);
    if(row){
        mysql_free_result(result);
        return true;
    }
    mysql_free_result(result);
    return false;
}

bool MyDB::changePwd(const string& usr, const string& pwd){
    
    if(!findKey(usr))
        return false;
    string sql = "UPDATE data SET pwd=" + pwd + " where usr=" + usr;
    mysql_real_query(conn,sql.c_str(),sql.size());
    return true;
}

bool MyDB::insertSql(const string& usr, const string& pwd){
    
    if(findKey(usr)){
        return false;
    }
    string sql = "insert into data values(" + usr + "," + pwd + ")";
    if(mysql_real_query(conn,sql.c_str(),sql.size())){
        return false;
    }
    return true;
}
