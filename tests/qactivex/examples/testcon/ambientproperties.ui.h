/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/


void AmbientProperties::init()
{
    container = 0;
}

void AmbientProperties::setControl( QWidget *widget )
{
    container = widget;
    backSample->setPaletteBackgroundColor( container->paletteBackgroundColor() );
    foreSample->setPaletteBackgroundColor( container->paletteForegroundColor() );
    fontSample->setFont( container->font() );
    enabledButton->setOn( container->isEnabled() );
    enabledSample->setEnabled( container->isEnabled() );
}

void AmbientProperties::backColor()
{
    QColor col = QColorDialog::getColor( backSample->paletteBackgroundColor(), this );
    backSample->setPaletteBackgroundColor( col );
    container->setPaletteBackgroundColor( col );
}

void AmbientProperties::foreColor()
{
    QColor col = QColorDialog::getColor( foreSample->paletteBackgroundColor(), this );
    foreSample->setPaletteBackgroundColor( col );
    container->setPaletteForegroundColor( col );
}

void AmbientProperties::pickFont()
{
    bool ok;
    QFont f = QFontDialog::getFont( &ok, fontSample->font(), this );
    if ( !ok )
	return;
    fontSample->setFont( f );
    container->setFont( f );
}

void AmbientProperties::toggleEnabled(bool on)
{
    enabledSample->setEnabled( on );
    container->setEnabled( on );
}
