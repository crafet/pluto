
#include "epoll_tutorial.h"

using pluto::Tutorial;

int main() {
	
	

    Tutorial* t = new Tutorial("127.0.0.1", "9191");
    int epollfd = t->CreateBind();    
    
    delete t;
	return 0;
}
