#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "licensefinder.h"

LicenseFinder::LicenseFinder()
{
    searched = false;
    memset(licensee, '\0', sizeof(licensee)*sizeof(char));
    memset(m_key, '\0', sizeof(m_key)*sizeof(char));
}

char *LicenseFinder::getLicensee()
{
    if (!searched)
        searchLicense();

    return licensee;
}

char *LicenseFinder::getLicenseKey()
{
    if (!searched)
        searchLicense();

    return m_key;
}

void LicenseFinder::searchLicense()
{
    searched = true;
    char *path = getenv("HOME");
    if (path && lookInDirectory(path))
        return;

    path = getenv("USERPROFILE");
    if (path && lookInDirectory(path))
        return;

    path = getenv("HOMEDRIVE");
    if (path) {
        char *dir = getenv("HOMEPATH");
        if (dir) {
            char *combined = (char*)malloc(sizeof(char)*(strlen(path) + strlen(dir) + 1));
            strcpy(combined, path);            
            strcat(combined, dir);
            lookInDirectory(combined);
            delete combined;
        }
    }    
}

bool LicenseFinder::lookInDirectory(const char *dir)
{
    char *file = (char*)malloc(sizeof(char)*(strlen(dir) + 13));
    strcpy(file, dir);
    strcat(file, "\\.qt-license");

    FILE *f;
    if ((f = fopen(file, "r")) != NULL) {
        char buf[60000];
        size_t r = fread(buf, sizeof(char), 60000, f);
        buf[r] = '\0';

        /* Licensee */
        char *pat = "Licensee=\"";
        char *tmp = findPattern(buf, pat, ulong(r));

        if (tmp && (strlen(tmp) > 1)) {
            char *end = strchr(tmp, '\"');
            if (end)
                strncpy(licensee, tmp, end-tmp);
        }

        /* LicenseKey */
        char *n = "LicenseKeyExt=";        
        char *key = findPattern(buf, n, ulong(r));

        if (key) {
            strcpy(m_key, key);
            delete file;
            return true;
        }
    }
    delete file;
    return false;
}

/* copied from binpatch.cpp */
char *LicenseFinder::findPattern(char *h, const char *n, ulong hlen)
{
    if (!h || !n || hlen == 0)
	return 0;

    ulong nlen;

    char nc = *n++;
    nlen = ulong(strlen(n));
    char hc;

    do {
        do {
            hc = *h++;
            if (hlen-- < 1)
                return 0;
        } while (hc != nc);
        if (nlen > hlen)
            return 0;
    } while (strncmp(h, n, nlen) != 0);
    return h + nlen;
}