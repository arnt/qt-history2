/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "metadatabase.h"
#include "../interfaces/actioninterface.h" // for GCC 2.7.* compatibility
#include "../interfaces/editorinterface.h"
#include "../interfaces/templatewizardiface.h"
#include "../interfaces/eventinterface.h"
#include "../interfaces/languageinterface.h"
#include "../interfaces/filterinterface.h"
#include "../interfaces/programinterface.h"
#include "../interfaces/interpreterinterface.h"
#include "../interfaces/preferenceinterface.h"
#include "../interfaces/projectsettingsiface.h"
#include "sourceeditor.h"

#if defined(HAVE_KDE)
#include <kmainwindow.h>
#else
#include <qmainwindow.h>
#endif

#include <qmap.h>
#include <qguardedptr.h>
#include <qpluginmanager.h>
#include <qobjectlist.h>

class PropertyEditor;
class QWorkspace;
class QMenuBar;
class FormWindow;
class QAction;
class QActionGroup;
class QPopupMenu;
class HierarchyView;
class QCloseEvent;
class Workspace;
class ActionEditor;
class Project;
class OutputWindow;
class QTimer;
class FindDialog;
struct DesignerProject;
class ReplaceDialog;
class GotoLineDialog;
class SourceFile;
class FormFile;

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
#include <qtoolbar.h>
#else
class QToolBar;
#endif
class Preferences;

#if defined(HAVE_KDE)
#define QMainWindow KMainWindow
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

#undef QMainWindow

public:
    MainWindow( bool asClient );
    ~MainWindow();

    HierarchyView *objectHierarchy() const;
    Workspace *workspace() const;
    PropertyEditor *propertyeditor() const;
    ActionEditor *actioneditor() const;

    void resetTool();
    int currentTool() const;

    FormWindow *formWindow();

    bool unregisterClient( FormWindow *w );
    void editorClosed( SourceEditor *e );
    QWidget *isAFormWindowChild( QObject *o ) const;
    QWidget *isAToolBarChild( QObject *o ) const;

    void insertFormWindow( FormWindow *fw );
    QWorkspace *qWorkspace() const;

    void popupFormWindowMenu( const QPoint &gp, FormWindow *fw );
    void popupWidgetMenu( const QPoint &gp, FormWindow *fw, QWidget *w );

    QPopupMenu *setupNormalHierarchyMenu( QWidget *parent );
    QPopupMenu *setupTabWidgetHierarchyMenu( QWidget *parent, const char *addSlot, const char *removeSlot );

    FormWindow *openFormWindow( const QString &fn, bool validFileName = TRUE, FormFile *ff = 0 );
    bool isCustomWidgetUsed( MetaDataBase::CustomWidget *w );

    void setGrid( const QPoint &p );
    void setShowGrid( bool b );
    void setSnapGrid( bool b );
    QPoint grid() const { return grd; }
    bool showGrid() const { return sGrid; }
    bool snapGrid() const { return snGrid && sGrid; }

    QString documentationPath() const;

    static MainWindow *self;
    void saveAllTemp();

    QString templatePath() const { return templPath; }

    void editFunction( const QString &func, const QString &l = QString::null, bool rereadSource = FALSE );

    bool isPreviewing() const { return previewing; }

    Project *currProject() const { return currentProject; }

    FormWindow *activeForm() const { return lastActiveFormWindow; }

    TemplateWizardInterface* templateWizardInterface( const QString& className );
    QUnknownInterface* designerInterface() const { return desInterface; }
    QPtrList<DesignerProject> projectList() const;
    QStringList projectNames() const;
    QStringList projectFileNames() const;
    Project *findProject( const QString &projectName ) const;
    void setCurrentProject( Project *pro );
    void setCurrentProjectByFilename( const QString& proFilename );
    OutputWindow *outputWindow() const { return oWindow; }
    void addPreferencesTab( QWidget *tab, const QString &title, QObject *receiver, const char *init_slot, const char *accept_slot );
    void addProjectTab( QWidget *tab, const QString &title, QObject *receiver, const char *init_slot, const char *accept_slot );
    void setModified( bool b, QWidget *window );
    void slotsChanged();
    void updateFunctionList();
    void updateWorkspace();
    QObjectList *runProject();

    void formNameChanged( FormWindow *fw );

    int currentLayoutDefaultSpacing() const;
    int currentLayoutDefaultMargin() const;

    void saveAllBreakPoints();
    void resetBreakPoints();

    SourceFile *sourceFile();
    void createNewProject( const QString &lang );

    void popupProjectMenu( const QPoint &pos );

