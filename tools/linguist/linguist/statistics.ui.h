/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Linguist.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

void Statistics::init()
{
    setFixedHeight( sizeHint().height() );
}

void Statistics::updateStats( int w1, int c1, int cs1, int w2, int c2, int cs2 )
{
    untrWords->setText( QString::number( w1 ) );
    untrChars->setText( QString::number( c1 ) );
    untrCharsSpc->setText( QString::number( cs1 ) );
    trWords->setText( QString::number( w2 ) );
    trChars->setText( QString::number( c2 ) );
    trCharsSpc->setText( QString::number( cs2 ) );
}

void Statistics::closeEvent( QCloseEvent * e )
{
    emit closed();
    QDialog::closeEvent( e );
}
