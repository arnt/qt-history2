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
#include <unistd.h>
#include "helpfulfunc.h"
#include "keydec.h"

#include <Cocoa/Cocoa.h>

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

    if (ret == LicenseOK) {
        // We need to check ourselves against the build date
        // First read in the build date and then use it to compare with the
        // Expiry Date if it's commercial or the current date if it's an eval.
        CDate date = key.getExpiryDate();
        NSCalendarDate *expiryDate = [NSCalendarDate dateWithYear:date.year()
                                                            month:date.month() day:date.day()
                                                             hour:23 minute:59 second:59 
                                                         timeZone:[NSTimeZone systemTimeZone]];
        NSCalendarDate *compareDay = 0;
        if (key.getLicenseSchema() & KeyDecoder::FullCommercial) {
            // There's a lot of bundles here, so we have to make sure we get the correct one
            NSBundle *bundle = nil;
            NSArray *bundles = [NSBundle allBundles];
            for (uint i = 0; i < [bundles count]; ++i) {
                NSBundle *bun = [bundles objectAtIndex: i];
                NSRange location = [[bun bundleIdentifier] rangeOfString:@"com.trolltech.qt4."];
                if (location.length != 0) {
                    bundle = bun;
                    break;
                }
            }
            NSString *contents = [NSString stringWithContentsOfFile:[bundle pathForResource:@".package_date" ofType:nil]
                                                        encoding:NSUTF8StringEncoding error:0];
            compareDay = [NSCalendarDate dateWithString: contents calendarFormat:@"%Y-%m-%d"];
        } else {
            compareDay = [NSCalendarDate calendarDate];
        }
        if ([expiryDate laterDate: compareDay] != expiryDate)
            ret = LicenseExpired;
    }
    return ret;
}
