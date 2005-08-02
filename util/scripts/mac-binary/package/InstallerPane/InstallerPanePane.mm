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
        
        // Copy the file first
        NSFileManager *fm = [NSFileManager defaultManager];
        NSString *qtLicense = [NSHomeDirectory() stringByAppendingPathComponent: @".qt-license"];
        
        
        if ([fm fileExistsAtPath:qtLicense]) {
            NSMutableString *alternatePlace = [NSMutableString stringWithCapacity: 256];
            [alternatePlace setString : [NSHomeDirectory() stringByAppendingPathComponent: @".qt-license.bak"]];
            NSNumber *backNumber = [NSNumber numberWithInt:1];
            int index = [alternatePlace length] - 1;
            
            while ([fm fileExistsAtPath:alternatePlace]) {
                [alternatePlace insertString:[backNumber stringValue] atIndex:index];
                int foo = [backNumber intValue];
                ++foo;
                backNumber = [NSNumber numberWithInt:foo];
            }
            [fm copyPath:qtLicense toPath:alternatePlace handler:nil];
        }

        NSMutableString *finalString = [NSMutableString stringWithCapacity:256];
        [finalString appendString:[NSString stringWithUTF8String: LicenseeString]];
        [finalString appendString:[nameField stringValue]];
        [finalString appendString:@"\n"];
        [finalString appendString:[NSString stringWithUTF8String: LicenseKeyExtString]];
        [finalString appendString:fullLicenseKey];
        [finalString appendString:@"\n"];
            
        const char *finalFinalString = [finalString UTF8String];
        
        NSData *data = [NSData dataWithBytes:finalFinalString length:strlen(finalFinalString)];
        [fm createFileAtPath:qtLicense contents:data attributes:nil];
    }
    return YES; 
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


