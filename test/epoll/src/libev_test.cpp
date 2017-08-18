#include <ev.h>

// for STDIN_FILENO
#include <unistd.h>
#include <stdio.h>


void io_cb(EV_P_ ev_io*w, int events) {

    static int times;
    printf("in io_cb function\n");

    // if data could read, read it from STDIN_FILENO
    // 这里做了测试，当STDIN_FILENO有数据可读的时候，如果不read，那么这个函数会被返回调用
    // 如果read部分，这个回调函数也会被反复回调，直到read没有数据可以读为止。
    // 这里测试每次read 1个byte，用2个自己的buf，末尾放\0，用于printf输出内容。
    // 输入123，结果为：
    /**
     * [root@SHVM002278 /data/yilinliu/git_workspace/pluto/test/epoll]# ./libev_test
	success to create loop
	major: 4, min:22
	io_watcher is active: 1
	123
	in io_cb function
	read data: 1, content:  1, value: 49, times: 0
	in io_cb function
	read data: 1, content:  2, value: 50, times: 1
	in io_cb function
	read data: 1, content:  3, value: 51, times: 2
	in io_cb function
	read data: 1, content:
	, value: 10, times: 3
     */
    // 输入123并回车，可以看到读了四次，包含最后一个回车。当然也输出了回车换行，回车的value为10
    char buf[2];
    memset(buf, 0, 2);
    int n = read(STDIN_FILENO, buf, 1);
    
    printf("read data: %d, content:  %s, value: %d, times: %d\n", n, buf, char(buf[0]), times);
    ++times;
    //ev_io_stop(EV_A_ w);
    return;
    
}

int main() {

    EV_P = ev_default_loop(0);
    if (loop != NULL) {
    
        fprintf(stdout, "success to create loop\n");
    }

    // get version of libev
    int major = ev_version_major ();
    int min = ev_version_minor();

    fprintf(stdout, "major: %d, min:%d\n", major, min);
    ev_io io_watcher;

    ev_init(&io_watcher, io_cb);

    // check the 
    ev_io_set(&io_watcher, STDIN_FILENO, EV_READ);
    ev_io_start(EV_A_ &io_watcher);

    int active = ev_is_active(&io_watcher);
    fprintf(stdout, "io_watcher is active: %d\n", active);

    ev_run(EV_A, 0);
    
    active = ev_is_active(&io_watcher);
  
    // active = 0 ,the io_watcher has been stop in its call back funciton
    fprintf(stdout, "io_watcher is active: %d\n", active);
    return 0;
}
