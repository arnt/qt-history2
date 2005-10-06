#include <cstdio>
#include "../InstallerPane/keydec.h"

int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;
    KeyDecoder key(argv[1]);
    if (!key.IsValid())
        return 2;
    CDate date = key.getExpiryDate();
    fprintf(stdout, "%04d-%02d-%02d\n", date.year(), date.month(), date.day());
    return 0;
}
