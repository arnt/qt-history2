#include <stdio.h>
#include <string.h>

#include "binpatch.h"

BinPatch::BinPatch()
{
}

char *BinPatch::findPattern(char *h, const char *n, ulong hlen)
{
    if (!h || !n || hlen == 0)
	return 0;

#ifdef Q_OS_UNIX
    size_t nlen;
#else
    ulong nlen;
#endif

    char nc = *n++;
    nlen = strlen(n);
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

bool BinPatch::patchFile(const char *fileName, const char *qtPath)
{
    if (!fileName && strlen(fileName) < 1
        || !qtPath && strlen(qtPath) < 1)
        return false;

    struct step {
	const char *key;
	char *value;
	bool done;
    } steps[8];

    steps[0].key = "qt_nstpath=";
    steps[0].value = const_cast<char *>(qtPath);
    steps[0].done = false;

    steps[1].key = "qt_binpath=";
    steps[1].value = new char[strlen(qtPath) + 5];
    strcpy(steps[1].value, qtPath);
    strcat(steps[1].value, "\\bin");
    steps[1].value[strlen(steps[1].value)] = '\0';
    steps[1].done = false;

    steps[2].key = "qt_docpath=";
    steps[2].value = new char[strlen(qtPath) + 5];
    strcpy(steps[2].value, qtPath);
    strcat(steps[2].value, "\\doc");
    steps[2].value[strlen(steps[2].value)] = '\0';
    steps[2].done = false;

    steps[3].key = "qt_hdrpath=";
    steps[3].value = new char[strlen(qtPath) + 9];
    strcpy(steps[3].value, qtPath);
    strcat(steps[3].value, "\\include");
    steps[3].value[strlen(steps[3].value)] = '\0';
    steps[3].done = false;

    steps[4].key = "qt_libpath=";
    steps[4].value = new char[strlen(qtPath) + 5];
    strcpy(steps[4].value, qtPath);
    strcat(steps[4].value, "\\lib");
    steps[4].value[strlen(steps[4].value)] = '\0';
    steps[4].done = false;

    steps[5].key = "qt_plgpath=";
    steps[5].value = new char[strlen(qtPath) + 9];
    strcpy(steps[5].value, qtPath);
    strcat(steps[5].value, "\\plugins");
    steps[5].value[strlen(steps[5].value)] = '\0';
    steps[5].done = false;

    steps[6].key = "qt_datpath=";
    steps[6].value = const_cast<char *>(qtPath);
    steps[6].done = false;

    steps[7].key = "qt_trnpath=";
    steps[7].value = new char[strlen(qtPath) + 14];
    strcpy(steps[7].value, qtPath);
    strcat(steps[7].value, "\\translations");
    steps[7].value[strlen(steps[7].value)] = '\0';
    steps[7].done = false;

    uint completed = 0;
    uint total_steps = sizeof(steps) / sizeof(step);

    FILE *input;

    if (!(input = fopen(fileName, "r+b")))
    {
        fprintf(stderr, "Cannot open file %s!\n", fileName);
        return false;
    }

    char data[60000];
	ulong offset = 0;

    while (!feof(input) && completed < total_steps) {
        ulong len = fread(data, sizeof(char), sizeof(data), input);
        if (len < 267)
            break;

        uint completed_save = completed;
        for (uint x = 0; x < total_steps; ++x) {
            if (steps[x].done) continue;

            char *s = BinPatch::findPattern(data, steps[x].key, len);
            if (s) {
                ulong where = s - data;
                if (len - where < 256) {
                    // not enough space left to write the full
                    // path... move the file pointer back to just
                    // before the pattern and continue
                    offset += where - 11;
                    fseek(input, offset, 0);                    
                    len = fread(data, sizeof(char), sizeof(data), input);
                    --x; // retry the current step
                    continue;
                }

                strncpy(s, steps[x].value, 256);
                steps[x].done = true;
                ++completed;
            }
        }

        if (completed != completed_save) {
            // something changed...  move file pointer back to
            // where the data was read and write the new data
            fseek(input, offset, 0);
            fwrite(data, sizeof(char), len, input);
            
        }

        // move to the new read position
        offset += len - 11;
        fseek(input, offset, 0);        
    }
    fclose(input);
    return true;
}

void BinPatch::checkLibData(const char *fileName)
{
    if (!fileName && strlen(fileName) < 1)
        return;

    struct step {
	const char *key;
	bool done;
    } steps[8];

    steps[0].key = "qt_nstpath=";
    steps[0].done = false;

    steps[1].key = "qt_binpath=";
    steps[1].done = false;

    steps[2].key = "qt_docpath=";
    steps[2].done = false;

    steps[3].key = "qt_hdrpath=";
    steps[3].done = false;

    steps[4].key = "qt_libpath=";
    steps[4].done = false;

    steps[5].key = "qt_plgpath=";
    steps[5].done = false;

    steps[6].key = "qt_datpath=";
    steps[6].done = false;

    steps[7].key = "qt_trnpath=";
    steps[7].done = false;

    uint completed = 0;
    uint total_steps = sizeof(steps) / sizeof(step);

    FILE *file;

    if (!(file = fopen(fileName, "rb"))) {
        fprintf(stderr, "Cannot open file.");
        return;
    }
    
	// instead of reading in the entire file, do the search in chunks
	char data[60000];
	ulong offset = 0;

	while (!feof(file) && completed < total_steps) {
        ulong len = fread(data, sizeof(char), sizeof(data), file);
        if (len < 267) {
            // not enough room to make any modifications... stop
            break;
        }

        for (uint x = 0; x < total_steps; ++x) {
            if (steps[x].done) continue;

            char *s = BinPatch::findPattern(data, steps[x].key, len);
            if (s) {
                ulong where = s - data;
                if (len - where < 256) {
                    // not enough space left to write the full
                    // path... move the file pointer back to just
                    // before the pattern and continue
                    offset += where - 11;
                    fseek(file, offset, 0);
                    len = fread(data, sizeof(char), sizeof(data), file);
                    --x; // retry the current step
                    continue;
                }
                fprintf(stdout, "%s%s\n", steps[x].key, s);
                steps[x].done = true;
                ++completed;
            }
        }

	    // move to the new read position
	    offset += len - 11;
        fseek(file, offset, 0);
	}
    fclose(file);
}