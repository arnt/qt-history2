/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/mergetr.h#1 $
**
** This is a utility program for merging findtr msgfiles
**
**
** Copyright (C) 1998 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef MERGETR_H
#define MERGETR_H

#include <qstring.h>

void merge( const QString& newname, const QString& oldname,
            const QString& resname );
QString extractContents( const QString& line );
bool hasHandle( const QString& line, const QString& handle);

#endif
