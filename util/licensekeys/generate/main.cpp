#include "../shared/keyinfo.h"

int main(int argc, char **argv)
{
    if (argc != 7) {
        printf("usage: %s product platform license-schema license-features license-id expiry-date\n", argv[0]);
        return 2;
    }
    QByteArray licenseKey = generateLicenseKey(argv[1],
                                               argv[2],
                                               argv[3],
                                               argv[4],
                                               argv[5],
                                               argv[6]);
    printf("%s\n", licenseKey.constData());
    return 0;
}
