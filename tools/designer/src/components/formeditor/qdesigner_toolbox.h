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

#ifndef QDESIGNER_TOOLBOX_H
#define QDESIGNER_TOOLBOX_H

#include "formeditor_global.h"

#include <QToolBox>

class QAction;

class QT_FORMEDITOR_EXPORT QDesignerToolBox : public QToolBox
{
    Q_OBJECT
    Q_OVERRIDE(int currentIndex READ currentIndex DESIGNABLE true)
    Q_PROPERTY(QString currentItemText READ currentItemText WRITE setCurrentItemText STORED false DESIGNABLE true)
    Q_PROPERTY(QString currentItemName READ currentItemName WRITE setCurrentItemName STORED false DESIGNABLE true)
    Q_PROPERTY(QIcon currentItemIcon READ currentItemIcon WRITE setCurrentItemIcon STORED false DESIGNABLE true)
    Q_PROPERTY(QString currentItemToolTip READ currentItemToolTip WRITE setCurrentItemToolTip STORED false DESIGNABLE true)
public:
    QDesignerToolBox(QWidget *parent = 0);

    QString currentItemText() const;
    void setCurrentItemText(const QString &itemText);

    QString currentItemName() const;
    void setCurrentItemName(const QString &itemName);

    QIcon currentItemIcon() const;
    void setCurrentItemIcon(const QIcon &itemIcon);

    QString currentItemToolTip() const;
    void setCurrentItemToolTip(const QString &itemToolTip);

    QPalette::ColorRole currentItemBackgroundRole() const;
    void setCurrentItemBackgroundRole(QPalette::ColorRole role);

    inline QAction *actionDeletePage() const
    { return m_actionDeletePage; }

    inline QAction *actionInsertPage() const
    { return m_actionInsertPage; }

private slots:
    void removeCurrentPage();
    void addPage();

protected:
    void itemInserted(int index);

private:
    QAction *m_actionDeletePage;
    QAction *m_actionInsertPage;
};


#endif // QDESIGNER_TOOLBOX_H
