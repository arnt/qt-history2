/****************************************************************************
**
** Definition of the QtTextView class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
