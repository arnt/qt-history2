#include <stdio.h>
#include <qglobal.h>
#include <qstring.h>
#include <qfile.h>
#include <Files.h>
#include "Resources.h"
#include "Script.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

bool
createFork(const QString &f)
{
    qDebug("Creating fork for %s", f.latin1());

    FSRef fref;
    FSSpec fileSpec;
    if(QFile::exists(f)) {
        mode_t perms = 0;
        {
            struct stat s;
            if(stat(f.latin1(), &s) == -1) {
		qDebug("Fatal!");
		return FALSE;
	    }
            if(!(s.st_mode & S_IWUSR)) {
                perms = s.st_mode;
                if(chmod(f.latin1(), perms | S_IWUSR) == -1) {
		    qDebug("%d: Fatal:%d: %s", __LINE__, errno, strerror(errno));
		    return FALSE;
		}
            }
        }
        FILE *o = fopen(f.latin1(), "a");
	qDebug("%d %s (%d)", o, f.latin1(), errno);
        if(!o) {
	    qDebug("%d: Fatal:%d: %s", __LINE__, errno, strerror(errno));
            return FALSE;
	}
        if(FSPathMakeRef((const UInt8 *)f.latin1(), &fref, NULL) == noErr) {
            if(FSGetCatalogInfo(&fref, kFSCatInfoNone, NULL, NULL, &fileSpec, NULL) == noErr)
                FSpCreateResFile(&fileSpec, 'CUTE', 'TEXT', smSystemScript);
            else
                qDebug("bogus %d", __LINE__);
        } else
            qDebug("bogus %d", __LINE__);
	if(o)
	    fclose(o);
        if(perms)
            chmod(f.latin1(), perms);
    }
    return TRUE;
}

int
main(int argc, char **argv)
{
	for(int i = 1; i < argc; i++) 
		createFork(argv[i]);
	return 1;
}
