/*
 *  helpfulfunc.h
 *  InstallerPane
 *
 *  Created by Trenton Schulz on 7/27/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef HELPFULFUNC_H
#define HELPFULFUNC_H

#import <stdio.h>

static const char LicenseKeyExtString[] = "LicenseKeyExt=";
static const char LicenseeString[] = "Licensee=";

enum LicenseValues { LicenseOK = 0, InvalidLicense, InvalidProduct, InvalidPlatform, InvalidType, LicenseExpired };

FILE *getQtLicenseFile(const char *mode);
int validateLicense(const char *string);

#endif // HELPFULFUNC_H