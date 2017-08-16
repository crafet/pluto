#include <string>
#include <iostream>

// for time_t
#include <sys/time.h>

#include <unistd.h>

// for uint32_t definition
#include <stdint.h>

#include <sys/epoll.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

// for memset
#include <string.h>

// for stderr
#include <stdio.h>

// for inet_ntoa
#include<arpa/inet.h>

// for fcntl api to set the non block attribute of fd
#include <fcntl.h>
using namespace std;

/**
 * aims to build a tutorial of epoll server
 */

namespace pluto {
class Tutorial {

public:

    Tutorial(string host, string port) {
        this->host = host;
        this->port = port;
    }
	int CreateBind();

	// set fd as non block;
	int SetNonBlock(int fd);


    int Run();
private:

	// listen at host:port
	string host;
	string port;

};

}
