/****************************************************************************
**
** Implementation of QFSDirEngine class for Windows
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <qfileengine.h>
#include "qfileengine_p.h"
#include <qplatformdefs.h>
#include <qfile.h>

#define d d_func()
#define q q_func()

QString qt_fixToQtSlashes(const QString &path)
{
    if (!path.length())
        return path;

    QString ret;
    for (int i=0, lastSlash=-1; i<(int)path.length(); i++) {
	if(path[i] == '/' || path[i] == '\\') {
	    if(i <= 3 || 
	       (i < path.length() - 1) && (lastSlash == -1 || i != lastSlash+1))
		ret += '/';
	    lastSlash = i;
	} else {
	    ret += path[i];
	}
    }
    return ret;
}

QByteArray qt_win95Name(const QString s)
{
    QString ss(s);
    if (s[0] == '/' && s[1] == '/') {
        // Win95 cannot handle slash-slash needs slosh-slosh.
        ss[0] = '\\';
        ss[1] = '\\';
        int n = ss.indexOf('/');
        if (n >= 0)
            ss[n] = '\\';
        return ss.toLocal8Bit();
    } else if (s.length() > 3 && s[2] == '/' && s[3] == '/') {
        ss[2] = '\\';
        ss.remove(3, 1);
        int n = ss.indexOf('/');
        if (n >= 0)
            ss[n] = '\\';
    }
    return ss.toLocal8Bit();
}

bool isValidFile(const QString& fileName)
{
    // Only : needs to be checked for, other invalid characters
    // are currently checked by fopen()
    int findColon = fileName.lastIndexOf(':');
    if (findColon == -1)
        return true;
    else if (findColon != 1)
        return false;
    else
        return fileName[0].isLetter();
}

void 
QFSFileEnginePrivate::init()
{
}

int
QFSFileEnginePrivate::sysOpen(const QString &fileName, int flags)
{
    QT_WA({
	return ::_wopen((TCHAR*)fileName.utf16(), flags, 0666);
    } , {
	return QT_OPEN(qt_win95Name(fileName), flags, 0666);
    });
}
    
bool
QFSFileEngine::remove()
{
    QT_WA({
        return ::_wremove((TCHAR*)d->file.utf16()) == 0;
    } , {
        return ::remove(qt_win95Name(d->file)) == 0;
    });
}

bool 
QFSFileEngine::rename(const QString &newName)
{
    QT_WA({
        return ::_wrename((TCHAR*)d->file.utf16(), (TCHAR*)newName.utf16()) == 0;
    } , {
        return ::rename(qt_win95Name(d->file), qt_win95Name(newName)) == 0;
    });
}

QFile::Offset
QFSFileEngine::size() const
{
    QT_STATBUF st;
    int ret = 0;
    if (isOpen()) {
        ret = QT_FSTAT(d->fd, &st);
    } else {
        QT_WA({
            ret = QT_TSTAT((TCHAR*)d->file.utf16(), (QT_STATBUF4TSTAT*)&st);
        } , {
            ret = QT_STAT(qt_win95Name(d->file), &st);
        });
    }
    if (ret == -1)
        return 0;
    return st.st_size;
}

uchar
*QFSFileEngine::map(Q_LONG /*len*/)
{
    return 0;
}




