/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
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
