/****************************************************************************
** Form interface generated from reading ui file 'richedit.ui'
**
** Created: Mon Feb 5 13:10:29 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef EDITORFORM_H
#define EDITORFORM_H

#include <qvariant.h>
#include <qapplication.h>
#include <qfiledialog.h>
#include <qfontdatabase.h>
#include <qmainwindow.h>
#include <qmessagebox.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QComboBox;
class QSpinBox;
class QTextEdit;

class EditorForm : public QMainWindow
{ 
    Q_OBJECT

public:
    EditorForm( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~EditorForm();

    QTextEdit* textEdit;
    QComboBox* fontComboBox;
    QSpinBox* SpinBox2;
    QPopupMenu *fileMenu;
    QPopupMenu *editMenu;
    QPopupMenu *PopupMenu_2;
    QPopupMenu *helpMenu;
    QToolBar *toolBar;
    QToolBar *Toolbar;
    QAction* fileNewAction;
    QAction* fileOpenAction;
    QAction* fileSaveAction;
    QAction* fileSaveAsAction;
    QAction* fileExitAction;
    QAction* editUndoAction;
    QAction* editRedoAction;
    QAction* editCutAction;
    QAction* editCopyAction;
    QAction* editPasteAction;
    QAction* helpContentsAction;
    QAction* helpIndexAction;
    QAction* helpAboutAction;
    QAction* boldAction;
    QAction* italicAction;
    QAction* underlineAction;
    QActionGroup* alignActionGroup;
    QAction* leftAlignAction;
    QAction* rightAlignAction;
    QAction* centerAlignAction;


public slots:
    virtual void fileNew();
    virtual void fileOpen();
    virtual void fileSave();
    virtual void fileSaveAs();
    virtual void helpAbout();
    virtual void helpContents();
    virtual void helpIndex();
    virtual void fileExit();
    virtual void changeAlignment( QAction *align );
    virtual void saveAndContinue( int &continueAction,const QString &action );

protected slots:
    virtual void init();
    virtual void destroy();

protected:
    QHBoxLayout* EditorFormLayout;
};

#endif // EDITORFORM_H
