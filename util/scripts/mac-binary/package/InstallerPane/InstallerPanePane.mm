#import "InstallerPanePane.h"
#import "keydec.h"
#import <stdio.h>


static FILE *getQtLicenseFile(const char *mode)
{
    const char *homeDir = getenv("HOME");
    static const char LicenseFile[] = ".qt-license";
    char *filename = 0;
    FILE *licenseFile = 0;
    if (homeDir) {
        int sizeEnv = strlen(homeDir);
        int sizeLicense = strlen(LicenseFile);
        filename = (char *)malloc(sizeLicense + sizeEnv + 2);
        strncpy(filename, homeDir, sizeEnv);
        filename[sizeEnv] = '/';
        strncpy(filename + sizeEnv + 1, LicenseFile, sizeLicense);
        licenseFile = fopen(filename, mode);
    }
    return licenseFile;
}


static int validateLicense(const char *string)
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
    if((dir == InstallerDirectionForward) 
       && (licenseStatus != LicenseOK) && !nameCheckOK){ 
        return NO;
    }
    FILE *licenseFile = getQtLicenseFile("w");
    fprintf(licenseFile, "Licensee=\"%s\"\nLicenseKeyExt=%s\n",
            [[nameField stringValue] UTF8String], [fullLicenseKey UTF8String]);
    fclose(licenseFile);
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


