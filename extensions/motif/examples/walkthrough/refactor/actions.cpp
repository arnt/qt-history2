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

// Motif includes
#include <Xm/Xm.h>
#include <Xm/Notebook.h>
#include <Xm/Text.h>

#include "page.h"

extern Widget textw, labelw;

extern int modified;


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
