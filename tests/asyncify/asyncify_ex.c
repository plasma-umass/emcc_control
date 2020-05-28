#include "../../include/asyncify.h"
#include <stdio.h>
#include <stdlib.h>

char sleeping = 0;
uint32_t buf;

void my_sleep() {
    if(sleeping == 0) {
        sleeping = 1;
        asyncify_start_unwind(buf);
    } else {
        asyncify_stop_rewind();
        sleeping = 0;
    }
}

void the_main() {
    printf("1\n");
    my_sleep();
    printf("3\n");
}

int main() {
    buf = alloc_asyncify_buf();

    the_main();
    asyncify_stop_unwind();
    printf("2\n");
    asyncify_start_rewind(buf); // change 16
    the_main();
    return 0;
}