#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "licensefinder.h"

LicenseFinder::LicenseFinder()
{
    searched = false;
    memset(licensee, '\0', sizeof(licensee)*sizeof(char));
    memset(m_key, '\0', sizeof(m_key)*sizeof(char));
    memset(m_oldkey, '\0', sizeof(m_oldkey)*sizeof(char));
    memset(m_customerId, '\0', sizeof(m_customerId)*sizeof(char));
    memset(m_products, '\0', sizeof(m_products)*sizeof(char));
    memset(m_expiryDate, '\0', sizeof(m_expiryDate)*sizeof(char));
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

char *LicenseFinder::getOldLicenseKey()
{
    if (!searched)
        searchLicense();

    return m_oldkey;
}

char *LicenseFinder::getCustomerID()
{
    if (!searched)
        searchLicense();

    return m_customerId;
}

char *LicenseFinder::getProducts()
{
    if (!searched)
        searchLicense();

    return m_products;
}

char *LicenseFinder::getExpiryDate()
{
    if (!searched)
        searchLicense();

    return m_expiryDate;
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
            free(combined);
        }
    }    
}

bool LicenseFinder::lookInDirectory(const char *dir)
{
    FILE *f;
    char file[_MAX_PATH];
    char buf[60000];

    // reset the buffers again, just to be safe :)
    memset(file, '\0', sizeof(file));
    memset(buf, '\0', sizeof(buf));
    memset(licensee, '\0', sizeof(licensee));
    memset(m_key, '\0', sizeof(m_key));
    memset(m_oldkey, '\0', sizeof(m_oldkey));
    memset(m_customerId, '\0', sizeof(m_customerId));
    memset(m_products, '\0', sizeof(m_products));
    memset(m_expiryDate, '\0', sizeof(m_expiryDate));

    if (((strlen(dir)+strlen("\\.qt-license"))*sizeof(char)) >= _MAX_PATH)
        return false;
        
    strcpy(file, dir);
    strcat(file, "\\.qt-license");
    if ((f = fopen(file, "r")) == NULL) 
        return false;

    size_t r = fread(buf, sizeof(char), 59999, f);
    buf[r] = '\0';

    /* Licensee */
    const char *pat1 = "Licensee=\"";
    char *tmp = findPattern(buf, pat1, ulong(r));
    if (tmp && (strlen(tmp) > 1)) {
        char *end = strchr(tmp, '\"');
        if (end && ((end-tmp) < MAX_LICENSEE_LENGTH))
            strncpy(licensee, tmp, end-tmp);
    }

    /* LicenseKey */
    const char *pat2 = "LicenseKeyExt=";
    tmp = findPattern(buf, pat2, ulong(r));
    if (tmp) {
        char *end = strchr(tmp, '\r');
        if (!end)
            end = strchr(tmp, '\n');
        if (end && ((end-tmp) < MAX_KEY_LENGTH))
            strncpy(m_key, tmp, end-tmp);
        else if (strlen(tmp) < MAX_KEY_LENGTH)
            strcpy(m_key, tmp);
    }

    /* OldLicenseKey */
    const char *pat3 = "LicenseKey=";
    tmp = findPattern(buf, pat3, ulong(r));
    if (tmp) {
        char *end = strchr(tmp, '\r');
        if (!end)
            end = strchr(tmp, '\n');
        if (end && ((end-tmp) < MAX_KEY_LENGTH))
            strncpy(m_oldkey, tmp, end-tmp);
        else if (strlen(tmp) < MAX_KEY_LENGTH)
            strcpy(m_oldkey, tmp);
    }

    /* CustomerID */
    const char *pat4 = "CustomerID=\"";
    tmp = findPattern(buf, pat4, ulong(r));
    if (tmp && (strlen(tmp) > 1)) {
        char *end = strchr(tmp, '\"');
        if (end && ((end-tmp) < MAX_QT3INFO_LENGTH))
            strncpy(m_customerId, tmp, end-tmp);
    }

    /* Products */
    const char *pat5 = "Products=\"";
    tmp = findPattern(buf, pat5, ulong(r));
    if (tmp && (strlen(tmp) > 1)) {
        char *end = strchr(tmp, '\"');
        if (end && ((end-tmp) < MAX_QT3INFO_LENGTH))
            strncpy(m_products, tmp, end-tmp);
    }

    /* ExpiryDate */
    const char *pat6 = "ExpiryDate=";
    tmp = findPattern(buf, pat6, ulong(r));
    if (tmp) {
        char *end = strchr(tmp, '\r');
        if (!end)
            end = strchr(tmp, '\n');
        if (end && ((end-tmp) < MAX_QT3INFO_LENGTH))
            strncpy(m_expiryDate, tmp, end-tmp);
        else if (strlen(tmp) < MAX_QT3INFO_LENGTH)
            strcpy(m_expiryDate, tmp);
    }

    fclose(f);

    return true;
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