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

#ifndef QDESIGNER_STACKEDBOX_H
#define QDESIGNER_STACKEDBOX_H

#include "shared_global.h"

#include <QtGui/QStackedWidget>
#include <QtCore/QList>

class QAction;
class QToolButton;

class QT_SHARED_EXPORT QDesignerStackedWidget : public QStackedWidget
{
    Q_OBJECT
public:
    QDesignerStackedWidget(QWidget *parent);

    inline QAction *actionPreviousPage() const
    { return m_actionPreviousPage; }

    inline QAction *actionNextPage() const
    { return m_actionNextPage; }

    inline QAction *actionDeletePage() const
    { return m_actionDeletePage; }

    inline QAction *actionInsertPage() const
    { return m_actionInsertPage; }

public slots:
    void updateButtons();

protected:
    void childEvent(QChildEvent *e);
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);

private slots:
    void prevPage();
    void nextPage();
    void removeCurrentPage();
    void addPage();

private:
    QToolButton *prev, *next;
    QAction *m_actionPreviousPage;
    QAction *m_actionNextPage;
    QAction *m_actionDeletePage;
    QAction *m_actionInsertPage;
};

#endif // QDESIGNER_STACKEDBOX_H
