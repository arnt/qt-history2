#include "../shared/keyinfo.h"

int main(int argc, char **argv)
{
    if (argc != 7) {
        printf("usage: %s product platform license-schema license-features license-id expiry-date\n", argv[0]);

        printf("\nproduct:\n");
        for (int i = 0; i < NumberOfProducts; ++i)
            if (Products[i])
                printf("\t\t%s\n", Products[i]);

        printf("\nplatform:\n ");
        for (int i = 0; i < NumberOfPlatforms; ++i)
            if (Platforms[i])
                printf("\t\t%s\n", Platforms[i]);

        printf("\nlicense schema:\n");
        for (int i = 0; i < NumberOfLicenseSchemas; ++i)
            if (LicenseSchemas[i])
                printf("\t\t%s\n", LicenseSchemas[i]);

        printf("\nlicense feature:\n");
        for (int i = 0; i < NumberOfLicenseFeatures; ++i)
            if (LicenseFeatures[i])
                printf("\t\t%s\n", LicenseFeatures[i]);
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
