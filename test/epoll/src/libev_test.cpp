#include <ev.h>

// for STDIN_FILENO
#include <unistd.h>
#include <stdio.h>


void io_cb(EV_P_ ev_io*w, int events) {

    static int times;
    printf("in io_cb function\n");

    // if data could read, read it from STDIN_FILENO
    // �������˲��ԣ���STDIN_FILENO�����ݿɶ���ʱ�������read����ô��������ᱻ���ص���
    // ���read���֣�����ص�����Ҳ�ᱻ�����ص���ֱ��readû�����ݿ��Զ�Ϊֹ��
    // �������ÿ��read 1��byte����2���Լ���buf��ĩβ��\0������printf������ݡ�
    // ����123�����Ϊ��
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
    // ����123���س������Կ��������ĴΣ��������һ���س�����ȻҲ����˻س����У��س���valueΪ10
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
