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
static char *rcsid = "$XConsortium: io.c /main/6 1995/07/14 09:46:23 drk $";
#endif
#endif

// Qt includes
#include <qmessagebox.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include <Xm/Notebook.h>
#include <Xm/Text.h>

// Wrap non-standard includes and global variables with extern "C"
extern "C" {
#include <Exm/TabB.h>
#include "page.h"

extern Widget notebook, textw, shell;
Page pages[MAXPAGES];

extern int currentPage;
extern int modified;
int maxpages;
Page AllocPage();

void ReadDB(char*);
void SaveDB(char*);
} // extern "C"

/* Pages are stored pretty simply:
 * each page starts with "*PLabel"
 * the next line has "*TLabel" if there is a major tab
 * or with *MLabel for a minor tab
 * *Cnnnn has the cursor position
 * *Lnnnn has the top line position
 * the page's text continues to the next page start.
 * regular lines start with .
 */

// Convert ParseNewLines to proper C++
/*
void ParseNewLines(char *label)
char * label;
*/
void ParseNewLines(char *label)
{
    /* look for "\n" and change in '\n' and compact */

    char * s ;

    while (*label) {
	if ((*label == '\\') && (*(label+1) == 'n')) {
            *label = '\n' ;
            s = label+1 ;
            while (*s) {
		*s = *(s+1) ;
		s++ ;
	    }
	}
	label ++ ;
    }

}

// Convert PrintWithNewLines to proper C++
/*
static void PrintWithNewLines(output, label)
FILE * output;
char * label;
*/
static void PrintWithNewLines(FILE *output, char *label)
{
    /* look for '\n' and print "\n" */

    while (*label) {
	if (*label == '\n') {
	    fprintf(output,"\\n");
	} else
	    fprintf(output,"%c", *label);
	label ++ ;
    }
    fprintf(output,"\n"); label ++ ;
}

void
ReadDB(char* filename)
{
  FILE *input;
  int i, number, first = 1;
  char *buffer;
  int max, current;
  char line[1024];
  Arg args[5];
  Widget tab;

  input = fopen(filename, "r");

  if (input == NULL && strncmp(filename,"untitled",8) != 0) {
    QString message = "Cannot access (%1) for reading";
    QMessageBox::warning( 0, "IO Error", message.arg(filename) );
  }

  /* Destroy current pages on reread */
  for(i = 0; i < maxpages; i++) {
    XtFree(pages[i] -> page);
    XtFree(pages[i] -> majorTab);
    XtFree(pages[i] -> minorTab);
    XtFree(pages[i] -> label);
    if (pages[i] -> majorPB)
      XtDestroyWidget(pages[i] -> majorPB);
    if (pages[i] -> minorPB)
      XtDestroyWidget(pages[i] -> minorPB);
  }

  number = 0;


  if (input != NULL) {
    max = MAXINIT;
    buffer = (char*) XtMalloc(max);
    buffer[0] = 0; /* Reset page buffer */
    current = 0;
    pages[0] = AllocPage();

    while(fgets(line, 1024, input) != NULL) {
      if (line[0] == '*') /* Special */
	{
	  if (line[1] == 'P') /* New Page */
	    {
	      if (first == 1) {
		first = 0;
	      }
	      else {
		pages[number] -> page = buffer;
		current = 0;
		max = MAXINIT;
		buffer = (char*) XtMalloc(max);
		buffer[0] = 0; /* Reset page buffer */
		number++;
		pages[number] = AllocPage();
	      }
	      if (strlen(line) > 3) {
		line[strlen(line) - 1] = 0; /* Remove newline */
		pages[number] -> label = XtNewString(&line[2]);
	      }
	    }
	  else if (line[1] == 'T') /* Tab */
	    {
	      XmString name;
	      line[strlen(line) - 1] = 0; /* Remove newline */
	      if (strlen(line) > 3) {
		pages[number] -> majorTab = XtNewString(&line[2]);
		i = 0;
		ParseNewLines(pages[number] -> majorTab);
		name = XmStringGenerate(pages[number] -> majorTab, NULL,
					XmCHARSET_TEXT, NULL);
		XtSetArg(args[i], XmNnotebookChildType, XmMAJOR_TAB); i++;
		XtSetArg(args[i], XmNpageNumber, (number + 1)); i++;
		XtSetArg(args[i], ExmNcompoundString, name); i++;
		XtSetArg(args[i], XmNshadowThickness, 1); i++;
		tab = ExmCreateTabButton(notebook, "atab", args, i);
		XtManageChild(tab);
		pages[number] -> majorPB = tab;
		XmStringFree(name);
	      }
	    }
	  else if (line[1] == 'M') /* Minor Tab */
	    {
	      XmString name;
	      line[strlen(line) - 1] = 0; /* Remove newline */
	      if (strlen(line) > 3) {
		pages[number] -> minorTab = XtNewString(&line[2]);
		i = 0;
		ParseNewLines(pages[number] -> minorTab);
		name = XmStringGenerate(pages[number] -> minorTab, NULL,
					XmCHARSET_TEXT, NULL);
		XtSetArg(args[i], XmNnotebookChildType, XmMINOR_TAB); i++;
		XtSetArg(args[i], XmNpageNumber, (number + 1)); i++;
		XtSetArg(args[i], ExmNcompoundString, name); i++;
		XtSetArg(args[i], XmNshadowThickness, 1); i++;
		tab = ExmCreateTabButton(notebook, "atab", args, i);
		pages[number] -> minorPB = tab;
		XtManageChild(tab);
		XmStringFree(name);
	      }
	    }
	  else if (line[1] == 'C') /* Cursor position */
	    {
	      pages[number] -> lastcursorpos = strtol(&line[2], NULL, 0);
	    }
	  else if (line[1] == 'L') /* Top line position */
	    {
	      pages[number] -> lasttoppos = strtol(&line[2], NULL, 0);
	    }
	}
      else /* Regular line.  "Remove" . and append */
	{
	  current += strlen(&line[1]);
	  if ((current - 2) > max) {
	    max = 2 * max;
	    buffer=(char*) XtRealloc(buffer, max);
	  }
	  strcat(buffer, &line[1]);
	}
    }
  }

  /* If we didn't have a file to read,  we need to setup a page */
  if (input == NULL) {
    number = 0;
    pages[0] = AllocPage();
    pages[0] -> page = XtMalloc(2);
    pages[0] -> page[0] = 0;
  } else {
    pages[number] -> page = buffer;
  }

  maxpages = number;

  i = 0;
  XtSetArg(args[i], XmNlastPageNumber, maxpages + 1); i++;
  XtSetArg(args[i], XmNcurrentPageNumber, 1); i++;
  XtSetValues(notebook, args, i);

  if (input) fclose(input);
}

