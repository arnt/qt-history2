/* InstallerPanePane */

#import <InstallerPlugins/InstallerPlugins.h>
#import <Cocoa/Cocoa.h>

enum LicenseValues { LicenseOK = 0, InvalidLicense, InvalidProduct, InvalidPlatform, InvalidType };

@interface InstallerPanePane : InstallerPane
{
    int licenseStatus;
    BOOL nameCheckOK;
    NSMutableString *fullLicenseKey;
    IBOutlet NSTextField *LicenseField1;
    IBOutlet NSTextField *LicenseField2;
    IBOutlet NSTextField *LicenseField3;
    IBOutlet NSTextField *LicenseField4;
    IBOutlet NSTextField *LicenseField5;
    IBOutlet NSTextField *LicenseField6;
    IBOutlet NSTextField *LicenseField7;
    IBOutlet NSTextField *nameField;
    IBOutlet NSTextField *errorField;
}    
- (void)checkLicense;
- (void)checkName;
- (void)tryEnable;
@end
