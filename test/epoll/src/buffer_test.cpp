#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>


using namespace pluto;


int main() {

	// default size is 4*1024
    Buffer buffer;
    fprintf(stdout, "readable size: %d, writable %d\n", buffer.readable(), buffer.writable());

    // append more bytes cause resize
    char buf[6*1024];
    memset(buf, 0, sizeof(buf));
    buffer.append(buf, sizeof(buf));

    fprintf(stdout, "readable size: %d, writable %d\n", buffer.readable(), buffer.writable());

    // read 1024 bytes, reader_index += 1024
    buffer.retrieve(1024);

    fprintf(stdout, "readable size: %d, writable %d\n", buffer.readable(), buffer.writable());
}
