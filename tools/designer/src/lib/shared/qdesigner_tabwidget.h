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

#ifndef QDESIGNER_TABWIDGET_H
#define QDESIGNER_TABWIDGET_H

#include "shared_global.h"

#include <QtGui/QTabWidget>

class QDesignerFormWindowInterface;

class QT_SHARED_EXPORT QDesignerTabWidget : public QTabWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentTabName READ currentTabName WRITE setCurrentTabName STORED false DESIGNABLE true)
    Q_PROPERTY(QString currentTabText READ currentTabText WRITE setCurrentTabText STORED false DESIGNABLE true)
    Q_PROPERTY(QString currentTabToolTip READ currentTabToolTip WRITE setCurrentTabToolTip STORED false DESIGNABLE true)
    Q_PROPERTY(QIcon currentTabIcon READ currentTabIcon WRITE setCurrentTabIcon STORED false DESIGNABLE true)

public:
    QDesignerTabWidget(QWidget *parent = 0);
    ~QDesignerTabWidget();

    QString currentTabName() const;
    void setCurrentTabName(const QString &tabName);

    QString currentTabText() const;
    void setCurrentTabText(const QString &tabText);

    QString currentTabToolTip() const;
    void setCurrentTabToolTip(const QString &tabToolTip);

    QIcon currentTabIcon() const;
    void setCurrentTabIcon(const QIcon &tabIcon);

    inline QAction *actionDeletePage() const
    { return m_actionDeletePage; }

    inline QAction *actionInsertPage() const
    { return m_actionInsertPage; }

    bool eventFilter(QObject *o, QEvent *e);

    QDesignerFormWindowInterface *formWindow() const;

private slots:
    void removeCurrentPage();
    void addPage();
    void slotCurrentChanged(int index);

private:
    bool canMove(QMouseEvent *e) const;

private:
    QPoint pressPoint;
    QWidget *dropIndicator;
    int dragIndex;
    QWidget *dragPage;
    QString dragLabel;
    QIcon dragIcon;
    bool mousePressed;
    QAction *m_actionDeletePage;
    QAction *m_actionInsertPage;
};

#endif // QDESIGNER_TABWIDGET_H
