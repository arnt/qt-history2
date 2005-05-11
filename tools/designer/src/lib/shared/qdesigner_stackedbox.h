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
    Q_PROPERTY(QString currentPageName READ currentPageName WRITE setCurrentPageName STORED false DESIGNABLE true)
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

    QString currentPageName() const;
    void setCurrentPageName(const QString &pageName);

public slots:
    void updateButtons();

protected:
    virtual void childEvent(QChildEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void showEvent(QShowEvent *e);
    virtual bool event(QEvent *e);

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
