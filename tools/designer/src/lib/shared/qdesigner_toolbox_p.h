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

#ifndef QDESIGNER_TOOLBOX_H
#define QDESIGNER_TOOLBOX_H

#include "shared_global_p.h"
#include "qdesigner_propertysheet_p.h"
#include <QtGui/QPalette>

namespace qdesigner_internal {
    class PromotionTaskMenu;
}

class QToolBox;

class QAction;
class QMenu;

class QDESIGNER_SHARED_EXPORT QToolBoxHelper : public QObject
{
    Q_OBJECT

    QToolBoxHelper(QToolBox *toolbox);
public:
    // Install helper on QToolBox
    static void install(QToolBox *toolbox);
    static QToolBoxHelper *helperOf(const QToolBox *toolbox);
    // Convenience to add a menu on a toolbox
    static QMenu *addToolBoxContextMenuActions(const QToolBox *toolbox, QMenu *popup);

    QPalette::ColorRole currentItemBackgroundRole() const;
    void setCurrentItemBackgroundRole(QPalette::ColorRole role);

    // Add context menu and return page submenu or 0.
    QMenu *addContextMenuActions(QMenu *popup) const;

private slots:
    void removeCurrentPage();
    void addPage();
    void addPageAfter();
    void changeOrder();
    void slotCurrentChanged(int index);

private:
    QToolBox *m_toolbox;
    QAction *m_actionDeletePage;
    QAction *m_actionInsertPage;
    QAction *m_actionInsertPageAfter;
    QAction *m_actionChangePageOrder;
    qdesigner_internal::PromotionTaskMenu* m_pagePromotionTaskMenu;
};

// PropertySheet to handle the page properties
class QDESIGNER_SHARED_EXPORT QToolBoxWidgetPropertySheet : public QDesignerPropertySheet {
public:
    explicit QToolBoxWidgetPropertySheet(QToolBox *object, QObject *parent = 0);

    virtual void setProperty(int index, const QVariant &value);
    virtual QVariant property(int index) const;
    virtual bool reset(int index);

    // Check whether the property is to be saved. Returns false for the page
    // properties (as the property sheet has no concept of 'stored')
    static bool checkProperty(const QString &propertyName);

private:
    enum ToolBoxProperty { PropertyCurrentItemText, PropertyCurrentItemName, PropertyCurrentItemIcon,
                           PropertyCurrentItemToolTip,  PropertyTabSpacing, PropertyToolBoxNone };

    static ToolBoxProperty toolBoxPropertyFromName(const QString &name);
    QToolBox *m_toolBox;
};

typedef QDesignerPropertySheetFactory<QToolBox, QToolBoxWidgetPropertySheet> QToolBoxWidgetPropertySheetFactory;

#endif // QDESIGNER_TOOLBOX_H
