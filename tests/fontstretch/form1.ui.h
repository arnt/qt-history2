/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qapplication.h>
#include <qfontdatabase.h>


void Form1::changeFamily( const QString & newfamily )
{
    QFont f = viewer->font();
    f.setFamily( newfamily );
    viewer->setFont( f );
}


void Form1::changeSize( int newsize )
{
    QFont f = viewer->font();
    f.setPointSize( newsize / 10 );
    viewer->setFont( f );
}


void Form1::changeStretch( int newstretch )
{
    QFont f = viewer->font();
    f.setStretch( newstretch );
    viewer->setFont( f );
}


void Form1::init()
{
    QFont f = QApplication::font();
    sizebox->setValue( f.pointSize() * 10 );

    QFontDatabase fdb;
    families->insertStringList( fdb.families() );
    families->setCurrentText( QApplication::font().family() );
}

