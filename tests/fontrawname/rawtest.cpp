#include "rawtest.h"

RawTest::RawTest( QWidget *parent, const char *name)
    : QDialog( parent, name )
{
    listB     = new QListBox( this );
    group     = new QButtonGroup( 1, Horizontal, this );
    bold      = new QCheckBox( "Bold", group );
    italic    = new QCheckBox( "Italic", group );
    underline = new QCheckBox( "Underline", group );
    range     = new LCDRange( this );
    sample    = new QLineEdit( this );

#if 0
    QPushButton *fett1 = new QPushButton( "Fett1", this ); 
    QPushButton *fett2 = new QPushButton( "Fett2", this ); 
    QPushButton *fett3 = new QPushButton( "Fett3", this ); 
#endif

#if 0
    group->insert( bold );
    group->insert( italic );
    group->insert( underline );
#endif

    QHBoxLayout *mainL = new QHBoxLayout( this );
    
    mainL->addWidget( listB );
    
    QGridLayout *grid = new QGridLayout( mainL, 2, 2 );
    grid->addWidget( group, 0, 0 );
    grid->addWidget( range, 0, 1 );
    grid->addMultiCellWidget( sample, 1, 1, 0, 1 );

    group->show();

    grid->setRowStretch( 0, 0 );
    grid->setRowStretch( 1, 1 );

    range->setMinimumWidth( 100 );

#ifdef Q_WS_X11
    int xFontCount = 0;
    static char **xFontList = 0;

    xFontList = XListFonts( qt_xdisplay(), "*", 32767, &xFontCount );

    listB->insertStrList( xFontList, xFontCount ); 
#endif

    connect( listB, SIGNAL(selected(const QString&)), 
	            SLOT(fontSelected(const QString&)));

    connect( bold, SIGNAL(toggled(bool)), 
	            SLOT(boldToggled(bool)));
    connect( italic, SIGNAL(toggled(bool)), 
	            SLOT(italicToggled(bool)));
    connect( underline, SIGNAL(toggled(bool)), 
	            SLOT(underlineToggled(bool)));
    connect( range, SIGNAL(valueChanged(int)), 
	            SLOT(sizeChanged(int)));

    sample->setText( "Meget fett!" );
    sample->setFixedHeight( 100 );
}

void RawTest::fontSelected( const QString &s )
{
    font.setRawName( s );
    bold->blockSignals( TRUE );
    italic->blockSignals( TRUE );
    underline->blockSignals( TRUE );
    range->blockSignals( TRUE );
    
    bold->setChecked( font.weight() > QFont::Normal );
    italic->setChecked( font.italic() );
    underline->setChecked( font.underline() );
    range->setValue( font.pointSize() );

    bold->blockSignals( FALSE );
    italic->blockSignals( FALSE );
    underline->blockSignals( FALSE );
    range->blockSignals( FALSE );

    updateSample();
}

void RawTest::boldToggled( bool on )
{
    if ( on )
	font.setWeight( QFont::Bold );
    else
	font.setWeight( QFont::Normal );
    updateSample();
}

void RawTest::italicToggled( bool on )
{
    font.setItalic( on );
    updateSample();
}

void RawTest::underlineToggled( bool on )
{
    font.setUnderline( on );
    updateSample();
}

void RawTest::sizeChanged( int sz )
{
    font.setPointSize( sz );
    updateSample();
}

void RawTest::updateSample()
{
    sample->setFont( font );
}


