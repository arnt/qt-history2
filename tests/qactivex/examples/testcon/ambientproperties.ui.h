/****************************************************************************
** $Id: $
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
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
