/****************************************************************************
**
** Definition of the QStyleSheet class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSTYLESHEET_H
#define QSTYLESHEET_H

#ifndef QT_H
#include "qstring.h"
#include "qlist.h"
#include "qhash.h"
#include "qobject.h"
#include "qmime.h"
#endif // QT_H

#ifndef QT_NO_RICHTEXT

class QStyleSheet;
class Q3TextDocument;
template<class Key, class T> class QMap;
class QStyleSheetItemData;

class Q_GUI_EXPORT QStyleSheetItem : public Qt
{
public:
    QStyleSheetItem(QStyleSheet* parent, const QString& name);
    QStyleSheetItem(const QStyleSheetItem &);
    ~QStyleSheetItem();

    QStyleSheetItem& operator=(const QStyleSheetItem& other);

    QString name() const;

    QStyleSheet* styleSheet();
    const QStyleSheet* styleSheet() const;

    enum AdditionalStyleValues { Undefined  = - 1};

    enum DisplayMode {
        DisplayBlock,
        DisplayInline,
        DisplayListItem,
        DisplayNone
#ifndef Q_QDOC
        , DisplayModeUndefined = -1
#endif
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
        WhiteSpaceNoWrap
#ifndef Q_QDOC
        , WhiteSpaceModeUndefined = -1
#endif
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
        MarginHorizontal
#ifndef Q_QDOC
        , MarginUndefined = -1
#endif
    };

    int margin(Margin m) const;
    void setMargin(Margin, int);

    enum ListStyle {
        ListDisc,
        ListCircle,
        ListSquare,
        ListDecimal,
        ListLowerAlpha,
        ListUpperAlpha
#ifndef Q_QDOC
        , ListStyleUndefined = -1
#endif
    };

    ListStyle listStyle() const;
    void setListStyle(ListStyle);

    QString contexts() const;
    void setContexts(const QString&);
    bool allowedInContext(const QStyleSheetItem*) const;

    bool selfNesting() const;
    void setSelfNesting(bool);

    void setLineSpacing(int ls);
    int lineSpacing() const;

private:
    void init();
    QStyleSheetItemData* d;
};

#ifndef QT_NO_TEXTCUSTOMITEM
class Q3TextCustomItem;
#endif

class Q_GUI_EXPORT QStyleSheet : public QObject
{
    Q_OBJECT
public:
    QStyleSheet(QObject *parent=0, const char *name=0);
    virtual ~QStyleSheet();

    static QStyleSheet* defaultSheet();
    static void setDefaultSheet(QStyleSheet*);


    QStyleSheetItem* item(const QString& name);
    const QStyleSheetItem* item(const QString& name) const;

    void insert(QStyleSheetItem* item);

#ifndef QT_NO_TEXTCUSTOMITEM
    virtual Q3TextCustomItem* tag(const QString& name,
                                  const QMap<QString, QString> &attr,
                                  const QString& context,
                                  const QMimeSourceFactory& factory,
                                  bool emptyTag, Q3TextDocument *doc) const;
#endif
    static QString escape(const QString&);
    static QString convertFromPlainText(const QString&,
                                         QStyleSheetItem::WhiteSpaceMode mode = QStyleSheetItem::WhiteSpacePre);
    static bool mightBeRichText(const QString&);

    virtual void scaleFont(QFont& font, int logicalSize) const;

    virtual void error(const QString&) const;

private:
    void init();
    QHash<QString, QStyleSheetItem *> styles;
    QStyleSheetItem* nullstyle;
private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QStyleSheet(const QStyleSheet &);
    QStyleSheet &operator=(const QStyleSheet &);
#endif
};

#endif // QT_NO_RICHTEXT

#endif // QSTYLESHEET_H
