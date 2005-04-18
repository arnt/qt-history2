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

#ifndef QTOOLBOX_H
#define QTOOLBOX_H

#include <QtGui/qframe.h>
#include <QtGui/qicon.h>

#ifndef QT_NO_TOOLBOX

class QToolBoxPrivate;

class Q_GUI_EXPORT QToolBox : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(int count READ count)

public:
    explicit QToolBox(QWidget *parent = 0, Qt::WFlags f = 0);
    ~QToolBox();

    int addItem(QWidget *widget, const QString &text);
    int addItem(QWidget *widget, const QIcon &icon, const QString &text);
    int insertItem(int index, QWidget *widget, const QString &text);
    int insertItem(int index, QWidget *widget, const QIcon &icon, const QString &text);

    void removeItem(int index);

    void setItemEnabled(int index, bool enabled);
    bool isItemEnabled(int index) const;

    void setItemText(int index, const QString &text);
    QString itemText(int index) const;

    void setItemIcon(int index, const QIcon &icon);
    QIcon itemIcon(int index) const;

    void setItemToolTip(int index, const QString &toolTip);
    QString itemToolTip(int index) const;

    int currentIndex() const;
    QWidget *currentWidget() const;
    void setCurrentWidget(QWidget *widget);
    QWidget *widget(int index) const;
    int indexOf(QWidget *widget) const;
    int count() const;

public slots:
    void setCurrentIndex(int index);

signals:
    void currentChanged(int index);

protected:
    virtual void itemInserted(int index);
    virtual void itemRemoved(int index);
    void showEvent(QShowEvent *e);
    void changeEvent(QEvent *);

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QToolBox(QWidget *parent, const char *name, Qt::WFlags f = 0);
    inline QT3_SUPPORT void setItemLabel(int index, const QString &text) { setItemText(index, text); }
    inline QT3_SUPPORT QString itemLabel(int index) const { return itemText(index); }
    inline QT3_SUPPORT QWidget *currentItem() const { return widget(currentIndex()); }
    inline QT3_SUPPORT void setCurrentItem(QWidget *item) { setCurrentIndex(indexOf(item)); }
    inline QT3_SUPPORT void setItemIconSet(int index, const QIcon &icon) { setItemIcon(index, icon); }
    inline QT3_SUPPORT QIcon itemIconSet(int index) const { return itemIcon(index); }
    inline QT3_SUPPORT int removeItem(QWidget *item)
    { int i = indexOf(item); removeItem(i); return i; }
    inline QT3_SUPPORT QWidget *item(int index) const { return widget(index); }
#endif

private:
    Q_DECLARE_PRIVATE(QToolBox)
    Q_DISABLE_COPY(QToolBox)
    Q_PRIVATE_SLOT(d_func(), void buttonClicked())
    Q_PRIVATE_SLOT(d_func(), void widgetDestroyed(QObject*))
};


inline int QToolBox::addItem(QWidget *item, const QString &text)
{ return insertItem(-1, item, QIcon(), text); }
inline int QToolBox::addItem(QWidget *item, const QIcon &iconSet,
                              const QString &text)
{ return insertItem(-1, item, iconSet, text); }
inline int QToolBox::insertItem(int index, QWidget *item, const QString &text)
{ return insertItem(index, item, QIcon(), text); }

#endif // QT_NO_TOOLBOX

#endif // QTOOLBOX_H
