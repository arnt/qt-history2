/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTITEMDELEGATE_H
#define QABSTRACTITEMDELEGATE_H

#ifndef QT_H
#include <qobject.h>
#include <qpalette.h>
#include <qrect.h>
#endif

class QPainter;
class QAbstractItemView;
class QAbstractItemModel;
class QModelIndex;

class Q_GUI_EXPORT QItemOptions
{
public:
    enum Position { Left, Right, Top, Bottom };
    QItemOptions()
        : palette(), itemRect(), selected(false), open(false),
          focus(false), disabled(false), smallItem(true), editing(false),
          displayAlignment(Qt::AlignAuto|Qt::AlignVCenter),
          decorationAlignment(Qt::AlignCenter),
          decorationPosition(Left) {}

    QPalette palette;
    QRect itemRect;
    uint selected : 1;
    uint open : 1;
    uint focus : 1;
    uint disabled : 1;
    uint smallItem : 1;
    uint editing : 1;
    int displayAlignment;
    int decorationAlignment;
    Position decorationPosition;
};

class QAbstractItemDelegatePrivate;

class Q_GUI_EXPORT QAbstractItemDelegate : public QObject
{
    Q_DECLARE_PRIVATE(QAbstractItemDelegate)

public:
    QAbstractItemDelegate(QAbstractItemModel *model, QObject *parent = 0);
    virtual ~QAbstractItemDelegate();

    QAbstractItemModel *model() const;

    enum EditType {
        NoEditType,
        PersistentWidget,
        WidgetOnTyping,
        WidgetWhenCurrent,
        NoWidget
    };

    enum StartEditAction {
        NeverEdit = 0,
        CurrentChanged = 1,
        DoubleClicked = 2,
        SelectedClicked = 4,
        EditKeyPressed = 8,
        AnyKeyPressed = 16,
        AlwaysEdit = 32
    };

    enum EndEditAction {
        Accepted = 1,
        Cancelled = 2
    };

    // painting
    virtual void paint(QPainter *painter, const QItemOptions &options,
                       const QModelIndex &index) const = 0;
    virtual QSize sizeHint(const QFontMetrics &fontMetrics, const QItemOptions &options,
                           const QModelIndex &index) const = 0;

    // editing
    virtual EditType editType(const QModelIndex &index) const;
    virtual QWidget *editor(StartEditAction action, QWidget *parent,
                            const QItemOptions &options, const QModelIndex &index);
    virtual void setModelData(QWidget *editor, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void updateEditorGeometry(QWidget *editor, const QItemOptions &options,
                                      const QModelIndex &index) const;
    virtual void releaseEditor(EndEditAction action, QWidget *editor, const QModelIndex &index);

    // events for non-widget editors
    virtual bool event(QEvent *e, const QModelIndex &index);

protected:
    QAbstractItemDelegate(QAbstractItemDelegatePrivate &, QAbstractItemModel* model,
                          QObject *parent = 0);
    QString ellipsisText(const QFontMetrics &fontMetrics, int width, int align,
                         const QString &org) const;
};

#endif
