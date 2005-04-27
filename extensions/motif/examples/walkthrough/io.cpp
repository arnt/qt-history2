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
static char *rcsid = "$XConsortium: io.c /main/6 1995/07/14 09:46:23 drk $";
#endif
#endif

// Local includes
#include "mainwindow.h"
#include "page.h"

// Qt includes
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qspinbox.h>
#include <qtextcodec.h>
#include <qtextedit.h>
#include <qtextstream.h>

extern int modified;


/* Pages are stored pretty simply:
 * each page starts with "*PLabel"
 * the next line has "*TLabel" if there is a major tab
 * or with *MLabel for a minor tab
 * *Cnnnn has the cursor position
 * *Lnnnn has the top line position
 * the page's text continues to the next page start.
 * regular lines start with .
 */

void MainWindow::readDB( const QString &filename )
{
    int i, number, first = 1;
    char line[1024];

    /* Destroy current pages on reread */
    for(i = 0; i < maxpages; i++) {
	delete pages[i];
	pages[i] = 0;
    }

    QFile file( filename );
    if ( ! file.open( IO_ReadOnly ) &&
	 ! filename.startsWith( QString::fromLatin1( "untitled" ) ) ) {
	QString message = "Cannot access (%1) for reading";
	QMessageBox::warning( this, "IO Error", message.arg(filename) );

	// setup a single page
	pages[0] = new Page();
	return;
    }

    number = 0;
    pages[0] = new Page();

    QString currentText;

    while ( file.readLine( line, 1024 ) > 0 ) {
	if (line[0] == '*') {/* Special */
            if (line[1] == 'P') {/* New Page */
                if (first == 1) {
                    first = 0;
                } else {
                    pages[number]->page = currentText;
                    currentText.clear();
                    number++;
                    pages[number] = new Page();
                }
                if (strlen(line) > 3) {
                    line[strlen(line) - 1] = 0; /* Remove newline */
                    pages[number]->label = QString::fromLocal8Bit( &line[2] );
                }
            } else if (line[1] == 'T') { /* Tab */
                line[strlen(line) - 1] = 0; /* Remove newline */
                if (strlen(line) > 3) {
                    pages[number]->majorTab = QString::fromLocal8Bit( &line[2] );
                    i = 0;
                    pages[number]->majorTab.replace( QString::fromLatin1( "\\n" ),
                                                     QChar( '\n' ) );
                }
            } else if (line[1] == 'M') { /* Minor Tab */
                line[strlen(line) - 1] = 0; /* Remove newline */
                if (strlen(line) > 3) {
                    pages[number]->minorTab = QString::fromLocal8Bit( &line[2] );
                    i = 0;
                    pages[number]->minorTab.replace( QString::fromLatin1( "\\n" ),
                                                     QChar( '\n' ) );
                }
            } else if (line[1] == 'C') { /* Cursor position */
                pages[number]->lastcursorpos = strtol(&line[2], NULL, 0);
            } else if (line[1] == 'L') {/* Top line position */
                pages[number]->lasttoppos = strtol(&line[2], NULL, 0);
            }
        } else { /* Regular line.  "Remove" . and append */
            currentText += QString::fromLocal8Bit( &line[1] );
        }
    }

    pages[number]->page = currentText;

    maxpages = number;
    spinbox->setMaxValue( maxpages + 1 );
    spinbox->setValue( 1 );

    file.close();
}

void MainWindow::saveDB( const QString &filename )
{
    QFileInfo fileinfo( filename );

    if ( fileinfo.exists() && ! fileinfo.isWritable() ) {
	QString message = "Cannot access (%1) for writing";
	QMessageBox::warning( this, "IO Error", message.arg(filename) );
	return;
    }

    /* Append a ~ to make the old filename */
    if ( fileinfo.exists() ) {
	QString oldfilename = filename + '~';
	fileinfo.dir().remove( oldfilename );
	fileinfo.dir().rename( filename, oldfilename );
    }

    /* Make sure to grab current page */
    if (modified)
	pages[currentPage]->page = textedit->text();

    QFile file( filename );
    if ( ! file.open( IO_WriteOnly | IO_Truncate ) ) {
	QString message = "Cannot open (%1) for writing";
	QMessageBox::warning( this, "IO Error", message.arg(filename) );
	return;
    }

    QTextStream stream( &file );

    // maintain backwards compatibility by writing all data in the
    // current encoding
    stream.setCodec( QTextCodec::codecForLocale() );

    for( int number = 0; number <= maxpages; number++ ) {
	if ( ! pages[number]->label.isEmpty() )
	    stream << "*P" << pages[number]->label << endl;
	else
	    stream << "*P" << endl;
	if ( ! pages[number]->majorTab.isEmpty() ) {
	    stream << "*T";
	    QString tmp( pages[number]->majorTab );
	    stream << tmp.replace( '\n', "\\n" ) << endl;
	}
	if ( ! pages[number]->minorTab.isEmpty() ) {
	    stream << "*M";
	    QString tmp( pages[number]->minorTab );
	    stream << tmp.replace( '\n', "\\n" ) << endl;
	}

	stream << "*C" << pages[number]->lastcursorpos << endl;
	stream << "*L" << pages[number]->lasttoppos << endl;

	QString tmp( pages[number]->page );
	stream << '.' << tmp.replace( '\n', "\n." ) << endl;
    }

    file.close();
}
