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

// Local includes
#include "mainwindow.h"
#include "page.h"

// Qt includes
#include <qapplication.h>
#include <qmotif.h>

#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>

#define APP_CLASS "XmdTodo"

// Global data
Page *pages[MAXPAGES];
int currentPage = 1;
int maxpages = 0;

Options options;

int modified = 0;


int main( int argc, char **argv )
{
  int i;
  char temppath[256];

  if (argc == 2 && strcmp(argv[1], "-help") == 0) {
    printf("Usage: todo [-todoFile pathname]\n");
    return(0);
  }

  QMotif integrator( APP_CLASS );
  QApplication app( argc, argv );
  MainWindow mainwindow;
  app.setMainWidget( &mainwindow );

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

  mainwindow.readDB(options.todoFile);
  mainwindow.setPage(0);

  mainwindow.show();

  return app.exec();
}
