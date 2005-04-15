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

long replaceString(char *data, const char *oldstr, const char *newstr, const char *endsWith, ulong len)
{
    long changed = -1;
    char nc1 = *oldstr++;
    char nc2;
    char hc;
    char tmp[1024];
    ulong nlen = ulong(strlen(oldstr));

    isupper(nc1) ? nc2 = tolower(nc1) : nc2 = toupper(nc1);

    while(len >= (nlen+1)) {
        hc = *data++;
        if((hc == nc1) || (hc == nc2)) {
            if(strnicmp(data, oldstr, nlen) == 0) {
                if(!terminatesInBufferScope(data-1, len))
                    return len;
                
                // replace
                if (strlen(data+nlen) >= sizeof(tmp)) break; //buffer to small, don't care :|
                strcpy(tmp, data+nlen); //copy rest of string into tmp
                ulong tlen = ulong(strlen(tmp)); //get length of tmp

                if (endsWithTokens(tmp, endsWith)) {
                    ulong slen = ulong(strlen(newstr)); //get length of newstr
                    strncpy(data-1, newstr, slen); //copy new string into buffer
                    strcpy((data-1)+slen, tmp); //append tmp
                    memset((data-1)+slen+tlen, '\0', (nlen+1)-slen); //pad rest with null
                    changed = 0;
                }
            }
        }
        len--;
    }

    return changed;
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

    while (!feof(input)) {
        ulong len = ulong(fread(data, sizeof(char), sizeof(data), input));
        if (len < olen)
            break;
        
        int res = replaceString(data, oldstr, newstr, endsWith, len);
        if (res > 0) { //entire string was not in buffer
            offset += len - res;
            fseek(input, offset, 0);
            continue;
        } else if (res == 0) { //write buffer
            fseek(input, offset, 0);
            fwrite(data, sizeof(char), len, input);
        }

        // move to the new read position (back up a bit to get everything)
        offset += (len - olen) + 1;
        fseek(input, offset, 0);        
    }

    fclose(input);
    return true;
}
