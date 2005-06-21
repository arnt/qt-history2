#include "../shared/keyinfo.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s license-key\n", argv[0]);
        return 2;
    }

    uint products = 0;
    uint platforms = 0;
    uint licenseSchema = 0;
    uint licenseFeatures = 0;
    uint licenseID = 0;
    QDate expiryDate;
    if (!decodeLicenseKey(argv[1],
                          &products,
                          &platforms,
                          &licenseSchema,
                          &licenseFeatures,
                          &licenseID,
                          &expiryDate)) {
        printf("invalid license key\n");
        return 1;
    }

    for (int i = 0; i < NumberOfProducts; ++i) {
        if ((products & (1 << i)) && Products[i]) {
            printf("%s", Products[i]);
            if ((products ^= (1 << i)) != 0)
                printf(",");
        }
    }
    printf("\n");
    for (int i = 0; i < NumberOfPlatforms; ++i) {
        if ((platforms & (1 << i)) && Platforms[i]) {
            printf("%s", Platforms[i]);
            if ((platforms ^= (1 << i)) != 0)
                printf(",");
        }
    }
    printf("\n");
    for (int i = 0; i < NumberOfLicenseSchemas; ++i) {
        if ((licenseSchema & (1 << i)) && LicenseSchemas[i])
            printf("%s", LicenseSchemas[i]);
    }
    printf("\n");
    if (licenseFeatures) {
        for (int i = 0; i < NumberOfLicenseFeatures; ++i) {
            if ((licenseFeatures & (1 << i)) && LicenseFeatures[i])
                printf(" %s", LicenseFeatures[i]);
        }
    } else {
        printf("none");
    }
    printf("\n"
           "%d\n"
           "%s\n",
           licenseID,
           expiryDate.toString("yyyy-MM-dd").toLocal8Bit().constData());
    return 0;
}
