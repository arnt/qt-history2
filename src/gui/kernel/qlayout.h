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

#ifndef QLAYOUT_H
#define QLAYOUT_H

#include "QtCore/qobject.h"
#include "QtGui/qlayoutitem.h"
#include "QtGui/qsizepolicy.h"
#include "QtCore/qrect.h"

#include "limits.h"

class QLayout;
class QSize;

#ifdef QT3_SUPPORT
class Q_GUI_EXPORT QLayoutIterator
{
public:
    inline QT3_SUPPORT_CONSTRUCTOR QLayoutIterator(QLayout *i) : layout(i), index(0) {}
    inline QLayoutIterator(const QLayoutIterator &i)
	: layout(i.layout), index(i.index) {}
    inline QLayoutIterator &operator=(const QLayoutIterator &i) {
        layout = i.layout;
        index = i.index;
        return *this;
    }
    inline QT3_SUPPORT QLayoutItem *operator++();
    inline QT3_SUPPORT QLayoutItem *current();
    inline QT3_SUPPORT QLayoutItem *takeCurrent();
    inline QT3_SUPPORT void deleteCurrent();

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

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QLayout(QWidget *parent, int margin, int spacing = -1,
                             const char *name = 0);
    QT3_SUPPORT_CONSTRUCTOR QLayout(QLayout *parentLayout, int spacing, const char *name = 0);
    QT3_SUPPORT_CONSTRUCTOR QLayout(int spacing, const char *name = 0);
    inline QT3_SUPPORT QWidget *mainWidget() const { return parentWidget(); }
    inline QT3_SUPPORT void remove(QWidget *w) { removeWidget(w); }
    inline QT3_SUPPORT void add(QWidget *w) { addWidget(w); }

    QT3_SUPPORT void setAutoAdd(bool a);
    QT3_SUPPORT bool autoAdd() const;
    inline QT3_SUPPORT QLayoutIterator iterator() { return QLayoutIterator(this,true); }

    inline QT3_SUPPORT int defaultBorder() const { return spacing(); }
#endif
};

#ifdef QT3_SUPPORT
inline QLayoutItem *QLayoutIterator::operator++() { return layout->itemAt(++index); }
inline QLayoutItem *QLayoutIterator::current() { return layout->itemAt(index); }
inline QLayoutItem *QLayoutIterator::takeCurrent() { return layout->takeAt(index); }
inline void QLayoutIterator::deleteCurrent() { delete  layout->takeAt(index); }
#endif


#endif

//### support old includes
#if 1 //def QT3_SUPPORT
#include "QtGui/qboxlayout.h"
#include "QtGui/qgridlayout.h"
#endif
