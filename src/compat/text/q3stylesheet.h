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

#ifndef Q3STYLESHEET_H
#define Q3STYLESHEET_H

#include "qstring.h"
#include "qlist.h"
#include "qhash.h"
#include "qobject.h"
#include "qcolor.h"
#include "qfont.h"
//#include "qmime.h"
//#include "qmimefactory.h"

#ifndef QT_NO_RICHTEXT

class Q3StyleSheet;
class Q3TextDocument;
template<class Key, class T> class QMap;
class Q3StyleSheetItemData;

class Q_COMPAT_EXPORT Q3StyleSheetItem
{
public:
    Q3StyleSheetItem(Q3StyleSheet* parent, const QString& name);
    Q3StyleSheetItem(const Q3StyleSheetItem &);
    ~Q3StyleSheetItem();

    Q3StyleSheetItem& operator=(const Q3StyleSheetItem& other);

    QString name() const;

    Q3StyleSheet* styleSheet();
    const Q3StyleSheet* styleSheet() const;

    enum AdditionalStyleValues { Undefined = -1 };

    enum DisplayMode {
        DisplayBlock,
        DisplayInline,
        DisplayListItem,
        DisplayNone,
	DisplayModeUndefined = -1
    };

    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode m);

    int alignment() const;
    void setAlignment(int f);

    enum VerticalAlignment {
        VAlignBaseline,
        VAlignSub,
        VAlignSuper
    };

    VerticalAlignment verticalAlignment() const;
    void setVerticalAlignment(VerticalAlignment valign);

    int fontWeight() const;
    void setFontWeight(int w);

    int logicalFontSize() const;
    void setLogicalFontSize(int s);

    int logicalFontSizeStep() const;
    void setLogicalFontSizeStep(int s);

    int fontSize() const;
    void setFontSize(int s);

    QString fontFamily() const;
    void setFontFamily(const QString&);

    int numberOfColumns() const;
    void setNumberOfColumns(int ncols);

    QColor color() const;
    void setColor(const QColor &);

    bool fontItalic() const;
    void setFontItalic(bool);
    bool definesFontItalic() const;

    bool fontUnderline() const;
    void setFontUnderline(bool);
    bool definesFontUnderline() const;

    bool fontStrikeOut() const;
    void setFontStrikeOut(bool);
    bool definesFontStrikeOut() const;

    bool isAnchor() const;
    void setAnchor(bool anc);

    enum WhiteSpaceMode {
        WhiteSpaceNormal,
        WhiteSpacePre,
        WhiteSpaceNoWrap,
        WhiteSpaceModeUndefined = -1
    };
    WhiteSpaceMode whiteSpaceMode() const;
    void setWhiteSpaceMode(WhiteSpaceMode m);

    enum Margin {
        MarginLeft,
        MarginRight,
        MarginTop,
        MarginBottom,
        MarginFirstLine,
        MarginAll,
        MarginVertical,
        MarginHorizontal,
	MarginUndefined = -1
    };

    int margin(Margin m) const;
    void setMargin(Margin, int);

    enum ListStyle {
        ListDisc,
        ListCircle,
        ListSquare,
        ListDecimal,
        ListLowerAlpha,
        ListUpperAlpha,
	ListStyleUndefined = -1
    };

    ListStyle listStyle() const;
    void setListStyle(ListStyle);

    QString contexts() const;
    void setContexts(const QString&);
    bool allowedInContext(const Q3StyleSheetItem*) const;

    bool selfNesting() const;
    void setSelfNesting(bool);

    void setLineSpacing(int ls);
    int lineSpacing() const;

private:
    void init();
    Q3StyleSheetItemData* d;
};

#ifndef QT_NO_TEXTCUSTOMITEM
class Q3TextCustomItem;
#endif

class Q_COMPAT_EXPORT Q3StyleSheet : public QObject
{
    Q_OBJECT
public:
    Q3StyleSheet(QObject *parent=0, const char *name=0);
    virtual ~Q3StyleSheet();

    static Q3StyleSheet* defaultSheet();
    static void setDefaultSheet(Q3StyleSheet*);


    Q3StyleSheetItem* item(const QString& name);
    const Q3StyleSheetItem* item(const QString& name) const;

    void insert(Q3StyleSheetItem* item);

    static QString escape(const QString&);
    static QString convertFromPlainText(const QString&,
                                        Q3StyleSheetItem::WhiteSpaceMode mode = Q3StyleSheetItem::WhiteSpacePre);
    static bool mightBeRichText(const QString&);

    virtual void scaleFont(QFont& font, int logicalSize) const;

    virtual void error(const QString&) const;

private:
    Q_DISABLE_COPY(Q3StyleSheet)

    void init();
    QHash<QString, Q3StyleSheetItem *> styles;
    Q3StyleSheetItem* nullstyle;
};

#endif // QT_NO_RICHTEXT

#endif // QSTYLESHEET_H
