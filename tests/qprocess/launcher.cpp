#include <qapplication.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qimage.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <qgroupbox.h>

#include <stdlib.h>

#include "launcher.h"
#include "infotext.h"
#include "commands.h"
#include "qprocess.h"

Launcher::Launcher() : QHBox( 0, 0, WStyle_NoBorder | WStyle_Maximize | WStyle_Customize )
{
    int i;
    QPushButton *pb;

    // general design
    setSpacing(10);
    setMargin(10);
    setBackgroundColor( black );

    // set images for later use in the info text
    for ( i=0; images[i].label!=0; i++ ) {
	QMimeSourceFactory::defaultFactory()
	    ->setImage( QString(images[i].label), QString(images[i].file) );
    }

    // layout stuff
    QVBox* vb;
    vb = new QVBox(this);
    setStretchFactor(vb,1);

    // the info text
    info = new QLabel( vb );
#if defined(UNIX)
    info->setFont( QFont("Coolvetica",36) );
#else
    info->setFont( QFont("Coolvetica",20) );
#endif
    info->setBackgroundColor( black );
    info->setAlignment( AlignTop );
    nextInfo();
    QTimer* infotimer = new QTimer( this );
    connect( infotimer, SIGNAL(timeout()), this, SLOT(nextInfo()) );
    infotimer->start( 20000 );

    // layout stuff
    vb = new QVBox( this );
    vb->setBackgroundColor( black );

    // commands as shorthand push buttons
    for ( i=0; command[i].label; i++ ) {
	pb = new QPushButton( command[i].label, vb, QString::number(i) );
	connect( pb, SIGNAL(clicked()), this, SLOT(execute()) );
    }

    // commands in the list view
    QListBox *lb = new QListBox( vb );
    for ( i=0; other_command[i].label; i++ ) {
	lb->insertItem(other_command[i].label);
    }
    //	lb->setFont(QFont("smoothtimes",17));
    lb->setMaximumHeight(pb->height()*8);
    connect(lb, SIGNAL(highlighted(int)), this, SLOT(executeOther(int)) );
    connect(lb, SIGNAL(selected(int)), this, SLOT(executeOther(int)) );

    QHBox* hb = new QHBox(vb);
    hb->setBackgroundColor(white);
    pb = new QPushButton("Quit",hb);
    connect(pb, SIGNAL(clicked()), qApp, SLOT(quit()));
    hb->setSpacing(10);
    hb->setFixedHeight(hb->sizeHint().height());
}

void Launcher::nextInfo()
{
    static int i = 0;

    QString t = infotext[i];
    info->setText( "<blockquote>" +t+ "</blockquote>" );
    i++;
    if ( infotext[i] == 0 ) {
	i = 0;
    }
}

void Launcher::run(const char*path, const char* cmd)
{
    QStringList list = QStringList::split( QChar(' '), cmd );
    QString command = list.first();
    list.remove( list.begin() );

    QDir p( baseDir );
    p.cd( path );
    p.cd( suffixDir );

    QProcess proc( p.absFilePath(command).latin1(), list );

    QDir p2( baseDir );
    p2.cd( path );
    proc.setWorkingDirectory( p2 );

    proc.start();
}

void Launcher::execute()
{
    int i = atoi( sender()->name() );
    run( command[i].path, command[i].file );
}

void Launcher::executeOther(int i)
{
    run( other_command[i].path, other_command[i].file );
}
