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

#ifndef QDESIGNER_ACTIONS_H
#define QDESIGNER_ACTIONS_H

#include <QObject>

class QDesignerMainWindow;
class QAction;
class QActionGroup;

class QDesignerActions: public QObject
{
    Q_OBJECT
public:
    QDesignerActions(QDesignerMainWindow *mainWindow);
    virtual ~QDesignerActions();

    QDesignerMainWindow *mainWindow() const;

    QActionGroup *fileActions() const;
    QActionGroup *editActions() const;

//
// file actions
//
    QAction *newFormAction() const;
    QAction *openFormAction() const;
    QAction *saveFormAction() const;
    QAction *saveFormAsAction() const;
    QAction *closeFormAction() const;
    QAction *quitAction() const;

//
// edit actions
//
    QAction *undoAction() const;
    QAction *redoAction() const;
    QAction *cutAction() const;
    QAction *copyAction() const;
    QAction *pasteAction() const;
    QAction *deleteAction() const;
    QAction *selectAllAction() const;
    QAction *sendToBackAction() const;
    QAction *bringToFrontAction() const;

//
// form actions
//
    QAction *layoutHorizontallyAction() const;
    QAction *layoutVerticallyAction() const;
    QAction *layoutHorizontallyInSplitterAction() const;
    QAction *layoutVerticallyInSplitterAction() const;
    QAction *layoutGridAction() const;
    QAction *breakLayoutAction() const;
    QAction *adjustSizeAction() const;
    QAction *previewFormAction() const;

private:
    QDesignerMainWindow *m_mainWindow;

    QActionGroup *m_fileActions;
    QActionGroup *m_editActions;
    QActionGroup *m_formActions;

    QAction *m_newFormAction;
    QAction *m_openFormAction;
    QAction *m_saveFormAction;
    QAction *m_saveFormAsAction;
    QAction *m_closeFormAction;

    QAction *m_quitAction;
    QAction *m_undoAction;
    QAction *m_redoAction;
    QAction *m_cutAction;
    QAction *m_copyAction;
    QAction *m_pasteAction;
    QAction *m_deleteAction;
    QAction *m_sendToBackAction;
    QAction *m_bringToFrontAction;
    QAction *m_selectAllAction;

    QAction *m_layoutHorizontallyAction;
    QAction *m_layoutVerticallyAction;
    QAction *m_layoutHorizontallyInSplitterAction;
    QAction *m_layoutVerticallyInSplitterAction;
    QAction *m_layoutGridAction;
    QAction *m_breakLayoutAction;
    QAction *m_adjustSizeAction;
    QAction *m_previewFormAction;
};

#endif // QDESIGNER_ACTIONS_H

