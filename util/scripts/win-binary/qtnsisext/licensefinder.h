#ifndef LICENSEFINDER_H
#define LICENSEFINDER_H

class LicenseFinder
{
public:
    LicenseFinder();
    char *getLicenseKey(int part);

private:
    void searchLicense();
    bool lookInDirectory(const char* dir);
    bool searched;
    char key1[5];
    char key2[5];
    char key3[5];
};

#endif