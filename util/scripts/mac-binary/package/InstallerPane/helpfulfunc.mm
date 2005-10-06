/*
 *  helpfulfunc.c
 *  InstallerPane
 *
 *  Created by Trenton Schulz on 7/27/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>
#include "helpfulfunc.h"
#include "keydec.h"

FILE *getQtLicenseFile(const char *mode)
{
    const char *homeDir = getenv("HOME");
    static const char LicenseFile[] = ".qt-license";
    char *filename = 0;
    FILE *licenseFile = 0;
    if (homeDir) {
        int sizeEnv = strlen(homeDir);
        int sizeLicense = strlen(LicenseFile);
        filename = (char *)malloc(sizeLicense + sizeEnv + 3);
        strncpy(filename, homeDir, sizeEnv);
        filename[sizeEnv] = '/';
        strncpy(filename + sizeEnv + 1, LicenseFile, sizeLicense);
        licenseFile = fopen(filename, mode);
        free(filename);
    }
    return licenseFile;
}

int validateLicense(const char *string)
{
    KeyDecoder key(string);
    int ret = InvalidLicense;
    if (key.IsValid()) {
        if (!(key.getProducts() & (KeyDecoder::QtUniversal | KeyDecoder::QtDesktop
                                   | KeyDecoder::QtDesktopLight))) {
            ret = InvalidProduct;
        } else {
            if (!(key.getPlatforms() & KeyDecoder::Mac)) {
                ret = InvalidPlatform;
            } else {
#ifdef QT_EVAL
                if (!(key.getLicenseSchema() & (KeyDecoder::SupportedEvaluation
                                                | KeyDecoder::UnsupportedEvaluation
                                                | KeyDecoder::FullSourceEvaluation)))
#else
                    if (!(key.getLicenseSchema() & KeyDecoder::FullCommercial))    
#endif
                    {
                        ret = InvalidType;
                    } else {
                        ret = LicenseOK;
                    }
            }
        }
    }
        return ret;
}
