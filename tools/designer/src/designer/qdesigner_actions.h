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

#include <QtCore/QPointer>
#include <QtCore/QObject>

class QDesignerMainWindow;
class QDesignerWorkbench;

class QAction;
class QActionGroup;
class AbstractFormEditor;
class AbstractFormWindow;

class QDesignerActions: public QObject
{
    Q_OBJECT
public:
    QDesignerActions(QDesignerWorkbench *mainWindow);
    virtual ~QDesignerActions();

    QDesignerWorkbench *workbench() const;
    AbstractFormEditor *core() const;

    bool saveForm(AbstractFormWindow *fw);
    bool readInForm(const QString &fileName);
    bool writeOutForm(AbstractFormWindow *formWindow, const QString &fileName);

    QActionGroup *fileActions() const;
    QActionGroup *recentFilesActions() const;
    QActionGroup *editActions() const;
    QActionGroup *formActions() const;
    QActionGroup *windowActions() const;
    QActionGroup *toolActions() const;

//
// file actions
//
    QAction *newFormAction() const;
    QAction *openFormAction() const;
    QAction *saveFormAction() const;
    QAction *saveFormAsAction() const;
    QAction *saveFormAsTemplateAction() const;
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

    QAction *preferences() const;

//
// edit mode actions
//
    QAction *editWidgets() const;
    QAction *editConnections() const;
    QAction *editTabOrders() const;
    QAction *editBuddies() const;

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

//
// window actions
//
    QAction *minimizeAction() const;
    QAction *zoomAction() const;
    QAction *bringAllToFront() const;

public slots:
    void activeFormWindowChanged(AbstractFormWindow *formWindow);
    void createForm();

private slots:
    void openForm();
    void saveForm();
    void saveFormAs();
    void saveFormAsTemplate();
    void previewForm();
    void notImplementedYet();
    void editPreferences();
    void shutdown();
    void editWidgets();
    void openRecentForm();
    void clearRecentFiles();
    void closeForm();
    void handlePreferenceChange();
    void minimizeForm();
    void zoomForm();
    void bringAllToFront();

private:
    bool saveFormAs(AbstractFormWindow *fw);
    void fixActionContext();
    void updateRecentFileActions();
    void addRecentFile(const QString &fileName);

private:
    enum { MaxRecentFiles = 10 };
    QDesignerWorkbench *m_workbench;
    AbstractFormEditor *m_core;

    QActionGroup *m_fileActions;
    QActionGroup *m_recentFilesActions;
    QActionGroup *m_editActions;
    QActionGroup *m_formActions;
    QActionGroup *m_windowActions;
    QActionGroup *m_toolActions;

    QAction *m_editWidgetsAction;

    QAction *m_newFormAction;
    QAction *m_openFormAction;
    QAction *m_saveFormAction;
    QAction *m_saveFormAsAction;
    QAction *m_saveFormAsTemplateAction;
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

    QAction *m_preferences;

    QAction *m_layoutHorizontallyAction;
    QAction *m_layoutVerticallyAction;
    QAction *m_layoutHorizontallyInSplitterAction;
    QAction *m_layoutVerticallyInSplitterAction;
    QAction *m_layoutGridAction;
    QAction *m_breakLayoutAction;
    QAction *m_adjustSizeAction;
    QAction *m_previewFormAction;

    QAction *m_minimizeAction;
    QAction *m_zoomAction;
    QAction *m_bringAllToFrontAction;
};

#endif // QDESIGNER_ACTIONS_H

