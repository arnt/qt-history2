#ifndef KEYCHECK_H
#define KEYCHECK_H

#define MASK(n) ((1 << (n)) - 1)

typedef unsigned int uint;

class KeyCheck
{
public:
    KeyCheck(const char *key1, const char *key2, const char *key3);
    
    bool isValidWindowsLicense();
    bool usesUSLicense();

private:
    enum { Feature_US = 0x1, Feature_Enterprise = 0x2, Feature_Unix = 0x4,
        Feature_Embedded = 0x8, Feature_Windows = 0x10, Feature_Mac = 0x20,
        Feature_Extra1 = 0x40, Feature_Extra2 = 0x80,
        NumFeatures = 8,
        FeatureMask = MASK( NumFeatures ) };

    enum { CheckMask = MASK( 32 - NumFeatures ) };

    enum { NumRandomBits = 12,
        RandomBitMask = MASK( NumRandomBits ) };

    uint featuresForKey();
    uint codeForKey();
    uint decodeBaseZ(const char *str);
    uint shuffledBits(uint bits);
    uint succ(uint x);
    uint makeCheck(uint features, uint randomBits);
    uint makeCode(uint features, uint check);
    uint featuresForCode(uint code);
    uint checkForCode(uint code);

private:
    char k1[5];
    char k2[5];
    char k3[5];
    bool keysOK;
};

#endif