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
static char *rcsidpageH = "$XConsortium: page.h /main/5 1995/07/14 09:46:34 drk $";
#endif
#endif

#ifndef Page_H
#define Page_H

#define MAXPAGES 100
#define MAXINIT 512

struct Page {
    Page()
	: page( 0 ), majorTab( 0 ), minorTab( 0 ), label( 0 ),
	  minorPB( (Widget) 0 ), majorPB( (Widget) 0 ),
	  lasttoppos( 0 ), lastcursorpos( 0 )
    {
    }

    ~Page()
    {
	if ( page )     XtFree( page );
	if ( majorTab ) XtFree( majorTab );
	if ( minorTab ) XtFree( minorTab );
	if ( label )    XtFree( label );

	if ( minorPB ) XtDestroyWidget( minorPB );
	if ( majorPB ) XtDestroyWidget( majorPB );
    }

    char *page;
    char *majorTab;
    char *minorTab;
    char *label;
    Widget minorPB;
    Widget majorPB;
    int lasttoppos;
    int lastcursorpos;
};

extern Page *pages[];
extern int currentPage;
extern int maxpages;

void SetPage(int);
void AdjustPages(int, int);
void FixPages();

struct Options {
    Options() : todoFile( 0 ) { }
    ~Options()
    {
	if ( todoFile ) XtFree( todoFile );
    }

    String todoFile;
};

extern Options options;

#endif /* Page_H */
