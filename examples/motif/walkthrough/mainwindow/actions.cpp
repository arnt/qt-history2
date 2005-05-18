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
static char *rcsid = "$TOG: actions.c /main/7 1997/05/02 10:01:40 dbl $";
#endif
#endif

// PageEditDialog includes
#include "pageeditdialog.h"
#include <qlineedit.h>

// Qt includes
#include <qmessagebox.h>

#include <stdlib.h>
#include <ctype.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/SelectioB.h>
#include <Xm/Notebook.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/BulletinB.h>
#include <Xm/RowColumn.h>
#include <Xm/MessageB.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>

// Wrap non-standard includes  and global variables with extern "C"
extern "C" {
#include <Exm/TabB.h>
#include "page.h"

extern Page pages[];
// Widget editDialog = 0, deleteDialog = 0;

void SetPage(int);
void AdjustPages(int, int);
void FixPages();
void PageChange(Widget w, XtPointer i, XmNotebookCallbackStruct *cs);
Page AllocPage();
char* Trim(char*);
extern void SaveDB(char*);

extern Widget shell, textw, labelw, notebook;
extern int maxpages, currentPage, modified;

void NewPage(Widget, XtPointer, XmPushButtonCallbackStruct *);
void DeletePage(Widget, XtPointer, XmPushButtonCallbackStruct *);
void EditPage(Widget, XtPointer, XmPushButtonCallbackStruct *);
void SaveIt(Widget, char *, XmPushButtonCallbackStruct *);
} // extern "C"

void
NewPage(Widget, XtPointer, XmPushButtonCallbackStruct *)
{
  Arg args[2];

  if (modified && pages[currentPage] != NULL) {
    if (pages[currentPage] -> page != NULL)
      XtFree(pages[currentPage] -> page);
     pages[currentPage] -> page = XmTextGetString(textw);
  }
  AdjustPages(currentPage, 1);
  pages[currentPage] = AllocPage();
  FixPages();
  XtSetArg(args[0], XmNcurrentPageNumber, (currentPage + 1));
  XtSetArg(args[1], XmNlastPageNumber, (maxpages + 1));
  XtSetValues(notebook, args, 2);
  SetPage(currentPage);
}

// DeletePage using QMessageBox
void
DeletePage(Widget, XtPointer client_data,
	   XmPushButtonCallbackStruct *)
{
  QWidget *toplevel = (QWidget *) client_data;
  int result =
    QMessageBox::information( toplevel, "Page Delete Dialog",
			      "Do you want to delete this page?",
			      QMessageBox::Yes, QMessageBox::No );
  if ( result != QMessageBox::Yes )
      return;

  Arg args[2];

  if (pages[currentPage] != NULL) {
    if (pages[currentPage] -> page != NULL)
	XtFree(pages[currentPage] -> page);
    if (pages[currentPage] -> minorPB != (Widget) 0)
      XtDestroyWidget(pages[currentPage] -> minorPB);
    if (pages[currentPage] -> majorPB != (Widget) 0)
      XtDestroyWidget(pages[currentPage] -> majorPB);
    // XtFree((XtPointer) pages[currentPage]);
    XtFree((char *) pages[currentPage]);
  }
  pages[currentPage] = NULL;
  AdjustPages(currentPage, -1);

  /* If there are no more pages left,  then create a blank one */
  if (maxpages < 0) {
    pages[0] = AllocPage();
    pages[0] -> page = XtMalloc(2);
    pages[0] -> page[0] = 0;
    maxpages = 0;
  }

  FixPages();
  XtSetArg(args[0], XmNcurrentPageNumber, (currentPage + 1));
  XtSetArg(args[1], XmNlastPageNumber, (maxpages + 1));
  XtSetValues(notebook, args, 2);
  SetPage(currentPage);
}

