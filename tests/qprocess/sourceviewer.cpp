#include <qtextstream.h>

#include "sourceviewer.h"

SourceViewer::SourceViewer( const QDir &directory, QWidget *parent, const char *name, WFlags f, bool allowLines ) 
    : QVBox( parent, name, f, allowLines ) 
{
    init( directory );
}

void SourceViewer::init( const QDir &directory )
{
    dir = directory;

    sourceSel = new QComboBox( FALSE, this );
    sourceSel->insertStringList(
	    dir.entryList( "*.cpp" ) );
    sourceSel->insertStringList(
	    dir.entryList( "*.h" ) );

    sourceCode = new QTextEdit( this );
    sourceCode->document()->setSyntaxHighlighter( new QCppSyntaxHighlighter( sourceCode->document() ) );
    if ( !sourceSel->currentText().isEmpty() )
	setSource( sourceSel->currentText() );

    closeButton = new QPushButton( "Close", this );

    connect( sourceSel, SIGNAL(activated( const QString &)), this, SLOT(setSource( const QString &)));
    connect( closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

SourceViewer::~SourceViewer()
{
}

void SourceViewer::close()
{
    if ( QVBox::close() )
	emit closed();
}

void SourceViewer::setSource( const QString & file )
{
    sourceCode->load( dir.absFilePath(file) );
}
