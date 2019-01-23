#include <stdio.h>
#include "coroutine.h"

int main(int argc, char const *argv[])
{
    tswCo_schedule *S = tswCo_open();
    if (S == NULL) {
        printf("tswCo_open error");
    }

    tswCo_close(S);
    return 0;
}