public slots:
    void showProperties( QObject *w );
    void updateProperties( QObject *w );
    void showDialogHelp();
    void showDebugStep( QObject *o, int line );
    void showStackFrame( QObject *o, int line );
    void showErrorMessage( QObject *o, int line, const QString &errorMessage );
    void finishedRun();
    void breakPointsChanged();

signals:
    void currentToolChanged();
    void hasActiveForm( bool );
    void hasActiveWindow( bool );
    void hasActiveWindowOrProject( bool );
    void hasNonDummyProject( bool );
    void formModified( bool );
    void formWindowsChanged();
    void formWindowChanged();
    void projectChanged();
    void editorChanged();

protected:
    bool eventFilter( QObject *o, QEvent *e );
    void closeEvent( QCloseEvent *e );

public slots:
    void fileNew();
    void fileClose();
    void fileCloseProject(); // not visible in menu, called from fileClose
    void fileOpen();
    void fileOpen( const QString &filter, const QString &extension, const QString &filename = "" , bool inProject = TRUE );
    bool fileSave();
    bool fileSaveForm(); // not visible in menu, called from fileSave
    bool fileSaveProject(); // not visible in menu, called from fileSaveProject
    bool fileSaveAs();
    void fileSaveAll();
    void fileCreateTemplate();

    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();
    void editDelete();
    void editSelectAll();
    void editLower();
    void editRaise();
    void editAdjustSize();
    void editLayoutHorizontal();
    void editLayoutVertical();
    void editLayoutHorizontalSplit();
    void editLayoutVerticalSplit();
    void editLayoutGrid();
    void editLayoutContainerHorizontal();
    void editLayoutContainerVertical();
    void editLayoutContainerGrid();
    void editBreakLayout();
    void editAccels();
    void editSlots();
    void editConnections();
    SourceEditor *editSource();
    SourceEditor *editSource( SourceFile *f );
    SourceEditor *openSourceEdior();
    void editFormSettings();
    void editProjectSettings();
    void editPixmapCollection();
    void editDatabaseConnections();
    void editPreferences();

    void projectInsertFile();

    void searchFind();
    void searchIncremetalFindMenu();
    void searchIncremetalFind();
    void searchIncremetalFindNext();
    void searchReplace();
    void searchGotoLine();

    void previewForm();
    void previewForm( const QString& );

    void toolsCustomWidget();

    void helpContents();
    void helpManual();
    void helpAbout();
    void helpAboutQt();
    void helpRegister();

private slots:
    void activeWindowChanged( QWidget *w );
    void updateUndoRedo( bool, bool, const QString &, const QString & );

    void toolSelected( QAction* );

    void clipboardChanged();
    void selectionChanged();

    void chooseDocPath();
    void windowsMenuActivated( int id );
    void setupWindowActions();

    void closeAllForms();
    void createNewTemplate();
    void projectSelected( QAction *a );

    void setupRecentlyFilesMenu();
    void setupRecentlyProjectsMenu();
    void recentlyFilesMenuActivated( int id );
    void recentlyProjectsMenuActivated( int id );

    void emitProjectSignals();

private:
    void setupMDI();
    void setupMenuBar();
    void setupEditActions();
    void setupProjectActions();
    void setupSearchActions();
    void setupToolActions();
    void setupLayoutActions();
    void setupFileActions();
    void setupPreviewActions();
    void setupHelpActions();
    void setupRMBMenus();

    void setupPropertyEditor();
    void setupHierarchyView();
    void setupWorkspace();
    void setupActionEditor();
    void setupOutputWindow();

    void setupActionManager();
    void setupPluginManagers();

    void enableAll( bool enable );

    QWidget* previewFormInternal( QStyle* style = 0, QPalette* pal = 0 );

    void writeConfig();
    void readConfig();
    void readOldConfig();

    void setupRMBProperties( QValueList<int> &ids, QMap<QString, int> &props, QWidget *w );
    void handleRMBProperties( int id, QMap<QString, int> &props, QWidget *w );
    void setupRMBSpecialCommands( QValueList<int> &ids, QMap<QString, int> &commands, QWidget *w );
    void handleRMBSpecialCommands( int id, QMap<QString, int> &commands, QWidget *w );
    void setupRMBSpecialCommands( QValueList<int> &ids, QMap<QString, int> &commands, FormWindow *w );
    void handleRMBSpecialCommands( int id, QMap<QString, int> &commands, FormWindow *w );
    bool openEditor( QWidget *w, FormWindow *fw );
    void rebuildCustomWidgetGUI();
    void checkTempFiles();
    void openHelpForDialog( const QString &dia );
    void openProject( const QString &fn );

    void addRecentlyOpened( const QString &fn, QStringList &lst );
    enum LineMode { Error, Step, StackFrame };
    void showSourceLine( QObject *o, int line, LineMode lm );
    QWidget *findRealForm( QWidget *w );

