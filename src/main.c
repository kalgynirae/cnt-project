#include <stdio.h>
#include "hello.h"
#include "common.h"

int main(int argc, char *argv[]) {
    say_hello();
    printf("\n");
    printf(FILE_NAME);
    printf("\n");
    return 0;
}
