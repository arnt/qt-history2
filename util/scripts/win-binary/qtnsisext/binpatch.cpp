#include <stdio.h>
#include <string.h>
#include <locale>

#include "binpatch.h"

// returns true if it finds a null termination inside the buffer
long getPathLength(char *data, char *end)
{
    long size = 0;
    while (data < end) {
        if (*data == '\0')
            return size;
        ++data;
        ++size;
    }

    return -1;
}

// returns true if data ends with one of the tokens. (Sep. with ;)
bool endsWithTokens(const char *data, const char *tokens)
{
    if(strlen(tokens) > 0) {
        char endstmp[1024];
        ulong tlen = ulong(strlen(data));

        if(strlen(tokens) >= sizeof(endstmp))
            return false;

        strcpy(endstmp, tokens);

        char *token = strtok(endstmp, ";");

        while(token != NULL) {
            // check if it ends with the token
            if ((tlen >= strlen(token)) 
                && (stricmp((data+tlen)-strlen(token), token) == 0))
                return true;
            token = strtok(NULL, ";");
        }
    } else {
        return true; //true if no tokens
    }

    return false; //no matching tokens
}

bool patchBuffer(char *inbuffer, const char *oldstr, const char *newstr, size_t len, long *rw, bool useLength, char *endTokens)
{
    char nc1 = *oldstr;
    char nc2 = 0;
    char *inend = inbuffer + len;
    size_t oldlen = strlen(oldstr);
    size_t newlen = strlen(newstr);
    char *instart = inbuffer;
    *rw = 0;
    bool write = true;

    isupper(nc1) ? nc2 = tolower(nc1) : nc2 = toupper(nc1);

    while(inbuffer < inend) {
        if ((*inbuffer == nc1) || (*inbuffer == nc2)) {
            if (inbuffer > (inend-oldlen) || inbuffer > (inend-newlen)) {
                *rw = (long)(inend-inbuffer); //rewind, not enough to make a compare
                break;
            }

            if (strnicmp(inbuffer, oldstr, oldlen) == 0) {
                if (useLength && (instart == inbuffer)) {
                    *rw = (long)(len+1); //we don't have access to the length byte, rewind all + 1!
                    write = false;
                    break;
                }
                
                long oldsize = -1;
                if (useLength) { //VC60
                    oldsize = *(inbuffer-1);

                    // vc60 pdb files sometimes uses 0A, then it should be null terminated
                    if (oldsize < (long)oldlen) {
                        if (oldsize != 0x0A) { //strange case... skip
                            inbuffer+=oldlen;
                            continue;
                        }

                        oldsize = getPathLength(inbuffer, inend);
                        
                        if (oldsize < 0) {
                            *rw = (long)(inend-inbuffer); //rewind, entire string not in buffer
                            break;
                        }
                    }

                    if (inbuffer > (inend-oldsize)) {
                        *rw = (long)(inend-inbuffer); //rewind, entire string not in buffer
                        break;
                    }
                } else { //VC7x
                    oldsize = getPathLength(inbuffer, inend);
                    if (oldsize < 0) {
                        *rw = (long)(inend-inbuffer); //rewind, entire string not in buffer
                        break;
                    }
                }

                char oldPath[1024];
                if (oldsize > sizeof(oldPath))
                    break; //at least don't crash
                memset(oldPath, '\0', sizeof(oldPath));
                strncpy(oldPath, newstr, newlen);

                strncat(oldPath, inbuffer+oldlen, oldsize-oldlen);

                // just replace if it ends with a token in endTokens
                if (endsWithTokens(oldPath, endTokens)) {
                    if (oldsize < (long)strlen(oldPath))
                        oldsize = (long)strlen(oldPath);

                    memcpy(inbuffer, oldPath, oldsize);
                }

                inbuffer+=oldsize;
                continue;
            }
        }
        ++inbuffer;
    }

    return write;
}

bool BinPatch::patchBinaryFile(const char *fileName, const char *oldstr, const char *newstr, bool useLength, char *endTokens)
{
    size_t oldlen = strlen(oldstr);

    if ((!fileName || strlen(fileName) < 1)
        || (!oldstr || oldlen < 1)
        || (!newstr || strlen(newstr) < 1))
        return false;

    FILE *input;

    if (!(input = fopen(fileName, "r+b")))
    {
        fprintf(stderr, "Cannot open file %s!\n", fileName);
        return false;
    }

    char data[60000];
    long rw = 0;
    long offset = 0;

    size_t len;
    len = fread(data, sizeof(char), sizeof(data), input);

    do {
        if (patchBuffer(data, oldstr, newstr, len, &rw, useLength, endTokens)) {
            fseek(input, offset, SEEK_SET); //overwrite
            fwrite(data, sizeof(char), len, input);
        }

        offset += (long)((-rw) + len);
        if (fseek(input, offset, SEEK_SET) != 0)
            break;
        len = fread(data, sizeof(char), sizeof(data), input);
    } while(!feof(input) && (len >= oldlen));
    
    fclose(input);
    
    return true;
}
