#include "unpack.h"

int main() {

    const char* paths[] = {
        "hello1.txt",
        "hello2.txt",
        "hello3.txt",
        "dir1/hello4.txt",
        "dir1/dir2/hello5.txt"
    };

    for (int i = 0; i < 5; i++) {
        const char* path = paths[i];
        size_t size;
        if (cdpk_get_data_size(path, &size)) return 1;
        char* buf = malloc(size + 1);
        buf[size] = 0;
        if (cdpk_get_data(path, &buf, 0)) return 1;
        printf("%d %s\n", size, buf);
        free(buf);
    }


}