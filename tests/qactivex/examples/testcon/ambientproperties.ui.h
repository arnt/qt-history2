/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/


void AmbientProperties::init()
{
    activex = 0;
}

void AmbientProperties::setControl( QActiveX *ax )
{
    activex = ax;
    backSample->setPaletteBackgroundColor( activex->paletteBackgroundColor() );
    foreSample->setPaletteBackgroundColor( activex->paletteForegroundColor() );
    fontSample->setFont( activex->font() );
    enabledButton->setOn( activex->isEnabled() );
    enabledSample->setEnabled( activex->isEnabled() );
}

void AmbientProperties::backColor()
{
    QColor col = QColorDialog::getColor( backSample->paletteBackgroundColor(), this );
    backSample->setPaletteBackgroundColor( col );
    activex->setPaletteBackgroundColor( col );
}

void AmbientProperties::foreColor()
{
    QColor col = QColorDialog::getColor( foreSample->paletteBackgroundColor(), this );
    foreSample->setPaletteBackgroundColor( col );
    activex->setPaletteForegroundColor( col );
}

void AmbientProperties::pickFont()
{
    bool ok;
    QFont f = QFontDialog::getFont( &ok, fontSample->font(), this );
    if ( !ok )
	return;
    fontSample->setFont( f );
    activex->setFont( f );
}

void AmbientProperties::toggleEnabled(bool on)
{
    enabledSample->setEnabled( on );
    activex->setEnabled( on );
}
