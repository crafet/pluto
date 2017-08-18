
#include "epoll_tutorial.h"

#include "buffer.h"

using pluto::Tutorial;
using pluto::Buffer;


int main() {
	
	

    Tutorial* t = new Tutorial("127.0.0.1", "9191");
    //int epollfd = t->CreateBind();    
    

    //t->SetNonBlock(epollfd);
    int ret = t->Run();
    if (ret < 0) {
        fprintf(stderr, "failed to Run");
    }

    delete t;
    return 0;
}
