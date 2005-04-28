#include <stdio.h>
#include <string.h>
#include <locale>

#include "binpatch.h"

// returns true if it finds a null termination inside the buffer
bool terminatesInBufferScope(char *data, ulong len)
{
    while(len > 0) {
        if (*data == '\0')
            return true;
        data++;
        len--;
    }

    return false;
}

// returns true if data ends with one of the tokens. (Sep. with ;)
bool endsWithTokens(const char *data, const char *tokens)
{
    if(strlen(tokens) > 0) {
        char endstmp[1024];
        ulong tlen = ulong(strlen(data));

        if(strlen(tokens) >= sizeof(endstmp)) return false;
        strcpy(endstmp, tokens);

        char *token = strtok(endstmp, ";");
        while(token != NULL)
        {
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

bool replaceString(char *data, const char *oldstr, const char *newstr, const char *endsWith, ulong len, ulong *offset)
{
    bool changed = false;
    char nc1 = *oldstr++;
    char nc2;
    char hc;
    char tmp[1024];
    ulong nlen = ulong(strlen(oldstr));
    *offset = 0;

    isupper(nc1) ? nc2 = tolower(nc1) : nc2 = toupper(nc1);

    while(len >= (nlen+1)) {
        hc = *data++;
        if((hc == nc1) || (hc == nc2)) {
            if(strnicmp(data, oldstr, nlen) == 0) {
                if(!terminatesInBufferScope(data-1, len)) {
                    *offset = len;
                    return changed;
                }
                
                // replace
                if (strlen(data+nlen) >= sizeof(tmp)) break; //buffer to small, don't care :|
                strcpy(tmp, data+nlen); //copy rest of string into tmp
                ulong tlen = ulong(strlen(tmp)); //get length of tmp

                if (endsWithTokens(tmp, endsWith)) {
                    ulong slen = ulong(strlen(newstr)); //get length of newstr
                    strncpy(data-1, newstr, slen); //copy new string into buffer
                    strcpy((data-1)+slen, tmp); //append tmp
                    if((nlen+1) > slen)
                        memset((data-1)+slen+tlen, '\0', (nlen+1)-slen); //pad rest with null
                    changed = true;

                    //fast forward...
                    data += strlen(data);
                    len -= strlen(data);
                }
            }
        }
        len--;
    }

    return changed;
}

size_t insertString(char *inbuffer, char *outbuffer, const char *oldstr, const char *newstr, size_t len, size_t *rw, bool length)
{
    char nc1 = *oldstr;
    char nc2 = 0;
    char *inend = inbuffer + len;
    size_t oldlen = strlen(oldstr);
    char *outstart = outbuffer;
    *rw = 0;

    isupper(nc1) ? nc2 = tolower(nc1) : nc2 = toupper(nc1);

    while(inbuffer < inend) {
        if ((*inbuffer == nc1) || (*inbuffer == nc2)) {
            if (inbuffer > (inend-oldlen)) {
                *rw = (inend-inbuffer); //rewind
                break;
            }

            if (strnicmp(inbuffer, oldstr, oldlen) == 0) {
                //insert
                size_t newlen = strlen(newstr);
                strncpy(outbuffer, newstr, newlen);

                if (length) {
                    if (outstart == outbuffer) {
                        //we don't have access to the length byte, rewind all + 1!
                        *rw = 1;
                        break;
                    }
                    char oldsize = *(inbuffer-1);
                    //pdb files only support 256 characters it seems :|
                    *(outbuffer-1) = (char)(oldsize-(oldlen-newlen));
                }

                outbuffer+=newlen;
                inbuffer+=oldlen;

                continue;
            }
        }

        //copy char
        *outbuffer = *inbuffer;
        ++outbuffer;
        ++inbuffer;
    }

    return (outbuffer - outstart);
}

bool BinPatch::patchFileInsert(const char *inName, const char *outName, const char *oldstr, const char *newstr, bool updateLength)
{
    size_t olen = strlen(oldstr);
    size_t nlen = strlen(newstr);

    if ((!inName && strlen(inName) < 1) ||
        (!outName && strlen(outName) < 1)
        || !oldstr && olen < 1
        || !newstr && nlen < 1)
        return false;

    FILE *input;
    FILE *output;

    if (!(input = fopen(inName, "rb")))
    {
        fprintf(stderr, "Cannot open file %s!\n", inName);
        return false;
    }

    if (!(output = fopen(outName, "wb")))
    {
        fprintf(stderr, "Cannot open file %s!\n", outName);
        fclose(input);
        return false;
    }

    //the diff between new and old should not be longer than 1000
    char inbuffer[7];
    char outbuffer[7];
    
    size_t inoffset = 0;
    size_t rw = 0;
    size_t outsize = 0;

    while (!feof(input)) {
        size_t len = fread(inbuffer, sizeof(char), sizeof(inbuffer), input);
        
        if (len < olen)
            break;
        
        outsize = insertString(inbuffer, outbuffer, oldstr, newstr, len, &rw, updateLength);
        if (rw == 1 && outsize == 0) {
            //rewind everything by one!
            fseek(input, -((long)len+1), SEEK_CUR);
            fseek(output, -1, SEEK_CUR);
            continue;
        }

        fwrite(outbuffer, sizeof(char), outsize, output);

        if (rw > 0) //entire string was not in buffer (rewind)
            fseek(input, -(long)rw, SEEK_CUR);
    }

    fclose(input);
    fclose(output);
    return true;
}

bool BinPatch::patchFile(const char *fileName, const char *oldstr, const char *newstr, const char *endsWith)
{
    ulong olen = ulong(strlen(oldstr));
    ulong nlen = ulong(strlen(newstr));

    if (!fileName && strlen(fileName) < 1
        || !oldstr && olen < 1
        || !newstr && nlen < 1)
        return false;

    FILE *input;

    if (!(input = fopen(fileName, "r+b")))
    {
        fprintf(stderr, "Cannot open file %s!\n", fileName);
        return false;
    }

    char data[60000];
    ulong offset = 0;
    ulong stroffset = 0;

    while (!feof(input)) {
        ulong len = ulong(fread(data, sizeof(char), sizeof(data), input));
        if (len < olen)
            break;
        
        if(replaceString(data, oldstr, newstr, endsWith, len, &stroffset)) {
            fseek(input, offset, 0);
            fwrite(data, sizeof(char), len, input);
        }

        if (stroffset > 0) { //entire string was not in buffer
            offset += len - stroffset;
            fseek(input, offset, 0);
        } else { // move to the new read position (back up a bit to get everything)
            offset += (len - olen) + 1;
            fseek(input, offset, 0);
        }
    }

    fclose(input);
    return true;
}
