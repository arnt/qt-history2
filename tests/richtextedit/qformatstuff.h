/****************************************************************************
** $Id: //depot/qt/main/tests/richtextedit/qformatstuff.h#16 $
**
** Definition of the QtTextView class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWHATEVER_H
#define QWHATEVER_H

#include <qmap.h>
#include <qdict.h>
#include <qcolor.h>
#include <qfont.h>
#include <qstring.h>

class QStyleSheetItem;
class QtTextCustomItem;
class QtTextFormatCollection;

class QtTextCharFormat
{
    friend class QtTextFormatCollection;

public:
    QtTextCharFormat();
    QtTextCharFormat( const QtTextCharFormat &format );
    QtTextCharFormat( const QFont &f, const QColor &c );
    QtTextCharFormat &QtTextCharFormat::operator=( const QtTextCharFormat &fmt );
    bool operator==( const QtTextCharFormat &format );
    virtual ~QtTextCharFormat();

    QtTextCharFormat makeTextFormat( const QStyleSheetItem *style, const QMap<QString,QString>& attr,
				     QtTextCustomItem* item = 0) const;

    QColor color() const;
    QFont font() const;
    QString anchorHref() const;
    QString anchorName() const;

    bool isAnchor() const;

    QtTextCharFormat formatWithoutCustom();

    int addRef();
    int removeRef();

    QtTextCustomItem *customItem() const;

private:
    QFont font_;
    QColor color_;
    QString key;
    int ref;
    int logicalFontSize;
    int stdPointSize;
    QString anchor_href;
    QString anchor_name;
    void createKey();
    QtTextFormatCollection* parent;
    QtTextCustomItem* custom;
};


class QtTextFormatCollection
{
    friend class QtTextCharFormat;

public:
    QtTextFormatCollection();

    QtTextCharFormat*  registerFormat( const QtTextCharFormat &format );
    void unregisterFormat( const QtTextCharFormat &format  );

protected:
    QDict<QtTextCharFormat > cKey;
    QtTextCharFormat* lastRegisterFormat;
};


inline QColor QtTextCharFormat::color() const
{
    return color_;
}

inline QFont QtTextCharFormat::font() const
{
    return font_;
}

inline QString QtTextCharFormat::anchorHref() const
{
    return anchor_href;
}

inline QString QtTextCharFormat::anchorName() const
{
    return anchor_name;
}

inline QtTextCustomItem * QtTextCharFormat::customItem() const
{
    return custom;
}

inline bool QtTextCharFormat::isAnchor() const
{
    return !anchor_href.isEmpty()  || !anchor_href.isEmpty();
}



#endif
