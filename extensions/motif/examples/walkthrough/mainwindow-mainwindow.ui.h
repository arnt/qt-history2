/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

// Qt includes
#include <qapplication.h>
#include <qmotifwidget.h>

// Motif includes
#include <Xm/Xm.h>

// Demo includes
#include <Xmd/Print.h>

// Existing functions/variables found in todo.cpp and action.cpp
extern "C" {
    void New(Widget, char*, XmPushButtonCallbackStruct *);
    void Open(Widget, XtPointer, XmPushButtonCallbackStruct *);
    void Save(Widget, XtPointer, XmPushButtonCallbackStruct *);
    void Print(Widget, char*, XmdPrintCallbackStruct *);
    void ShowPrintDialog(Widget, XtPointer, XmPushButtonCallbackStruct *);
    void SaveIt(Widget, char*, XmPushButtonCallbackStruct *);
    void NewPage(Widget, XtPointer, XmPushButtonCallbackStruct *);
    void DeletePage(Widget, XtPointer, XmPushButtonCallbackStruct *);
    void EditPage(Widget, XtPointer, XmPushButtonCallbackStruct *);
} // extern "C"


void MainWindow::fileNew()
{
    ::New(NULL, NULL, NULL);
}


void MainWindow::fileOpen()
{
    ::Open(NULL, this, NULL);
}


void MainWindow::fileSave()
{
    ::SaveIt(NULL, NULL, NULL);
}


void MainWindow::fileSaveAs()
{
    ::Save(NULL, this, NULL);
}


void MainWindow::filePrint()
{
    ::ShowPrintDialog(NULL, this, NULL);
}


void MainWindow::fileExit()
{
    qApp->quit();
}


void MainWindow::selProperties()
{
    ::EditPage(NULL, this, NULL);
}


void MainWindow::selNewPage()
{
    ::NewPage(NULL, NULL, NULL);
}


void MainWindow::selDeletePage()
{
    ::DeletePage(NULL, this, NULL);
}
