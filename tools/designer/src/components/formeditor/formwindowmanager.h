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

#ifndef FORMWINDOWMANAGER_H
#define FORMWINDOWMANAGER_H

#include "formeditor_global.h"
#include "formwindow.h"

#include <abstractformwindowmanager.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QPointer>

class QAction;
class QActionGroup;
class MetaDataBase;
class AbstractFormEditor;

class QT_FORMEDITOR_EXPORT FormWindowManager: public AbstractFormWindowManager
{
    Q_OBJECT
public:
    FormWindowManager(AbstractFormEditor *core, QObject *parent = 0);
    virtual ~FormWindowManager();

    virtual AbstractFormEditor *core() const;

    inline QAction *actionCut() const { return m_actionCut; }
    inline QAction *actionCopy() const { return m_actionCopy; }
    inline QAction *actionPaste() const { return m_actionPaste; }
    inline QAction *actionDelete() const { return m_actionDelete; }
    inline QAction *actionSelectAll() const { return m_actionSelectAll; }
    inline QAction *actionLower() const { return m_actionLower; }
    inline QAction *actionRaise() const { return m_actionRaise; }
    QAction *actionUndo() const;
    QAction *actionRedo() const;

    inline QAction *actionHorizontalLayout() const { return m_actionHorizontalLayout; }
    inline QAction *actionVerticalLayout() const { return m_actionVerticalLayout; }
    inline QAction *actionSplitHorizontal() const { return m_actionSplitHorizontal; }
    inline QAction *actionSplitVertical() const { return m_actionSplitVertical; }
    inline QAction *actionGridLayout() const { return m_actionGridLayout; }
    inline QAction *actionBreakLayout() const { return m_actionBreakLayout; }
    inline QAction *actionAdjustSize() const { return m_actionAdjustSize; }

    inline QAction *actionShowResourceEditor() const { return m_actionShowResourceEditor; }

    AbstractFormWindow *activeFormWindow() const;

    int formWindowCount() const;
    AbstractFormWindow *formWindow(int index) const;

    AbstractFormWindow *createFormWindow(QWidget *parentWidget = 0, Qt::WindowFlags flags = 0);

    bool eventFilter(QObject *o, QEvent *e);

    void dragItems(const QList<AbstractDnDItem*> &item_list, AbstractFormWindow *source_form);

public slots:
    void addFormWindow(AbstractFormWindow *formWindow);
    void removeFormWindow(AbstractFormWindow *formWindow);
    void setActiveFormWindow(AbstractFormWindow *formWindow);

private slots:
    void slotActionCutActivated();
    void slotActionCopyActivated();
    void slotActionPasteActivated();
    void slotActionDeleteActivated();
    void slotActionSelectAllActivated();
    void slotActionLowerActivated();
    void slotActionRaiseActivated();
    void slotActionHorizontalLayoutActivated();
    void slotActionVerticalLayoutActivated();
    void slotActionSplitHorizontalActivated();
    void slotActionSplitVerticalActivated();
    void slotActionGridLayoutActivated();
    void slotActionBreakLayoutActivated();
    void slotActionAdjustSizeActivated();
    void slotActionShowResourceEditorActivated();

    void slotUpdateActions();

private:
    void setupActions();
    FormWindow *findFormWindow(QWidget *w);
    QWidget *findManagedWidget(FormWindow *fw, QWidget *w);

    void layoutContainerHorizontal();
    void layoutContainerVertical();
    void layoutContainerGrid();

    void setCurrentUndoStack(QtUndoStack *stack);

    bool isPassiveInteractor(QWidget *w) const;

private:
    AbstractFormEditor *m_core;
    FormWindow *m_activeFormWindow;
    QList<FormWindow*> m_formWindows;

    bool m_layoutChilds;
    bool m_layoutSelected;
    bool m_breakLayout;

    // edit actions
    QAction *m_actionCut;
    QAction *m_actionCopy;
    QAction *m_actionPaste;
    QAction *m_actionSelectAll;
    QAction *m_actionDelete;
    QAction *m_actionLower;
    QAction *m_actionRaise;
    // layout actions
    QAction *m_actionHorizontalLayout;
    QAction *m_actionVerticalLayout;
    QAction *m_actionSplitHorizontal;
    QAction *m_actionSplitVertical;
    QAction *m_actionGridLayout;
    QAction *m_actionBreakLayout;
    QAction *m_actionAdjustSize;

    QAction *m_actionUndo;
    QAction *m_actionRedo;

    QAction *m_actionShowResourceEditor;
    
    // DnD stuff
    void beginDrag(const QList<AbstractDnDItem*> &item_list, const QPoint &globalPos);
    void endDrag(const QPoint &pos);
    void setItemsPos(const QPoint &pos);
    bool isDecoration(QWidget *widget) const;
    QList<AbstractDnDItem*> m_drag_item_list;
    QWidget *m_last_widget_under_mouse;
    FormWindow *m_last_form_under_mouse;
    FormWindow *m_source_form;

    mutable QPointer<QWidget> lastPassiveInteractor;
    mutable bool lastWasAPassiveInteractor;
};

#endif // FORMWINDOWMANAGER_H
