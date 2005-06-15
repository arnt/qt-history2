#include "../shared/keyinfo.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s license-key\n", argv[0]);
        return 2;
    }

    uint products;
    uint platforms;
    uint licenseSchema;
    uint licenseFeatures;
    uint licenseID;
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

    printf("Products:        ");
    for (int i = 0; i < NumberOfProducts; ++i) {
        if ((products & (1 << i)) && Products[i])
            printf(" %s", Products[i]);
    }
    printf("\n"
           "Platforms:       ");
    for (int i = 0; i < NumberOfPlatforms; ++i) {
        if ((platforms & (1 << i)) && Platforms[i])
            printf(" %s", Platforms[i]);
    }
    printf("\n"
           "License Schema:  ");
    for (int i = 0; i < NumberOfLicenseSchemas; ++i) {
        if ((licenseSchema & (1 << i)) && LicenseSchemas[i])
            printf(" %s", LicenseSchemas[i]);
    }
    printf("\n"
           "License Features:");
    for (int i = 0; i < NumberOfLicenseFeatures; ++i) {
        if ((licenseFeatures & (1 << i)) && LicenseFeatures[i])
            printf(" %s", LicenseFeatures[i]);
    }
    printf("\n"
           "License ID:       %d\n"
           "Expiry Date:      %s\n",
           licenseID,
           expiryDate.toString("yyyy-MM-dd").toLocal8Bit().constData());
    return 0;
}
