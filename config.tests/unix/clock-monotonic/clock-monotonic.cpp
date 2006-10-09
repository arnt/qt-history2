#include <time.h>

int main(int, char **)
{
#if defined(_POSIX_MONOTONIC_CLOCK) && (_POSIX_MONOTONIC_CLOCK-0 >= 0)
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
#else
#  error "Feature _POSIX_MONOTONIC_CLOCK not available"
#endif
    return 0;
}

