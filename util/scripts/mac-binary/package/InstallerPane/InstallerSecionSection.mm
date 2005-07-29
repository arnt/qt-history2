//
//  InstallerSecionSection.mm
//  InstallerPane
//
//  Created by Trenton Schulz on 7/27/05.
//  Copyright 2005 __MyCompanyName__. All rights reserved.
//

#import "InstallerSecionSection.h"
#import "helpfulfunc.h"
#import <stdio.h>
#import <string.h>


static BOOL checkForLicenseFile()
{
    static const int BUFFERSIZE = 8192;
    static const int LICENSESIZE = 42;
    if (FILE *licenseFile = getQtLicenseFile("r")) {
        char charBuffer[BUFFERSIZE];
        while (!feof(licenseFile)) {
            if (fgets(charBuffer, 8192, licenseFile)) {
                char *location = strstr(charBuffer, LicenseKeyExtString);
                if (location) {
                    location += strlen(LicenseKeyExtString);
                    char licenseString[LICENSESIZE];
                    strncpy(licenseString, location, LICENSESIZE);
                    licenseString[LICENSESIZE - 1] = '\0';
                    location = strstr(licenseString, "\n");
                    if (location)
                        *location = '\0';
                    return validateLicense(licenseString) == LicenseOK;
                }
                
            }
        }
        return NO;
    }
    return NO;
}


@implementation InstallerSecionSection

- (BOOL)shouldLoad
{
    return !checkForLicenseFile();
}

@end
