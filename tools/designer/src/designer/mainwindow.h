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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtCore/QHash>
#include <QtCore/QPointer>

#include "preferencedialog.h"

class QActionGroup;
class QMenu;
class QVariant;
class QComboBox;

class AbstractFormEditor;
class AbstractFormWindowManager;
class AbstractFormWindow;
class NewFormDialog;
class PreferenceDialog;
class QAssistantClient;

class MainWindow: public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    virtual ~MainWindow();

    virtual QSize sizeHint() const
    { return QMainWindow::sizeHint() + QSize(100, 0); }

    bool readInForm(const QString &fileName);
    bool writeOutForm(AbstractFormWindow *fw, const QString &saveFile);

public slots:
    void newForm();
    void showTheNewStuff();

protected:
    void closeEvent(QCloseEvent *ev);
    void changeEvent(QEvent *ev);

private slots:
    void windowChanged();
    void selectionChanged();
    void propertyChanged(const QString &name, const QVariant &value);
    void newForm(const QString &widgetClass);
    void saveForm();
    bool saveForm(AbstractFormWindow *fw);
    void saveFormAs();
    bool saveFormAs(AbstractFormWindow *fw);
    void openForm();
    void handleClose(AbstractFormWindow *fw, bool *accept);
    void closeForm();
    void previewForm();
    void readSettings();
    void saveSettings();
    void openRecentForm();
    void clearRecentFiles();
    void showPropertyEditor(bool);
    void showObjectInspector(bool);
    void minimizeForm();
    void maximizeForm();
    void updateWindowMenu();
    void activateFormWindow(QAction *action);
    void showGrid(bool b);
    void readOnly(bool b);
    void onActivated(QWidget *w);
    void showDesignerHelp();
    void aboutDesigner();
    void editMode(QAction *action);
    void editMode(int i);
    void showPreferenceDialog();

private:
    void setupWidgetBox();
    void setupFormEditor();
    void setupMenuBar();
    void setupToolBar();
    void setupFormWindow(AbstractFormWindow *formWindow);
    void enableFormActions(bool enable);
    void addRecentFile(const QString &fileName);
    void updateRecentFileActions();
    void showHelp(const QString &url);

private:
    AbstractFormEditor *core;
    AbstractFormWindowManager *m_formWindowManager;
    QAction *m_actionPreviewForm;
    QAction *m_actionClose;
    QAction *m_actionSave;
    QAction *m_actionSaveAs;
    QAction *m_actionPE;
    QAction *m_actionOI;
    QAction *m_actionMinimize;
    QAction *m_actionMaximize;
    QAction *m_showGrid;
    QAction *m_readOnly;
    QAction *m_widgetEditMode;
    QAction *m_connectionEditMode;
    QAction *m_tabOrderEditMode;
    QAction *m_buddyEditMode;
#ifdef DESIGNER_VIEW3D    
    QAction *m_view3DEditMode;
#endif
    QActionGroup *m_editModeGrp;
    QMenu *m_menuWindow;
    bool m_settingsSaved;
    NewFormDialog *m_newFormDialog;
    QPointer<PreferenceDialog> m_preferenceDialog;

    enum { MaxRecentFiles = 10 };
    QAction *recentFilesActs[MaxRecentFiles];
    QActionGroup *m_actionWindowList;
    QAction *m_actionWindowSeparator;

    QHash<QAction*, AbstractFormWindow*> m_showWindowActions;

    QWidget *invisibleParent;
    QAssistantClient *assistant;
    QComboBox *m_editModeSelector;
    QList<QAction*> m_formActionList;
};

#endif // MAINWINDOW_H
