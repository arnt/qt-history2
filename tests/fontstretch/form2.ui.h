/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qapplication.h>
#include <qfont.h>


void Form2::init()
{
    QFont f = QApplication::font();

    f.setStyleStrategy( (QFont::StyleStrategy) ( QFont::ForceOutline |
						 QFont::PreferAntialias ) );
    f.setPointSize( f.pointSize() * 2 );

    f.setStretch( QFont::UltraCondensed );
    ultracondensed->setFont( f );

    f.setStretch( QFont::ExtraCondensed );
    extracondensed->setFont( f );

    f.setStretch( QFont::Condensed );
    condensed->setFont( f );

    f.setStretch( QFont::SemiCondensed );
    semicondensed->setFont( f );

    f.setStretch( QFont::Unstretched );
    unstretched->setFont( f );

    f.setStretch( QFont::SemiExpanded );
    semiexpanded->setFont( f );

    f.setStretch( QFont::Expanded );
    expanded->setFont( f );

    f.setStretch( QFont::ExtraExpanded );
    extraexpanded->setFont( f );

    f.setStretch( QFont::UltraExpanded );
    ultraexpanded->setFont( f );
}

