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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_TABWIDGET_H
#define QDESIGNER_TABWIDGET_H

#include "shared_global_p.h"

#include <QtGui/QTabWidget>

class QDesignerFormWindowInterface;

class QMenu;

class QDESIGNER_SHARED_EXPORT QDesignerTabWidget : public QTabWidget
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

    void addContextMenuActions(QMenu *popup);

    bool eventFilter(QObject *o, QEvent *e);

    QDesignerFormWindowInterface *formWindow() const;

private slots:
    void removeCurrentPage();
    void addPage();
    void addPageAfter();
    void slotCurrentChanged(int index);

protected:
    bool canMove(QMouseEvent *e) const;
    virtual void tabInserted(int index);
    virtual void tabRemoved(int index);

private:

private:
    int pageFromPosition(const QPoint &pos, QRect &rect) const;

    QPoint m_pressPoint;
    QWidget *m_dropIndicator;
    int m_dragIndex;
    QWidget *m_dragPage;
    QString m_dragLabel;
    QIcon m_dragIcon;
    bool m_mousePressed;
    QAction *m_actionDeletePage;
    QAction *m_actionInsertPage;
    QAction *m_actionInsertPageAfter;
};

#endif // QDESIGNER_TABWIDGET_H
