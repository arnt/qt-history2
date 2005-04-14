#ifndef BINPATCH_H
#define BINPATCH_H

typedef unsigned long ulong;
typedef unsigned int uint;

namespace BinPatch
{
    bool patchFile(const char *fileName, const char *oldstr, const char *newstr, const char *endsWith);
}

#endif