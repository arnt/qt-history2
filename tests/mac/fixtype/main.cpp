#include <stdio.h>
#include <qglobal.h>
#include <qstring.h>
#include <qfile.h>
#include <Files.h>
#include "Resources.h"
#include "Script.h"
#include <sys/types.h>
#include <sys/stat.h>

bool
createFork(const QString &f)
{
    FSRef fref;
    FSSpec fileSpec;
    if(QFile::exists(f)) {
        mode_t perms = 0;
        {
            struct stat s;
            stat(f.latin1(), &s);
            if(!(s.st_mode & S_IWUSR)) {
                perms = s.st_mode;
                chmod(f.latin1(), perms | S_IWUSR);
            }
        }
        FILE *o = fopen(f.latin1(), "a");
        if(!o)
            return FALSE;
        if(FSPathMakeRef((const UInt8 *)f.latin1(), &fref, NULL) == noErr) {
            if(FSGetCatalogInfo(&fref, kFSCatInfoNone, NULL, NULL, &fileSpec, NULL) == noErr)
                FSpCreateResFile(&fileSpec, 'CUTE', 'TEXT', smSystemScript);
            else
                qDebug("bogus %d", __LINE__);
        } else
            qDebug("bogus %d", __LINE__);
        fclose(o);
        if(perms)
            chmod(f.latin1(), perms);
    }
    return TRUE;
}

int
main(int argc, char **argv)
{
	for(int i = 1; i < argc; i++) {
		qDebug("%s", argv[i]);
		createFork(argv[i]);
	}
	return 1;
}
