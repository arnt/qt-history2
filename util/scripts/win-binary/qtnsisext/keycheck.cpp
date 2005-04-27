#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "keycheck.h"

static const char alphabet[] = "X9MUEC7AJH3KS6DB4YFG2L5PQRT8VNZ";
static const int AlphabetSize = 31;

KeyCheck::KeyCheck(const char *key1, const char *key2, const char *key3)
{
    keysOK = true;

    if (key1 && strlen(key1) == 4) {
        strcpy(k1, key1);
        k1[4] = '\0';
    }
    else
        keysOK = false;

    if (key2 && strlen(key2) == 4) {
        strcpy(k2, key2);
        k2[4] = '\0';
    }
    else
        keysOK = false;

    if (key3 && strlen(key3) == 4) {
        strcpy(k3, key3);
        k3[4] = '\0';
    }
    else
        keysOK = false;
}

bool KeyCheck::isValidWindowsLicense()
{
    if (!keysOK)
        return false;
    unsigned int features = featuresForKey();    
    if (features == 0)
        return false;
    if ((features & Feature_Windows) != 0)
        return true;
    return false;
}

bool KeyCheck::usesUSLicense()
{
    if (!keysOK)
        return false;
    unsigned int features = featuresForKey();
    if ((features & Feature_US) != 0)
        return true;    
    return false;
}

uint KeyCheck::featuresForKey()
{
    uint code = codeForKey();
    uint features = featuresForCode(code);
    uint check = checkForCode(code);

    for (uint bits = 0; bits < (1 << NumRandomBits); bits++) {
	    if ( check == makeCheck(features, bits) )
	        return features;
    }
    return 0;
}

uint KeyCheck::codeForKey()
{
    int len = int(strlen(k1) + strlen(k2));
    char *u = new char[sizeof(char)*len];
    strcpy(u, k1+1);
    strcat(u, k2);
    u[len-1] = '\0';
    uint code = decodeBaseZ(u);
    delete u;
    
    char t[2];    
    t[0] = k1[0];
    t[1] = '\0';
    int i = atoi(t);
    if (i == 0 && t[0] != '0') {
        if (t[0]  == 'A')
            i = 10;
        else if (t[0] == 'B')
            i = 11;
        else if (t[0] == 'C')
            i = 12;
        else if (t[0] == 'D')
            i = 13;
        else if (t[0] == 'E')
            i = 14;
        else if (t[0] == 'F')
            i = 15;
        else if (t[0] == 'G')
            i = 16;
        else if (t[0] == 'H')
            i = 17;
        else
            return 0;
    }   
    uint extra = i - 2;
    if (((featuresForCode(code) ^ extra) &
        (Feature_US | Feature_Enterprise | Feature_Unix)) == 0 )
            return code;
    return 0;  
}

uint KeyCheck::decodeBaseZ(const char *str)
{
    uint k = 0;
    int i = int(strlen(str));
    while ( i > 0 ) {
        i--;
        const char *p = strchr(alphabet, str[i]);
        if (p == 0) {
            return 0;
        } else {
            k = (k*AlphabetSize) + (p-alphabet);
        }
    }
    return k;
}

uint KeyCheck::shuffledBits(uint bits)
{
    const uint OddPrime = 23;
    uint shuf = 0;

    for (int i = 0; i < 32; i++) {
        shuf = (shuf << 1) | (bits & 0x1);
        bits = (bits >> OddPrime) | (bits << (32 - OddPrime));
    }
    return shuf;
}

uint KeyCheck::succ(uint x)
{
    return 3141592621u * ( x ^ 0xa5a5a5a5 ) + 1;
}

uint KeyCheck::makeCheck(uint features, uint randomBits)
{
    uint s = succ((features << NumRandomBits) | (randomBits & RandomBitMask));
    return shuffledBits(s) & CheckMask;
}

uint KeyCheck::makeCode(uint features, uint check)
{
    return (check << NumFeatures) | features;
}

uint KeyCheck::featuresForCode(uint code)
{
    return code & FeatureMask;
}

uint KeyCheck::checkForCode(uint code)
{
    return code >> NumFeatures;
}

int KeyCheck::getNumberOfDays()
{
    uint y = decodeBaseZ(k3);
    y = y ^ 0x0000beef;
	int days = ((y >> 7) ^ y) >> 7;

    if (days > 4000)
        return 0; 
    return days;
}