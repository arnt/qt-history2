#ifndef BINPATCH_H
#define BINPATCH_H

typedef unsigned long ulong;
typedef unsigned int uint;

class BinPatch
{
public:
    BinPatch();
    void checkLibData(const char *fileName);
    bool patchFile(const char *fileName, const char *qtPath);

private:
    char *find_pattern(char *h, const char *n, ulong hlen);
};

#endif