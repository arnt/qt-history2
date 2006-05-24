/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QUNDOVIEW_H
#define QUNDOVIEW_H

#include <QtGui/qlistview.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER

class QUndoViewPrivate;
class QUndoStack;
class QUndoGroup;
class QIcon;

QT_MODULE(Gui)

class Q_GUI_EXPORT QUndoView : public QListView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QUndoView)
    Q_PROPERTY(QString emptyLabel READ emptyLabel WRITE setEmptyLabel)
    Q_PROPERTY(QIcon cleanIcon READ cleanIcon WRITE setCleanIcon)

public:
    explicit QUndoView(QWidget *parent = 0);
    explicit QUndoView(QUndoStack *stack, QWidget *parent = 0);
    explicit QUndoView(QUndoGroup *group, QWidget *parent = 0);
    ~QUndoView();

    QUndoStack *stack() const;
    QUndoGroup *group() const;

    void setEmptyLabel(const QString &label);
    QString emptyLabel() const;

    void setCleanIcon(const QIcon &icon);
    QIcon cleanIcon() const;

public Q_SLOTS:
    void setStack(QUndoStack *stack);
    void setGroup(QUndoGroup *group);

private:
    Q_DISABLE_COPY(QUndoView)
};

QT_END_HEADER

#endif // QUNDOVIEW_H
