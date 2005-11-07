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

extern const char LicenseeString[];
extern const char LicenseKeyExtString[];

enum LicenseValues { LicenseOK = 0, InvalidLicense, InvalidProduct, InvalidPlatform, InvalidType, LicenseExpired };

int validateLicense(const char *string);

#endif // HELPFULFUNC_H