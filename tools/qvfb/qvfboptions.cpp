/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qvfboptions.h"
#include <QRegExp>
#include <QVariant>
#include <QSettings>
#include <stdlib.h>
#include <stdio.h>

QVFbOptions *qvfbOptions = 0;

QVFbOptions::QVFbOptions(int &argc, char **argv)
    : mCursor(true), mProtocol(0)
{
    qvfbOptions = this;
    /* first check if options are valid before actually setting anything */
    int i;
    int invalidArgs = 1;
    bool useDefaults = false;
    for ( i = 1; i < argc; i++ ){
	QString arg = argv[i];
	if ( arg == "-width" || arg == "-height" || arg == "-depth"
                || arg == "-zoom" || arg == "-qwsdisplay" || arg == "-skin") {
            i++;
            continue;
        }
        if (arg == "-mmap" || arg == "-nocursor" || arg == "-defaults") 
            continue;
        /* error: later, add to 'failed to parse' and continue */
        char * targ = argv[invalidArgs];
        argv[invalidArgs] = argv[i];
        argv[i] = targ;
        invalidArgs++;
    }
    if (invalidArgs > 1) {
        argc = invalidArgs;
        return;
    }

    if (useDefaults) {
        setWidth(240);
        setHeight(320);
        setDepth(32);
        setZoom(1.0);
        setSkin(QString());
        setDisplay(0);
        setRotation(0);
    }

    for ( i = 1; i < argc; i++ ){
        QString arg = argv[i];
	if ( arg == "-width" ) {
	    setWidth(atoi(argv[++i]));
	} else if ( arg == "-height" ) {
	    setHeight(atoi(argv[++i]));
	} else if ( arg == "-skin" ) {
	    setSkin(argv[++i]);
	} else if ( arg == "-depth" ) {
	    setDepth(atoi(argv[++i]));
	} else if ( arg == "-nocursor" ) {
	    setCursor(false);
	} else if ( arg == "-mmap" ) {
	    setProtocol(1);
	} else if ( arg == "-zoom" ) {
	    setZoom(atof(argv[++i]));
	} else if ( arg == "-qwsdisplay" ) {
            QString spec = argv[i];
            QRegExp r(":[0-9]+");
            int m = r.indexIn(spec, 0);
            int len = r.matchedLength();
            if (m >= 0) {
                int displayId = spec.mid(m + 1, len - 1).toInt();
                setDisplay(displayId);
            }

            QRegExp rotRegExp("Rot[0-9]+");
            m = rotRegExp.indexIn(spec, 0);
            len = r.matchedLength();
            if (m >= 0) {
                int rotation = spec.mid(m + 3, len - 3).toInt();
                setRotation(rotation);
            }
        }
    }
    argc = 1;
}

QVFbOptions::~QVFbOptions()
{
}

void QVFbOptions::setWidth(int w)
{
    QSettings s;
    s.setValue("width", w);
}

void QVFbOptions::setHeight(int h)
{
    QSettings s;
    s.setValue("height", h);
}

void QVFbOptions::setDepth(int d)
{
    QSettings s;
    s.setValue("depth", d);
}

void QVFbOptions::setCursor(bool c)
{
    mCursor = c;
}

void QVFbOptions::setProtocol(int p)
{
    mProtocol = p;
}

void QVFbOptions::setZoom(double z)
{
    QSettings s;
    s.setValue("zoom", z);
}

void QVFbOptions::setDisplay(int displayId)
{
    QSettings s;
    s.setValue("display", displayId);
}

void QVFbOptions::setRotation(int r)
{
    QSettings s;
    s.setValue("rotation", r);
}

void QVFbOptions::setSkin(const QString &skin)
{
    QSettings s;
    s.setValue("skin", skin);
}

int QVFbOptions::width() const
{
    QSettings s;
    return s.value("width", 240).toInt();
}
int QVFbOptions::height() const
{
    QSettings s;
    return s.value("height", 320).toInt();
}
int QVFbOptions::depth() const
{
    QSettings s;
    return s.value("depth", 32).toInt();
}
bool QVFbOptions::cursor() const
{
    return mCursor;
}
int QVFbOptions::protocol() const
{
    return mProtocol;
}
QString QVFbOptions::skin() const
{
    QSettings s;
    return s.value("skin").toString();
}
double QVFbOptions::zoom() const 
{
    QSettings s;
    return s.value("zoom", 1.0).toDouble();
}
int QVFbOptions::display() const
{
    QSettings s;
    return s.value("display", 0).toInt();
}
int QVFbOptions::rotation() const
{
    QSettings s;
    return s.value("rotation", 0).toInt();
}

