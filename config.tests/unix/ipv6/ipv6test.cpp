/* Sample program for configure to test IPv6 support on target
platforms. We check for the required IPv6 data structures. */

#include <sys/types.h>
#include <netinet/in.h>

int main()
{
    sockaddr_in6 tmp;
    (void)tmp;

    return 0;
}
