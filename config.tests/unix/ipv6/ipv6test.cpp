/* Sample program for configure to test IPv6 support on target
platforms. We check for the required IPv6 data structures. */

#include <netinet/in.h>

int main()
{
    sockaddr_in6 tmp;

    return 0;
}
