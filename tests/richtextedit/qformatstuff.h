/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qformatstuff.h#7 $
**
** Definition of the QtTextView class
**
** Created : 990101
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWHATEVER_H
#define QWHATEVER_H

#include <qmap.h>
#include <qcolor.h>
#include <qfont.h>
#include <qstring.h>

class QStyleSheetItem;
class QtTextCustomItem;

class QtTextCharFormat
{
    friend class QtTextFormatCollection;

public:
    QtTextCharFormat();
    QtTextCharFormat( const QtTextCharFormat &format );
    QtTextCharFormat( const QFont &f, const QColor &c, QtTextCustomItem *ci = 0 );
    QtTextCharFormat &QtTextCharFormat::operator=( const QtTextCharFormat &fmt );

    QtTextCharFormat makeTextFormat( const QStyleSheetItem *item );

    QColor color() const;
    QFont font() const;

    bool isCustomItem() { return customItem_ != 0; }

    int addRef();
    int removeRef();

    QtTextCustomItem *customItem() const;

protected:
    QFont font_;
    QColor color_;
    QString key;
    int ref;
    QtTextCustomItem *customItem_;
    int logicalFontSize;
    
};

class QtTextCustomItem
{
public:
    QtTextCustomItem() {}
};

class QtTextFormatCollection
{
    friend class QtTextCharFormat;

public:
    QtTextFormatCollection();

    ushort registerFormat( const QtTextCharFormat &format );
    void unregisterFormat( ushort index );
    QtTextCharFormat format( ushort index );

protected:
    QMap< QString, QtTextCharFormat* > cKey;
    QMap< int, QtTextCharFormat* > cIndex;
    QMap< QString, int > cKeyIndex;

    QtTextCharFormat *lastRegisterFormat;
    ushort lastRegisterIndex;
    ushort lastFormatIndex;
    QtTextCharFormat *lastFormatFormat;

};

#endif