// EditPage using custom QDialog
void
EditPage(Widget, XtPointer client_data,
	 XmPushButtonCallbackStruct *)
{
  if (pages[currentPage] == NULL) return;

  QWidget *toplevel = (QWidget *) client_data;
  PageEditDialog pedlg( toplevel, "page edit dialog", true );

  if (pages[currentPage] -> label != NULL)
      pedlg.titleEdit->setText( pages[currentPage]->label );
  if (pages[currentPage] -> majorTab != NULL)
      pedlg.majorEdit->setText( pages[currentPage]->majorTab );
  if (pages[currentPage] -> minorTab != NULL)
      pedlg.minorEdit->setText( pages[currentPage]->minorTab );

  int result = pedlg.exec();

  if ( result != QDialog::Accepted )
      return;

  char *temp;
  XmString tstr;
  Arg args[5];
  int i;

  // pages[currentPage] -> label = XmTextFieldGetString(labelEditW);
  QString qstr = pedlg.titleEdit->text().simplifyWhiteSpace();
  pages[currentPage]->label = qstrdup( qstr.local8Bit().data() );

  if (pages[currentPage] -> minorTab != NULL)
    XtFree(pages[currentPage] -> minorTab);
  // temp = XmTextGetString(minorTabW);
  // temp = Trim(temp);
  qstr = pedlg.minorEdit->text().simplifyWhiteSpace();
  temp = qstrdup( qstr.local8Bit().data() );
  if (strlen(temp) > 0)
    pages[currentPage] -> minorTab = temp;
  else {
    XtFree(temp);
    pages[currentPage] -> minorTab = NULL;
    if (pages[currentPage] -> minorPB)
      XtUnmanageChild(pages[currentPage] -> minorPB);
  }

  if (pages[currentPage] -> majorTab != NULL)
    XtFree(pages[currentPage] -> majorTab);
  // temp = XmTextGetString(majorTabW);
  // temp = Trim(temp);
  qstr = pedlg.majorEdit->text().simplifyWhiteSpace();
  temp = qstrdup( qstr.local8Bit().data() );
  if (strlen(temp) > 0)
    pages[currentPage] -> majorTab = temp;
  else {
    XtFree(temp);
    pages[currentPage] -> majorTab = NULL;
    if (pages[currentPage] -> majorPB)
      XtUnmanageChild(pages[currentPage] -> majorPB);
  }

  if (pages[currentPage] -> majorTab != NULL) {
    if (pages[currentPage] -> majorPB == (Widget) 0) {
      i = 0;
      XtSetArg(args[i], XmNpageNumber, currentPage + 1); i++;
      XtSetArg(args[i], XmNnotebookChildType, XmMAJOR_TAB); i++;
      XtSetArg(args[i], XmNshadowThickness, 1); i++;
      pages[currentPage] -> majorPB =
	ExmCreateTabButton(notebook, "atab", args, i);
    }
    tstr = XmStringGenerate(pages[currentPage] -> majorTab, NULL,
			    XmCHARSET_TEXT, NULL);
    XtSetArg(args[0], ExmNcompoundString, tstr);
    XtSetValues(pages[currentPage] -> majorPB, args, 1);
    XtManageChild(pages[currentPage] -> majorPB);
  }

  if (pages[currentPage] -> minorTab != NULL) {
    if (pages[currentPage] -> minorPB == (Widget) 0) {
      i = 0;
      XtSetArg(args[i], XmNpageNumber, currentPage + 1); i++;
      XtSetArg(args[i], XmNnotebookChildType, XmMINOR_TAB); i++;
      XtSetArg(args[i], XmNshadowThickness, 1); i++;
      pages[currentPage] -> minorPB =
	ExmCreateTabButton(notebook, "atab", args, i);
    }
    tstr = XmStringGenerate(pages[currentPage] -> minorTab, NULL,
			    XmCHARSET_TEXT, NULL);
    XtSetArg(args[0], ExmNcompoundString, tstr);
    XtSetValues(pages[currentPage] -> minorPB, args, 1);
    XtManageChild(pages[currentPage] -> minorPB);
  }

  /* Get contents before update */
  XtFree(pages[currentPage] -> page);
  pages[currentPage] -> page = XmTextGetString(textw);

  SetPage(currentPage);
}

