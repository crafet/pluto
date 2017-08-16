// tutorial
// date 2017-08-15

#include "epoll_tutorial.h"

namespace pluto {

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


}// namespace

// appendix

