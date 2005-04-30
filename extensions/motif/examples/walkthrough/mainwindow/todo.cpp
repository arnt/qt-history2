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

/*
 *  @OPENGROUP_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  Copyright (c) 1996, 1997, 1998, 1999, 2000 The Open Group
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
 *
 *  This software is subject to an open license. It may only be
 *  used on, with or for operating systems which are themselves open
 *  source systems. You must contact The Open Group for a license
 *  allowing distribution and sublicensing of this software on, with,
 *  or for operating systems which are not Open Source programs.
 *
 *  See http://www.opengroup.org/openmotif/license for full
 *  details of the license agreement. Any use, reproduction, or
 *  distribution of the program constitutes recipient's acceptance of
 *  this agreement.
 *
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, THE PROGRAM IS
 *  PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 *  WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY
 *  OR FITNESS FOR A PARTICULAR PURPOSE
 *
 *  EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT
 *  NOR ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM OR THE
 *  EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGES.
 */
/*
 * HISTORY
 */

#ifdef REV_INFO
#ifndef lint
static char *rcsid = "$XConsortium: todo.c /main/6 1995/07/14 09:46:43 drk $";
#endif
#endif

// Qt based Main Window
#include "mainwindow.h"

// Qt includes
#include <qapplication.h>
#include <qfiledialog.h>
#include <qmotif.h>
#include <qmotifwidget.h>
#include <qmotifdialog.h>

#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
/* X include files */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
/* Motif include files */
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/Notebook.h>
#include <Xm/Text.h>

/* Demo include files */
extern "C" {
#include <Xmd/Print.h>

#include "page.h"
} // extern "C"


#define APP_CLASS "XmdTodo"

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) > (y) ? (y) : (x))

char * fallback_resources[] = {
"*text.rows: 24",
"*text.columns: 80",
"*print_manager.printerList: lp,./todo.txt",
"*notebook.frameShadowThickness: 2",
"*notebook.bindingType:	XmSPIRAL",
NULL
};

/* Options */
OptionsRec options;

#define Offset(field) XtOffsetOf(OptionsRec, field)

XtResource resources[] = {
  {"todoFile", "TodoFile", XtRString, sizeof(String),
    Offset(todoFile), XtRImmediate, NULL}
};

#undef Offset

XrmOptionDescRec optionDesc[] = {
  {"-todoFile", "*todoFile", XrmoptionSepArg, NULL}
};

static void TextChanged(Widget, XtPointer, XtPointer);

// Wrap extern/callback functions and global variables with extern "C"
extern "C" {

    void New(Widget, char*, XmPushButtonCallbackStruct *);
    void Open(Widget, XtPointer, XmPushButtonCallbackStruct *);
    void Save(Widget, XtPointer, XmPushButtonCallbackStruct *);
    void Print(Widget, char*, XmdPrintCallbackStruct *);
    void ShowPrintDialog(Widget, XtPointer, XmPushButtonCallbackStruct *);
    void SaveIt(Widget, char*, XmPushButtonCallbackStruct *);
    void PageChange(Widget, XtPointer, XmNotebookCallbackStruct *);
    void NewPage(Widget, XtPointer, XmPushButtonCallbackStruct *);
    void DeletePage(Widget, XtPointer, XmPushButtonCallbackStruct *);
    void EditPage(Widget, XtPointer, XmPushButtonCallbackStruct *);
    void SetPage(int);

    extern void ReadDB(char*);
    extern void SaveDB(char*);

    extern Page pages[];

    Widget notebook, textw, labelw;
    int currentPage = 1;
    int modified;
    extern int maxpages;
    struct passwd *user;

} // extern "C"


int main( int argc, char **argv )
{
  Arg args[10];
  int n, i;
  char temppath[256];

  if (argc == 2 && strcmp(argv[1], "-help") == 0) {
    printf("Usage: todo [-todoFile pathname]\n");
    return(0);
  }

  QMotif integrator( APP_CLASS, NULL,
		     optionDesc, XtNumber(optionDesc) );
  QApplication app( argc, argv );
  MainWindow mainwindow;
  app.setMainWidget( &mainwindow );

  n = 0;
  XtSetArg(args[n], XmNcurrentPageNumber, 1); n++;
  XtSetArg(args[n], XmNlastPageNumber, 100); n++;

  QMotifWidget *center =
      new QMotifWidget( &mainwindow, xmNotebookWidgetClass,
			args, n, "notebook" );
  mainwindow.setCentralWidget( center );
  notebook = center->motifWidget();

  XtAddCallback(notebook, XmNpageChangedCallback,
		(XtCallbackProc) PageChange, NULL);

  n = 0;
  XtSetArg(args[n], XmNpageNumber, 1); n++;
  XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
  textw = XmCreateScrolledText(notebook, "text", args, n);
  XtManageChild(textw);
  XtAddCallback(textw, XmNvalueChangedCallback,
		(XtCallbackProc) TextChanged, NULL);

  n = 0;
  XtSetArg(args[n], XmNnotebookChildType, XmSTATUS_AREA); n++;
  XtSetArg(args[n], XmNpageNumber, 1); n++;
  labelw = XmCreateLabel(notebook, "label", args, n);
  XtManageChild(labelw);

  user = getpwuid(getuid());
  for (i = 0; i < MAXPAGES; i++) {
    pages[i] = NULL;
  }

  if (options.todoFile == NULL) {
    strcpy(temppath, user -> pw_dir);
    strcat(temppath, "/.todo");
    options.todoFile = XtNewString(temppath);
  } else {
    /* Copy the string for consistency */
    options.todoFile = XtNewString(options.todoFile);
  }

  ReadDB(options.todoFile);
  SetPage(0);

  mainwindow.show();
  return app.exec();
}

