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

#ifndef QDESIGNER_ACTIONS_H
#define QDESIGNER_ACTIONS_H

#include <QtCore/QObject>
#include <QtCore/QPointer>

class QDesignerWorkbench;

class QDir;
class QTimer;
class QAction;
class QActionGroup;
class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QAssistantClient;
class QRect;
class QWidget;

class QDesignerActions: public QObject
{
    Q_OBJECT
public:
    QDesignerActions(QDesignerWorkbench *mainWindow);
    virtual ~QDesignerActions();

    QDesignerWorkbench *workbench() const;
    QDesignerFormEditorInterface *core() const;

    bool saveForm(QDesignerFormWindowInterface *fw);
    bool readInForm(const QString &fileName);
    bool writeOutForm(QDesignerFormWindowInterface *formWindow, const QString &fileName);

    QActionGroup *fileActions() const;
    QActionGroup *recentFilesActions() const;
    QActionGroup *editActions() const;
    QActionGroup *formActions() const;
    QActionGroup *windowActions() const;
    QActionGroup *toolActions() const;
    QActionGroup *helpActions() const;
    QActionGroup *uiMode() const;
    QActionGroup *styleActions() const;

    // window actions
    QAction *minimizeAction() const;
    // edit mode actions
    QAction *editWidgets() const;
    // form actions
    QAction *previewFormAction() const;

    void setBringAllToFrontVisibility(bool visible);
    
    virtual bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void activeFormWindowChanged(QDesignerFormWindowInterface *formWindow);
    void createForm();
    bool openForm();

signals:
    void useBigIcons(bool);

private slots:
    void saveForm();
    void saveFormAs();
    void saveAllForms();
    void saveFormAsTemplate();
    void previewForm(QAction *action = 0);
    void previewFormLater(QAction *action = 0);
    void notImplementedYet();
    void shutdown();
    void editWidgetsSlot();
    void openRecentForm();
    void clearRecentFiles();
    void closeForm();
    void showDesignerHelp();
    void showWhatsNew();
    void aboutPlugins();
    void aboutDesigner();
    void updateUIMode(QAction *act);
    void showWidgetSpecificHelp();
    void showFormSettings();
    void backupForms();
    void showNewFormDialog(const QString &fileName);

private:
    bool saveFormAs(QDesignerFormWindowInterface *fw);
    void fixActionContext();
    void updateRecentFileActions();
    void addRecentFile(const QString &fileName);
    void showHelp(const QString &help);
    void updateCloseAction();
    bool closePreview();
    QRect fixDialogRect(const QRect &rect) const;
    QString fixResourceFileBackupPath(QDesignerFormWindowInterface *fwi, const QDir& backupDir);
    void showStatusBarMessage(const QString &message) const;
    QActionGroup *createHelpActions();
    bool ensureBackupDirectories();
    
    enum { MaxRecentFiles = 10 };
    QDesignerWorkbench *m_workbench;
    QDesignerFormEditorInterface *m_core;
    QAssistantClient *m_assistantClient;
    QString m_openDirectory;
    QString m_saveDirectory;


    QString m_backupPath;
    QString m_backupTmpPath;

    QTimer* m_backupTimer;
    
    QActionGroup *m_fileActions;
    QActionGroup *m_recentFilesActions;
    QActionGroup *m_editActions;
    QActionGroup *m_formActions;
    QActionGroup *m_windowActions;
    QActionGroup *m_toolActions;
    QActionGroup *m_helpActions;
    QActionGroup *m_uiMode;
    QActionGroup *m_styleActions;

    QAction *m_editWidgetsAction;

    QAction *m_newFormAction;
    QAction *m_openFormAction;
    QAction *m_saveFormAction;
    QAction *m_saveFormAsAction;
    QAction *m_saveAllFormsAction;
    QAction *m_saveFormAsTemplateAction;
    QAction *m_closeFormAction;

    QAction *m_quitAction;

    QAction *m_previewFormAction;
    QAction *m_formSettings;

    QAction *m_minimizeAction;
    QAction *m_bringAllToFrontSeparator;
    QAction *m_bringAllToFrontAction;

    QAction *m_sdiAction;
    QAction *m_dockedMdiAction;
    
    QPointer<QWidget> m_previewWidget;
};

#endif // QDESIGNER_ACTIONS_H

