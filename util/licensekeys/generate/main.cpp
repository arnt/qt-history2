#include "keyinfo.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 7) {
        printf("usage: %s product platform license-schema license-features license-id expiry-date\n", argv[0]);

        printf("\nproduct:\n");
        for (int i = 0; i < KeyDecoder::NumberOfProducts; ++i)
            if (KeyDecoder::Products[i])
                printf("\t\t%s\n", KeyDecoder::Products[i]);

        printf("\nplatform:\n ");
        for (int i = 0; i < KeyDecoder::NumberOfPlatforms; ++i)
            if (KeyDecoder::Platforms[i])
                printf("\t\t%s\n", KeyDecoder::Platforms[i]);

        printf("\nlicense schema:\n");
        for (int i = 0; i < KeyDecoder::NumberOfLicenseSchemas; ++i)
            if (KeyDecoder::LicenseSchemas[i])
                printf("\t\t%s\n", KeyDecoder::LicenseSchemas[i]);

        printf("\nlicense feature:\n");
        for (int i = 0; i < KeyDecoder::NumberOfLicenseFeatures; ++i)
            if (KeyDecoder::LicenseFeatures[i])
                printf("\t\t%s\n", KeyDecoder::LicenseFeatures[i]);
        printf("\t\tnone\n");

        printf("\nlicense id: integer\n");
        printf("\nexpiry-date: YYYY-MM-DD\n");

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