void
SaveIt(Widget, char *, XmPushButtonCallbackStruct *)
{
  SaveDB(options.todoFile);
}

void
SetPage(int pageNumber)
{
  XmString tmp;
  char buf[80];
  Arg args[5];

  currentPage = pageNumber;
  if (pageNumber <= maxpages) {
    XtSetArg(args[0], XmNpageNumber, (pageNumber + 1));
    XtSetValues(XtParent(textw), args, 1);
  }
  if (pages[pageNumber] != NULL) {
    if (pages[pageNumber] -> page != NULL)
      XtSetArg(args[0], XmNvalue, pages[pageNumber] -> page);
    else
      XtSetArg(args[0], XmNvalue, "");
    XtSetValues(textw, args, 1);

    if (pages[pageNumber] -> label != NULL)
      tmp = XmStringCreateLocalized(pages[pageNumber] -> label);
    else {
      sprintf(buf, "Page %d", pageNumber + 1);
      tmp = XmStringCreateLocalized(buf);
    }
  }
  else {
    XtSetArg(args[0], XmNvalue, "");
    XtSetValues(textw, args, 1);
    sprintf(buf, "Page %d (Bad Page)", pageNumber + 1);
    tmp = XmStringCreateLocalized(buf);
  }

  XmTextSetTopCharacter(textw, pages[pageNumber] -> lasttoppos);
  XmTextSetInsertionPosition(textw, pages[pageNumber] -> lastcursorpos);

  XtSetArg(args[0], XmNpageNumber, (pageNumber + 1));
  XtSetArg(args[1], XmNlabelString, tmp);
  XtSetValues(labelw, args, 2);
  XmStringFree(tmp);
}

void
PageChange(Widget, XtPointer, XmNotebookCallbackStruct *cs)
{
  if (modified && pages[currentPage] != NULL) {
    if (pages[currentPage] -> page != NULL)
      XtFree(pages[currentPage] -> page);
    pages[currentPage] -> page = XmTextGetString(textw);
    pages[currentPage] -> lasttoppos = XmTextGetTopCharacter(textw);
    pages[currentPage] -> lastcursorpos = XmTextGetInsertionPosition(textw);
  }

  SetPage(cs -> page_number - 1);
}

void
AdjustPages(int startpage, int ins)
{
  int i;

  /* ins is either +1 or -1 for insert or delete a page */

  if (ins > 0) {
    for(i = maxpages; i >= startpage; i--)
      pages[i + 1] = pages[i];
    maxpages += 1;
  } else {
    for(i = startpage; i <= maxpages; i++)
      pages[i] = pages[i + 1];
    maxpages -= 1;
  }
}

void FixPages() {
  int i;
  Arg args[2];

  /* Now scan the pages and fix the tabs */
  for (i = 0; i <= maxpages; i++) {
    XtSetArg(args[0], XmNpageNumber, (i + 1));
    if (pages[i] -> majorPB != (Widget) 0)
      XtSetValues(pages[i] -> majorPB, args, 1);
    if (pages[i] -> minorPB != (Widget) 0)
      XtSetValues(pages[i] -> minorPB, args, 1);
  }
}

/* This function removes leading and trailing whitespace.  It
   deallocates the original string and returns a brand new string */
// Change variable name 'new'
char*
Trim(char* str)
{
  char *newstr;
  int first, last;
  int length;

  if (str == NULL) return(XtNewString(""));

  length = strlen(str);

  for(first = 0; first < length && isspace(str[first]); first++);
  for(last = length - 1; last > first && isspace(str[last]); last--);

  if (! isspace(str[last])) last++; /* Last needs to be adjusted */

  if (first == last) { /* Empty */
    free(str);
    return(strdup(""));
  }

  newstr = XtMalloc(last - first + 1); /* Don't forget last 0 */
  strncpy(newstr, &str[first], last - first);
  newstr[last - first] = 0;
  free(str);
  return(newstr);
}