void
SaveDB(char* filename)
{
  int number;
  FILE *output;
  int i;
  char oldfilename[256];

  if (access(filename, F_OK) == 0 &&
      access(filename, W_OK) != 0) {
    QString message = "Cannot access (%1) for writing";
    QMessageBox::warning( 0, "IO Error", message.arg(filename) );
    return;
  }

  /* Append a ~ to make the old filename */
  if (access(filename, F_OK) == 0) {
    strcpy(oldfilename, filename);
    strcat(oldfilename, "~");
    rename(filename, oldfilename);
  }

  /* Make sure to grab current page */
  if (modified) {
    if (pages[currentPage] -> page != NULL)
      XtFree(pages[currentPage] -> page);
    pages[currentPage] -> page = XmTextGetString(textw);
  }

  output = fopen(filename, "w");
  for(number = 0; number <= maxpages; number++) {
    if (pages[number] -> label != NULL)
      fprintf(output, "*P%s\n", pages[number] -> label);
    else
      fprintf(output, "*P\n");
    if (pages[number] -> majorTab != NULL) {
	fprintf(output, "*T");
	PrintWithNewLines(output, pages[number] -> majorTab);
    }
    if (pages[number] -> minorTab != NULL) {
	fprintf(output, "*M");
	PrintWithNewLines(output, pages[number] -> minorTab);
    }
    fprintf(output, "*C%d\n", pages[number] -> lastcursorpos);
    fprintf(output, "*L%d\n", pages[number] -> lasttoppos);
    fputc('.', output);
    if (pages[number] -> page != NULL) {
      for(i = 0; pages[number] -> page[i] != 0; i++) {
	fputc(pages[number] -> page[i], output);
	if (pages[number] -> page[i] == '\n')
	  fputc('.', output);
      }
    }
    fputc('\n', output);
  }
  fclose(output);
}

Page AllocPage()
{
  Page p;

  p = (Page) XtMalloc(sizeof(PageRec));
  p -> page = NULL;
  p -> label = NULL;
  p -> minorTab = NULL;
  p -> majorTab = NULL;
  p -> minorPB = (Widget) 0;
  p -> majorPB = (Widget) 0;
  p -> lasttoppos = 0;
  p -> lastcursorpos = 0;

  return(p);
}
