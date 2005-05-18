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

// MainWindow includes
#include "mainwindow.h"

// Qt includes
#include <qapplication.h>
#include <qmotif.h>
#include <qmotifwidget.h>

#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>

// Motif includes
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/Notebook.h>
#include <Xm/Text.h>

#include "page.h"

#define APP_CLASS "XmdTodo"

void ReadDB(char*);

// Global data
Page *pages[MAXPAGES];
int currentPage = 1;
int maxpages = 0;

Options options;

Widget notebook, textw, labelw;

int modified = 0;


static void TextChanged(Widget, XtPointer, XtPointer)
{ modified = 1; }


static void
PageChange(Widget, XtPointer, XmNotebookCallbackStruct *cs)
{
  if (modified && pages[currentPage] != NULL) {
    delete [] pages[currentPage] -> page;

    char *p = XmTextGetString(textw);
    pages[currentPage] -> page = qstrdup( p );
    XtFree( p );

    pages[currentPage] -> lasttoppos = XmTextGetTopCharacter(textw);
    pages[currentPage] -> lastcursorpos = XmTextGetInsertionPosition(textw);
  }

  SetPage(cs -> page_number - 1);
}


int main( int argc, char **argv )
{
  Arg args[10];
  int n, i;
  char temppath[256];

  if (argc == 2 && strcmp(argv[1], "-help") == 0) {
    printf("Usage: todo [-todoFile pathname]\n");
    return(0);
  }

  QMotif integrator( APP_CLASS );
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

  struct passwd *user = getpwuid(getuid());
  for (i = 0; i < MAXPAGES; i++) {
    pages[i] = NULL;
  }

  if (options.todoFile == NULL) {
    strcpy(temppath, user -> pw_dir);
    strcat(temppath, "/.todo");
    options.todoFile = qstrdup( temppath );
  } else {
    /* Copy the string for consistency */
    options.todoFile = qstrdup( options.todoFile );
  }

  ReadDB(options.todoFile);
  SetPage(0);

  mainwindow.show();
  return app.exec();
}
