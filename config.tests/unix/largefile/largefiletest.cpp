/* Sample program for configure to test Large File support on target
platforms.
*/

#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

int main()
{
// check that off_t can hold 2^63 - 1 and perform basic operations...
#define OFF_T_64 (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))
    assert( OFF_T_64 % 2147483647 == 1 );
}