private slots:
    void doSlotsChanged();
    bool openProjectSettings( Project *pro );

private:
    struct Tab
    {
	QWidget *w;
	QString title;
	QObject *receiver;
	const char *init_slot, *accept_slot;
    };

private:
    PropertyEditor *propertyEditor;
    HierarchyView *hierarchyView;
    Workspace *wspace;
    QWidget *lastPressWidget;
    QWorkspace *qworkspace;
#if defined(HAVE_KDE)
    KMenuBar *menubar;
#else
    QMenuBar *menubar;
#endif
    FormWindow *lastActiveFormWindow;
    bool breakLayout, layoutChilds, layoutSelected;
    QPoint grd;
    bool sGrid, snGrid;
    bool restoreConfig;
    bool backPix;
    bool splashScreen;
    QString docPath;
    QString fileFilter;

    QMap<QAction*, Project*> projects;
    QAction *actionEditUndo, *actionEditRedo, *actionEditCut, *actionEditCopy,
    *actionEditPaste, *actionEditDelete,
    *actionEditAdjustSize,
    *actionEditHLayout, *actionEditVLayout, *actionEditGridLayout,
    *actionEditSplitHorizontal, *actionEditSplitVertical,
    *actionEditSelectAll, *actionEditBreakLayout, *actionEditSlots, *actionEditConnections,
    *actionEditLower, *actionEditRaise;
    QActionGroup *actionGroupTools, *actionGroupProjects;
    QAction* actionPointerTool, *actionConnectTool, *actionOrderTool;
    QAction* actionCurrentTool;
    QAction *actionHelpContents, *actionHelpAbout, *actionHelpAboutQt, *actionHelpWhatsThis;
    QAction *actionHelpManual;
#if defined(QT_NON_COMMERCIAL)
    QAction *actionHelpRegister;
#endif
    QAction *actionToolsCustomWidget, *actionEditPreferences;
    QAction *actionWindowTile, *actionWindowCascade, *actionWindowClose, *actionWindowCloseAll;
    QAction *actionWindowNext, *actionWindowPrevious;
    QAction *actionEditFormSettings, *actionEditAccels;
    QAction *actionEditSource;
    QAction *actionSearchFind, *actionSearchIncremetal, *actionSearchReplace, *actionSearchGotoLine;

    QPopupMenu *rmbWidgets;
    QPopupMenu *rmbFormWindow;
    QPopupMenu *customWidgetMenu, *windowMenu, *fileMenu, *recentlyFilesMenu, *recentlyProjectsMenu;
    QToolBar *customWidgetToolBar, *layoutToolBar, *projectToolBar;

    Preferences *prefDia;
    QMap<QString,QString> propertyDocumentation;
    bool client;
    QString templPath;
    ActionEditor *actionEditor;
    Project *currentProject;
    QPluginManager<ActionInterface> *actionPluginManager;
    QPluginManager<EditorInterface> *editorPluginManager;
    QPluginManager<TemplateWizardInterface> *templateWizardPluginManager;
    QPluginManager<ProgramInterface> *programPluginManager;
    QPluginManager<InterpreterInterface> *interpreterPluginManager;
    QPluginManager<PreferenceInterface> *preferencePluginManager;
    QPluginManager<ProjectSettingsInterface> *projectSettingsPluginManager;
    QPtrList<SourceEditor> sourceEditors;
    bool previewing;
    QUnknownInterface *desInterface;
    QStringList recentlyFiles;
    QStringList recentlyProjects;
    OutputWindow *oWindow;
    QValueList<Tab> preferenceTabs;
    QValueList<Tab> projectTabs;
    bool databaseAutoEdit;
    QTimer *updateSlotsTimer;
    QLineEdit *incrementalSearch;
    QGuardedPtr<FindDialog> findDialog;
    QGuardedPtr<ReplaceDialog> replaceDialog;
    QGuardedPtr<GotoLineDialog> gotoLineDialog;
    Project *eProject;
    bool inDebugMode;
    QObjectList debuggingForms;
    QString lastOpenFilter;
    QGuardedPtr<QWidget> previewedForm;
    QPopupMenu *projectMenu;

public:
    QString lastSaveFilter;

};

class SenderObject : public QObject
{
    Q_OBJECT

public:
    SenderObject( QUnknownInterface *i ) : iface( i ) { iface->addRef(); }
    ~SenderObject() { iface->release(); }

public slots:
    void emitInitSignal() { emit initSignal( iface ); }
    void emitAcceptSignal() { emit acceptSignal( iface ); }

signals:
    void initSignal( QUnknownInterface * );
    void acceptSignal( QUnknownInterface * );

private:
    QUnknownInterface *iface;

};

#endif