static void
TextChanged(Widget, XtPointer, XtPointer)
{
  modified = 1;
}

void
New( Widget, char *, XmPushButtonCallbackStruct * )
{
  char buf[128];
  char *str;
  Boolean found = False;
  int i = 0;

  while(! found) {
    sprintf(buf, "untitled%d.todo", i++);
    found = access(buf, F_OK) != 0;
  }

  str = XtNewString(buf);
  ReadDB(str);
  XtFree(options.todoFile);
  options.todoFile = str;
  // XtVaSetValues(shell, XmNtitle, str, XmNtitleEncoding,
  //               XA_STRING, NULL, NULL);
  SetPage(0);
}

// Save using QFileDialog
void
Save(Widget, XtPointer client_data,
     XmPushButtonCallbackStruct *)
{
  QWidget *toplevel = (QWidget *) client_data;
  QString filename =
      QFileDialog::getSaveFileName( QString(), QString(), toplevel );

  if ( ! filename.isEmpty() ) {
    char *str = qstrdup( filename.local8Bit() );
    SaveDB(str);
    XtFree(options.todoFile);
    options.todoFile = str;
    // XtVaSetValues(shell, XmNtitle, str, XmNtitleEncoding,
    //               XA_STRING, NULL, NULL);
  }
}

// Open using QFileDialog
void
Open(Widget, XtPointer client_data,
     XmPushButtonCallbackStruct *)
{
  QWidget *toplevel = (QWidget *) client_data;
  QString filename =
      QFileDialog::getOpenFileName( QString(), QString(), toplevel );

  if ( ! filename.isEmpty() ) {
    char *str = qstrdup( filename.local8Bit() );
    ReadDB(str);
    XtFree(options.todoFile);
    options.todoFile = str;
    // XtVaSetValues(shell, XmNtitle, str, XmNtitleEncoding,
    //               XA_STRING, NULL, NULL);
  }
}

// Print using QMotifDialog
void ShowPrintDialog(Widget, XtPointer client_data,
		     XmPushButtonCallbackStruct *)
{
    QMotifDialog dialog( (QWidget *) client_data );
    (void) XtCreateWidget( "print dialog", xmdPrintWidgetClass,
			   dialog.shell(), NULL, 0 );

    // the print callback calls QMotifDialog::acceptCallback()
    XtAddCallback( dialog.dialog(), XmdNprintCallback,
		   (XtCallbackProc) QMotifDialog::acceptCallback, &dialog );
    // the cancel callback calls QMotifDialog::rejectCallback()
    XtAddCallback( dialog.dialog(), XmNcancelCallback,
		   (XtCallbackProc) QMotifDialog::rejectCallback, &dialog );

    // the print callback also calls the original Print() function
    XtAddCallback( dialog.dialog(), XmdNprintCallback,
		   (XtCallbackProc) Print, NULL );

    dialog.exec();
}

void
Print(Widget, char *, XmdPrintCallbackStruct *cb)
{
  int i;
  FILE *temp;
  int from, to;

  temp = fopen("/tmp/.todoout", "w");

  if (cb -> first == cb -> last &&
      cb -> first == 0) {
    from = 0;
    to = maxpages - 1;
  } else {
    from = MAX(0, cb -> first - 1);
    to = MIN(maxpages, cb -> last - 1);
  }

  for (i = from; i <= to; i++) {
    if (pages[i] -> label != NULL) {
      fprintf(temp, "Subject: %s\n", pages[i] -> label);
      fprintf(temp, "---------------------------\n\n\n");
    }
    fprintf(temp, "%s", pages[i] -> page);
    if (i != (maxpages - 1)) fprintf(temp, "\f");
  }

  fclose(temp);
  XmdPrintDocument("/tmp/.todoout", cb);
}
