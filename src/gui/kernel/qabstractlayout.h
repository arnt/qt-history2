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

#ifndef QABSTRACTLAYOUT_H
#define QABSTRACTLAYOUT_H

#include "QtCore/qobject.h"
#include "QtGui/qsizepolicy.h"
#include "QtCore/qrect.h"

#include "limits.h"

static const int QLAYOUTSIZE_MAX = INT_MAX/256/16;

class QLayout;
class QLayoutItem;
class QSpacerItem;
class QWidget;
class QSize;

class Q_GUI_EXPORT QLayoutItem
{
public:
    explicit QLayoutItem(Qt::Alignment alignment = 0) : align(alignment) { }
    virtual ~QLayoutItem();
    virtual QSize sizeHint() const = 0;
    virtual QSize minimumSize() const = 0;
    virtual QSize maximumSize() const = 0;
    virtual QSizePolicy::ExpandData expanding() const = 0;
    virtual void setGeometry(const QRect&) = 0;
    virtual QRect geometry() const = 0;
    virtual bool isEmpty() const = 0;
    virtual bool hasHeightForWidth() const;
    virtual int heightForWidth(int) const;
    virtual int minimumHeightForWidth(int) const;
    virtual void invalidate();

    virtual QWidget *widget();
    virtual QLayout *layout();
    virtual QSpacerItem *spacerItem();

    Qt::Alignment alignment() const { return align; }
    void setAlignment(Qt::Alignment a);

protected:
    Qt::Alignment align;
};

class Q_GUI_EXPORT QSpacerItem : public QLayoutItem
{
public:
    QSpacerItem(int w, int h,
                 QSizePolicy::SizeType hData = QSizePolicy::Minimum,
                 QSizePolicy::SizeType vData = QSizePolicy::Minimum)
        : width(w), height(h), sizeP(hData, vData) { }
    void changeSize(int w, int h,
                     QSizePolicy::SizeType hData = QSizePolicy::Minimum,
                     QSizePolicy::SizeType vData = QSizePolicy::Minimum);
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    QSizePolicy::ExpandData expanding() const;
    bool isEmpty() const;
    void setGeometry(const QRect&);
    QRect geometry() const;
    QSpacerItem *spacerItem();

private:
    int width;
    int height;
    QSizePolicy sizeP;
    QRect rect;
};

class Q_GUI_EXPORT QWidgetItem : public QLayoutItem
{
public:
    explicit QWidgetItem(QWidget *w) : wid(w) { }
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    QSizePolicy::ExpandData expanding() const;
    bool isEmpty() const;
    void setGeometry(const QRect&);
    QRect geometry() const;
    virtual QWidget *widget();

    bool hasHeightForWidth() const;
    int heightForWidth(int) const;

private:
    QWidget *wid;
};

#ifdef QT_COMPAT
class Q_GUI_EXPORT QLayoutIterator
{
public:
    inline QT_COMPAT_CONSTRUCTOR QLayoutIterator(QLayout *i) : layout(i), index(0) {}
    inline QLayoutIterator(const QLayoutIterator &i)
	: layout(i.layout), index(i.index) {}
    inline QLayoutIterator &operator=(const QLayoutIterator &i) {
        layout = i.layout;
        index = i.index;
        return *this;
    }
    inline QT_COMPAT QLayoutItem *operator++();
    inline QT_COMPAT QLayoutItem *current();
    inline QT_COMPAT QLayoutItem *takeCurrent();
    inline QT_COMPAT void deleteCurrent();

private:
    // hack to avoid deprecated warning
    friend class QLayout;
    inline QLayoutIterator(QLayout *i, bool) : layout(i), index(0) {}
    QLayout *layout;
    int index;
};
#endif

class QLayoutPrivate;

class Q_GUI_EXPORT QLayout : public QObject, public QLayoutItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QLayout)

    Q_ENUMS(ResizeMode)
    Q_PROPERTY(int margin READ margin WRITE setMargin)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(ResizeMode resizeMode READ resizeMode WRITE setResizeMode)

public:
    enum ResizeMode { Auto, FreeResize, Minimum, Fixed };

    QLayout(QWidget *parent);
    QLayout(QLayout *parentLayout);
    QLayout();
    ~QLayout();

    int margin() const;
    int spacing() const;

    void setMargin(int);
    void setSpacing(int);

    void setResizeMode(ResizeMode);
    ResizeMode resizeMode() const;

    void setMenuBar(QWidget *w);
    QWidget *menuBar() const;

    QWidget *parentWidget() const;
    bool isTopLevel() const;

    void invalidate();
    QRect geometry() const;
    bool activate();
    void update();

    void addWidget(QWidget *w);
    virtual void addItem(QLayoutItem *) = 0;

    void removeWidget(QWidget *w);
    void removeItem(QLayoutItem *);

    QSizePolicy::ExpandData expanding() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    void setGeometry(const QRect&) = 0;
    virtual QLayoutItem *itemAt(int index) const = 0;
    virtual QLayoutItem *takeAt(int index) = 0;
    bool isEmpty() const;

    int totalHeightForWidth(int w) const;
    QSize totalMinimumSize() const;
    QSize totalMaximumSize() const;
    QSize totalSizeHint() const;
    QLayout *layout();

    void setEnabled(bool);
    bool isEnabled() const;

    void freeze(int w=0, int h=0);

    static QSize closestAcceptableSize(const QWidget *w, QSize s);

protected:
    void widgetEvent(QEvent *);
    void childEvent(QChildEvent *e);
    void addChildLayout(QLayout *l);
    void addChildWidget(QWidget *w);
    void deleteAllItems();

    QRect alignmentRect(const QRect&) const;
protected:
    QLayout(QLayoutPrivate &d, QLayout*, QWidget*);

private:
    Q_DISABLE_COPY(QLayout)

    static void activateRecursiveHelper(QLayoutItem *item);

    friend class QApplication;

#ifdef QT_COMPAT
public:
    QT_COMPAT_CONSTRUCTOR QLayout(QWidget *parent, int margin, int spacing = -1,
                             const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QLayout(QLayout *parentLayout, int spacing, const char *name = 0);
    QT_COMPAT_CONSTRUCTOR QLayout(int spacing, const char *name = 0);
    inline QT_COMPAT QWidget *mainWidget() const { return parentWidget(); }
    inline QT_COMPAT void remove(QWidget *w) { removeWidget(w); }
    inline QT_COMPAT void add(QWidget *w) { addWidget(w); }

    QT_COMPAT void setAutoAdd(bool a);
    QT_COMPAT bool autoAdd() const;
    inline QT_COMPAT QLayoutIterator iterator() { return QLayoutIterator(this,true); }

    inline QT_COMPAT int defaultBorder() const { return spacing(); }
#endif
};

#ifdef QT_COMPAT
inline QLayoutItem *QLayoutIterator::operator++() { return layout->itemAt(++index); }
inline QLayoutItem *QLayoutIterator::current() { return layout->itemAt(index); }
inline QLayoutItem *QLayoutIterator::takeCurrent() { return layout->takeAt(index); }
inline void QLayoutIterator::deleteCurrent() { delete  layout->takeAt(index); }
#endif


#endif
