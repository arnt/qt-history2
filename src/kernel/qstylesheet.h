/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qstylesheet.h#5 $
**
** Definition of the QStyleSheet class
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

#ifndef QSTYLESHEET_H
#define QSTYLESHEET_H

#include "qlist.h"
#include "qdict.h"
#include "qpixmap.h"
#include "qscrollview.h"
#include "qcolor.h"
#include "qml.h" //########## QMLProvider


class QStyleSheet;
class QStyleSheetItemData;


class Q_EXPORT QStyleSheetItem : public Qt
{
public:
    QStyleSheetItem( QStyleSheet* parent, const QString& name );
    ~QStyleSheetItem();

    QString name() const;

    enum AdditionalStyleValues { Undefined  = - 1};

    enum DisplayMode {DisplayBlock, DisplayInline, DisplayListItem, DisplayNone};
    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode m);

    int alignment() const;
    void setAlignment( int f);

    int fontWeight() const;
    void setFontWeight(int w);

    int fontSize() const;
    void setFontSize(int s);

    int fontSizeRelative() const;
    void setFontSizeRelative(int s);


    QString fontFamily() const;
    void setFontFamily( const QString& );

    int numberOfColumns() const;
    void setNumberOfColumns(int ncols);

    QColor color() const;
    void setColor( const QColor &);

    bool fontItalic() const;
    void setFontItalic( bool );
    bool definesFontItalic() const;

    bool fontUnderline() const;
    void setFontUnderline( bool );
    bool definesFontUnderline() const;

    bool isAnchor() const;
    void setAnchor(bool anc);

    enum WhiteSpaceMode {WhiteSpaceNormal, WhiteSpacePre };
    WhiteSpaceMode whiteSpaceMode() const;
    void setWhiteSpaceMode(WhiteSpaceMode m);

    enum Margin { MarginLeft, MarginRight, MarginTop, MarginBottom, MarginAll, MarginVertical, MarginHorizontal };
    int margin( Margin m) const;
    void setMargin( Margin, int);

    enum ListStyle { ListDisc, ListCircle, ListSquare, ListDecimal, ListLowerAlpha, ListUpperAlpha };

    ListStyle listStyle() const;
    void setListStyle( ListStyle );

    QString contexts() const;
    void setContexts( const QString& );
    bool allowedInContext( const QStyleSheetItem* ) const;

    bool selfNesting() const;
    void setSelfNesting( bool );

private:
    void init();
    QStyleSheetItemData* d;
};


#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QDict<QStyleSheetItem>;
// MOC_SKIP_END
#endif

class QTextNode;
class QTextContainer;


class Q_EXPORT QStyleSheet : public QObject
{
    Q_OBJECT
public:
    QStyleSheet( QObject *parent=0, const char *name=0 );
    virtual ~QStyleSheet();

    static QStyleSheet* defaultSheet();
    static void setDefaultSheet( QStyleSheet* );


    QStyleSheetItem* item( const QString& name);
    const QStyleSheetItem* item( const QString& name) const;

    void insert( QStyleSheetItem* item);

    virtual QTextNode* tag( const QString& name,
			  const QDict<QString>&attr,
			  QMLProvider& provider,
			  bool emptyTag = FALSE) const;

    static QString convertFromPlainText( const QString& );
    static bool mightBeRichText( const QString& );

private:
    void init();
    QDict<QStyleSheetItem> styles;
    QStyleSheetItem* nullstyle;
};


#endif // QSTYLESHEET_H
