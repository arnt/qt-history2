#include "../../scripts/mac-binary/package/InstallerPane/keydec.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s license-key\n", argv[0]);
        return 2;
    }

    KeyDecoder keydec(argv[1]);
    if (!keydec.IsValid()) {
        printf("invalid license key\n");
        return 1;
    }

    uint products = keydec.getProducts();
    uint platforms = keydec.getPlatforms();
    uint licenseSchema = keydec.getLicenseSchema();
    uint licenseFeatures = keydec.getLicenseFeatures();
    for (int i = 0; i < KeyDecoder::NumberOfProducts; ++i) {
        if ((products & (1 << i)) && KeyDecoder::Products[i]) {
            printf("%s", KeyDecoder::Products[i]);
            if ((products ^= (1 << i)) != 0)
                printf(",");
        }
    }
    printf("\n");
    for (int i = 0; i < KeyDecoder::NumberOfPlatforms; ++i) {
        if ((platforms & (1 << i)) && KeyDecoder::Platforms[i]) {
            printf("%s", KeyDecoder::Platforms[i]);
            if ((platforms ^= (1 << i)) != 0)
                printf(",");
        }
    }
    printf("\n");
    for (int i = 0; i < KeyDecoder::NumberOfLicenseSchemas; ++i) {
        if ((licenseSchema & (1 << i)) && KeyDecoder::LicenseSchemas[i])
            printf("%s", KeyDecoder::LicenseSchemas[i]);
    }
    printf("\n");
    if (licenseFeatures) {
        for (int i = 0; i < KeyDecoder::NumberOfLicenseFeatures; ++i) {
            if ((licenseFeatures & (1 << i)) && KeyDecoder::LicenseFeatures[i])
                printf(" %s", KeyDecoder::LicenseFeatures[i]);
        }
    } else {
        printf("none");
    }

    CDate date = keydec.getExpiryDate();
    printf("\n"
           "%d\n"
           "%d-%02d-%02d\n",
           keydec.getLicenseID(),
           date.year(), date.month(), date.day());
    return 0;
}
