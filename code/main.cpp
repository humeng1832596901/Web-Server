#include "HttpServer.h"
#include "Logging.h"
#include <iostream>
#include <signal.h>
#include <stdlib.h>

HttpServer* serverPtr;

//优雅关闭
void sighandler(int sig){
    serverPtr->stop();
}

int main(int argc, char** argv){
	// TODO 读配置文件
	int port = 8888;
	if(argc >= 2) {
		port = atoi(argv[1]);
	}
	int numThread = 4;
	if(argc >= 3) {
		numThread = atoi(argv[2]);
	}
    
    signal(SIGINT, sighandler);
    signal(SIGQUIT,sighandler);
    std::string logPath = "../log/Server.log";
    Logger::setLogFileName(logPath);

	HttpServer server(port, numThread);
    serverPtr = &server;
	server.run();

	return 0;
}
