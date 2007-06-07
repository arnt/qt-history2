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

#ifndef PROMOTIONTASKMENU_H
#define PROMOTIONTASKMENU_H

#include "shared_global_p.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QList>

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;

class QAction;
class QMenu;
class QWidget;
class QSignalMapper;

namespace qdesigner_internal {

// A helper class for creating promotion context menus and handling promotion actions.

class QDESIGNER_SHARED_EXPORT PromotionTaskMenu: public QObject
{
    Q_OBJECT
public:
    enum Mode { ModeSingleWidget, ModeMultiSelection };

    explicit PromotionTaskMenu(QWidget *widget,Mode mode = ModeMultiSelection, QObject *parent = 0);

    void setWidget(QWidget *widget);

    // Set menu labels
    void setPromoteLabel(const QString &promoteLabel);
    void setEditPromoteToLabel(const QString &promoteEditLabel);
    // Defaults to "Demote to %1".arg(class).
    void setDemoteLabel(const QString &demoteLabel);

    typedef QList<QAction*> ActionList;

    enum AddFlags { LeadingSeparator = 1, TrailingSeparator = 2, SuppressGlobalEdit = 4};

    // Adds a list of promotion actions according to the current promotion state of the widget.
    void addActions(QDesignerFormWindowInterface *fw, unsigned flags, ActionList &actionList);
    // Convenience that finds the form window.
    void addActions(unsigned flags, ActionList &actionList);

    void addActions(QDesignerFormWindowInterface *fw, unsigned flags, QMenu *menu);
    void addActions(unsigned flags, QMenu *menu);

    // Pop up the editor in a global context.
    static void editPromotedWidgets(QDesignerFormEditorInterface *core, QWidget* parent);

private slots:
    void slotPromoteToCustomWidget(const QString &customClassName);
    void slotDemoteFromCustomWidget();
    void slotEditPromotedWidgets();
    void slotEditPromoteTo();
    void slotEditSignalsSlots();

private:
    void promoteTo(QDesignerFormWindowInterface *fw, const QString &customClassName);

    enum PromotionState { NotApplicable, NoHomogenousSelection, CanPromote, CanDemote };
    PromotionState createPromotionActions(QDesignerFormWindowInterface *formWindow);
    QDesignerFormWindowInterface *formWindow() const;

    typedef QList<QPointer<QWidget> > PromotionSelectionList;
    PromotionSelectionList promotionSelectionList(QDesignerFormWindowInterface *formWindow) const;

    const Mode m_mode;

    QPointer<QWidget> m_widget;

    QSignalMapper *m_promotionMapper;
    // Per-Widget actions
    QList<QAction *> m_promotionActions;

    QAction *m_globalEditAction;
    QAction *m_EditPromoteToAction;
    QAction *m_EditSignalsSlotsAction;

    QString m_promoteLabel;
    QString m_demoteLabel;
};

} // namespace qdesigner_internal

#endif // PROMOTIONTASKMENU_H
