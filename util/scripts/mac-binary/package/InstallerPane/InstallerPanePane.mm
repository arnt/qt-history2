#import "InstallerPanePane.h"
#import "helpfulfunc.h"

@implementation InstallerPanePane
- (NSString *)title
{
    return [[NSBundle bundleForClass:[self class]] localizedStringForKey:@"PaneTitle"
                                                                   value:nil table:nil];
}

- (id)init
{
    self = [super init];
    if (self) {
        licenseStatus = InvalidLicense;
        nameCheckOK = NO;
        fullLicenseKey = [[NSMutableString alloc] initWithCapacity:35];
    }
    return self;
}

- (BOOL)shouldExitPane:(InstallerSectionDirection)dir
{ 
    if ((dir == InstallerDirectionForward)) {
        if ((licenseStatus != LicenseOK) && !nameCheckOK)
            return NO;
        FILE *licenseFile = getQtLicenseFile("w");
        char outputString[sizeof(LicenseeString) + 5 + sizeof(LicenseKeyExtString) + 4];
        snprintf(outputString, sizeof(outputString), "%s\"%%s\"\n%s%%s\n", LicenseeString,
                 LicenseKeyExtString);
        fprintf(licenseFile, outputString,
                [[nameField stringValue] UTF8String], [fullLicenseKey UTF8String]);
        fclose(licenseFile);
    }
    return YES; 
}

- (BOOL)shouldLoad
{
    BOOL okToLoad = NO;
 
    return okToLoad;
}

- (void)didEnterPane:(InstallerSectionDirection)dir
{
    [self tryEnable];
    [nameField selectText:self];
}

- (void)controlTextDidChange:(NSNotification *)aNotification
{
    [self checkName];
    [self checkLicense];
}


- (void)checkLicense
{
    int i;
    [fullLicenseKey setString:@""];
    NSTextField *theLicenseFields[] = { LicenseField1, LicenseField2, LicenseField3, LicenseField4,
                                        LicenseField5, LicenseField6, LicenseField7 };
    
    for (i = 0; i < sizeof(theLicenseFields) / sizeof(NSTextField*); ++i) {
        NSString *part = [[theLicenseFields[i] stringValue]
                            stringByTrimmingCharactersInSet:[NSCharacterSet
                                                                whitespaceAndNewlineCharacterSet]];
        if ([part length] <= 0) {
            licenseStatus = InvalidLicense;
            [self tryEnable];
            return;
        }
        if (i != 0)
            [fullLicenseKey appendString:@"-"];
        [fullLicenseKey appendString:[part uppercaseString]];
    }
    
    licenseStatus = validateLicense([fullLicenseKey UTF8String]);
    [self tryEnable];
}

- (void)checkName
{
    NSString *name = [nameField stringValue];
    nameCheckOK = [name length] > 0;
    [self tryEnable];
}


- (void)tryEnable
{
    [self setNextEnabled:(licenseStatus == LicenseOK) && nameCheckOK];
    switch (licenseStatus) {
    case LicenseOK:
    case InvalidLicense:
        [errorField setStringValue:@""];
        break;
    case InvalidType:
        [errorField setStringValue:@"This License cannot be used with this version of Qt"];
        break;
    case InvalidPlatform:
        [errorField setStringValue:@"This License cannot be used for this Qt Platform"];
        break;
    }
}


@end


