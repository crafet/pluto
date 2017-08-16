// tutorial
// date 2017-08-15

#include "epoll_tutorial.h"

namespace pluto {


const uint32_t kMaxFDCount = 64;
// create fd
int Tutorial::CreateBind() {
	struct addrinfo hints;
	struct addrinfo* result;
	struct addrinfo* rp;


    int sfd;
	memset((void*)(&hints), 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC; /* Return IPv4 and IPv6 choices */
	hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
	hints.ai_flags = AI_PASSIVE; /* All interfaces */

	int s = getaddrinfo(NULL, port.c_str(), &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}

    int index = 0;
	for (index=0,rp = result; rp != NULL; rp = rp->ai_next,index++) {
        cout << "index: " << index << endl;
        sockaddr_in* tp = (sockaddr_in*)(rp->ai_addr);
        //cout << "family: " << rp->ai_family << ", addr: " << tp->sin_addr.s_addr << endl;
        cout << "family: " << rp->ai_family << ", addr: " << inet_ntoa(tp->sin_addr) << endl;
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0) {
			/* We managed to bind successfully! */
			break;
		}

		close (sfd);
	}

	if (rp == NULL) {
		fprintf(stderr, "Could not bind\n");
		return -1;
	}

	freeaddrinfo(result);

    cout << "sfd: " << sfd << endl;
	return sfd;
}


int Tutorial::SetNonBlock(int fd) {

	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		fprintf(stderr, "failed to get flags of fd: %d\n", fd);
		return -1;
	}

	fprintf(stdout, "before set flags: %d\n", flags);
	flags |= O_NONBLOCK;
	fprintf(stdout, "after set flags: %d\n", flags);
	int s =  fcntl(fd, F_SETFL, flags);
	if (s == -1) {
		fprintf(stderr, "failed to set non block for fd\n");
		return -1;
	}

	return 0;
}


int Tutorial::Run() {
    fprintf(stdout, "epoll starts...\n");

    //used for listenfd
    epoll_event event;
    memset(&event, 0, sizeof(event));

    // used for epoll_wait, while epoll_wait returns, the readable event or writeable event will stores in events array
    epoll_event* events = NULL;

    // get fd
    int sfd = this->CreateBind();

    if (this->SetNonBlock(sfd) < 0) {
    	fprintf(stderr, "failed to set non block\n");
    	return -1;
    }

    listen(sfd, 1024);

    int efd = epoll_create(1024);
    fprintf(stdout, "success to create epoll fd: %d\n", efd);
    if (efd < 0) {
    	return -1;
    }

    // add listen fd to epollfd
    event.data.fd = sfd;

    // edge trigger will notice only once while the fd is readable or writeable
    // so the upstream should read or write the buffer of fd using a while loop
    fprintf(stdout, "before set events: %d\n", event.events);
    event.events = EPOLLIN|EPOLLET;
    fprintf(stdout, "after set events: %d\n", event.events);


    // add the listen fd (sfd) associated to epoll fd(efd), let the efd monitor the event of sfd;
    int s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);

    if (s < 0) {
    
        fprintf(stderr, "failed to associated listen fd: %d to epoll fd: %d", sfd, efd);
        return -1;
    }


    // get buffer for events for epoll_wait
    fprintf(stdout, "sizeof(epoll_event): %d\n", sizeof(epoll_event));
    events = (epoll_event*)calloc(kMaxFDCount, sizeof(event));
    if (events == NULL) {
        fprintf(stderr, "failed to calloc for arrya events");
        return -1;
    } 


    fprintf(stdout, "events size: %d, EPOLLET %d\n", sizeof(events), EPOLLET);

    while (true) {
    	// -1 means wait for indefinitely
    	int n = epoll_wait(efd, events, kMaxFDCount, -1);
    	for (int i = 0; i <n; i++) {
    		// if event is EPOLLERR
    		// if event is EPOLLHUP
    		// IF event is not EPOLLIN, not a readable event
    		if (events[i].events & EPOLLERR || events[i].events&EPOLLHUP || (!events[i].events & EPOLLIN)) {
    			fprintf(stderr, "index[%d] fd[%d] occer error, close it and continue to loop\n", i, events[i].data.fd);
    			close(events[i].data.fd);
    			continue;

    		} else if (sfd == events[i].data.fd) {
    			// if the listen fd
    			fprintf(stdout, "evented fd is sfd\n");
    			struct sockaddr in_addr;
    			socklen_t in_len = sizeof(in_addr);
    			int infd;

    			infd = accept(sfd, &in_addr, &in_len);
    			if (infd == -1) {
    				fprintf(stderr, "failed to accept fd");
    				return -1;
    			}

    			struct sockaddr client_addr;
    			socklen_t client_len = sizeof(client_addr);
    			getpeername(infd, &client_addr, &client_len);
    			sockaddr_in* client_in_addr = (sockaddr_in*)(&client_addr);
    			fprintf(stdout, "success to accept fd: %d, host :%s, port: %d\n", infd, inet_ntoa(client_in_addr->sin_addr),
    					client_in_addr->sin_port);

    			// set it as non block
    			int ret = this->SetNonBlock(infd);
    			if (ret < 0) {
    				//should not happen
    			}

    			event.data.fd = infd;
    			event.events =  EPOLLIN | EPOLLET;
    			epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);

    		} else {

    			// we have socket readable or writeable
    			// using loop to read or write
    			fprintf(stdout, "we have events to handle, fd: %d\n", events[i].data.fd);
    			char buf[512];
    			while (true) {
    				int count = read(events[i].data.fd, buf, sizeof(buf));
    				if (count == 0) {
    					fprintf(stderr, "fd %d has been closed", events[i].data.fd);
    					close(events[i].data.fd);
    					break;
    				}

    				// if -1 returns ,need to check the error and break the while loop
    				if (count == -1) {
    					switch(errno) {
    						// EAGAIN is EWOULDBLOCK, so check only one is ok
    						//case EAGAIN:
    						case EWOULDBLOCK:
    							fprintf(stderr, "fd %d is EAGAIN or EWOULDBLOCK break the loop\n", events[i].data.fd);
    							break;
    						case EINTR:
    							fprintf(stderr, "fd %d is EINTR continue\n", events[i].data.fd);
    							// continue to read
    							continue;
    						default:
    							fprintf(stderr, "fd %d is default handle method \n", events[i].data.fd);
    							break;
    					}

    					fprintf(stdout, "read fd %d count -1, need to break the while read loop\n", events[i].data.fd);
    					break;
    				} // if -1

    				//normal
    				fprintf(stdout, "while read");
    			} // while
    		} // else


    		fprintf(stdout, "i %d\n", i);
    	} //for

    	fprintf(stdout, "out of for\n");

    }
    return 0;
}

}// namespace

// appendix

