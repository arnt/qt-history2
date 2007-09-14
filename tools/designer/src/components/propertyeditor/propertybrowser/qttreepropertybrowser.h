/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTTREEPROPERTYBROWSER_H
#define QTTREEPROPERTYBROWSER_H

#include "qtpropertybrowser.h"

QT_BEGIN_NAMESPACE

class QTreeWidgetItem;

class QtTreePropertyBrowser : public QtAbstractPropertyBrowser
{
    Q_OBJECT
    Q_ENUMS(ResizeMode)
    Q_PROPERTY(int indentation READ indentation WRITE setIndentation)
    Q_PROPERTY(bool rootIsDecorated READ rootIsDecorated WRITE setRootIsDecorated)
    Q_PROPERTY(bool headerVisible READ isHeaderVisible WRITE setHeaderVisible)
    Q_PROPERTY(ResizeMode resizeMode READ resizeMode WRITE setResizeMode)
    Q_PROPERTY(int splitterPosition READ splitterPosition WRITE setSplitterPosition)
public:

    enum ResizeMode
    {
        Interactive,
        Stretch,
        Fixed,
        ResizeToContents
    };

    QtTreePropertyBrowser(QWidget *parent = 0);
    ~QtTreePropertyBrowser();

    int indentation() const;
    void setIndentation(int i);

    bool rootIsDecorated() const;
    void setRootIsDecorated(bool show);

    bool isHeaderVisible() const;
    void setHeaderVisible(bool visible);

    ResizeMode resizeMode() const;
    void setResizeMode(ResizeMode mode);

    int splitterPosition() const;
    void setSplitterPosition(int position);

    void setExpanded(QtBrowserItem *item, bool expanded);
    bool isExpanded(QtBrowserItem *item) const;

    void setBackgroundColor(QtBrowserItem *item, const QColor &color);
    QColor backgroundColor(QtBrowserItem *item) const;
    QColor calculatedBackgroundColor(QtBrowserItem *item) const;

    void setMarkPropertiesWithoutValue(bool mark);
    bool markPropertiesWithoutValue() const;

Q_SIGNALS:

    void collapsed(QtBrowserItem *item);
    void expanded(QtBrowserItem *item);

protected:
    virtual void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem);
    virtual void itemRemoved(QtBrowserItem *item);
    virtual void itemChanged(QtBrowserItem *item);

private slots:
    void slotCurrentBrowserItemChanged(QtBrowserItem *);
    void slotCurrentTreeItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);

private:

    class QtTreePropertyBrowserPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtTreePropertyBrowser)
    Q_DISABLE_COPY(QtTreePropertyBrowser)

    Q_PRIVATE_SLOT(d_func(), void slotCollapsed(const QModelIndex &))
    Q_PRIVATE_SLOT(d_func(), void slotExpanded(const QModelIndex &))
};

QT_END_NAMESPACE

#endif
