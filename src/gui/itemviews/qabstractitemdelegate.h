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
    QItemOptions()
        : palette(), itemRect(), selected(false), open(false),
          focus(false), disabled(false), smallItem(true), editing(false),
          iconAlignment(Qt::AlignAuto|Qt::AlignVCenter),
          textAlignment(Qt::AlignAuto|Qt::AlignVCenter) {}

    QPalette palette;
    QRect itemRect;
    uint selected : 1;
    uint open : 1;
    uint focus : 1;
    uint disabled : 1;
    uint smallItem : 1;
    uint editing : 1;
    int iconAlignment;
    int textAlignment;
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
        NoAction = 1,
        CurrentChanged = 2,
        DoubleClicked = 4,
        SelectedClicked = 8,
        EditKeyPressed = 16,
        AnyKeyPressed = 32
    };

    enum EndEditAction {
        Accepted = 1,
        Cancelled = 2
    };

    // painting
    virtual void paint(QPainter *painter, const QItemOptions &options, const QModelIndex &index) const = 0;
    virtual QSize sizeHint(const QFontMetrics &fontMetrics, const QItemOptions &options,
                           const QModelIndex &index) const = 0;

    // editing
    virtual EditType editType(const QModelIndex &index) const;
    virtual QWidget *createEditor(StartEditAction action, QWidget *parent,
                                  const QItemOptions &options, const QModelIndex &index);
    virtual void setContentFromEditor(QWidget *editor, const QModelIndex &index) const;
    virtual void updateEditorContents(QWidget *editor, const QModelIndex &index) const;
    virtual void updateEditorGeometry(QWidget *editor, const QItemOptions &options, const QModelIndex &index) const;
    virtual void removeEditor(EndEditAction action, QWidget *editor, const QModelIndex &index);

    // events for non-widget editors
    virtual bool event(QEvent *e, const QModelIndex &index);
    
protected:
    QAbstractItemDelegate(QAbstractItemDelegatePrivate &, QAbstractItemModel* model, QObject *parent = 0);
    QString ellipsisText(const QFontMetrics &fontMetrics, int width, int align, const QString &org) const;
};

#endif
