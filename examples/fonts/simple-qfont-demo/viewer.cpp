/* $Id$ */

#include "viewer.h"
#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qfont.h>
 
Viewer::Viewer()
       :QWidget()  
{
    setFontSubstitutions();

    QString greeting_heb = QString::fromUtf8( "שלום" ); 
    QString greeting_ru = QString::fromUtf8( "Здравствуйте" ); 
    QString greeting_en( "Hallo" );      

    textView = new QTextView( this, "textview" );

    textView->setText( greeting_en + "\n" + 
                       greeting_ru + "\n" +
                       greeting_heb );

    fontInfo = new QTextView( this, "fontinfo" );

    setDefault();

    QPushButton * defaultButton = new QPushButton( "Default", this, 
                                                   "pushbutton1" );
    defaultButton->setFont( QFont( "times" ) );
    connect( defaultButton, SIGNAL( clicked() ), this, SLOT( setDefault() ) );

    QPushButton * sansSerifButton = new QPushButton( "Sans Serif", this, 
                                                     "pushbutton2" );
    sansSerifButton->setFont( QFont( "Helvetica", 12 ) );
    connect( sansSerifButton, SIGNAL( clicked() ), this, SLOT( setSansSerif() ) );

    QPushButton * italicsButton = new QPushButton( "Italics", this, 
                                                   "pushbutton3" );
    italicsButton->setFont( QFont( "lucida", 12, QFont::Bold, TRUE ) );
    connect( italicsButton, SIGNAL( clicked() ), this, SLOT( setItalics() ) );

    QHBoxLayout * textViewContainer = new QHBoxLayout(); 
    textViewContainer->addWidget( textView );
    textViewContainer->addWidget( fontInfo );

    QHBoxLayout * buttonContainer = new QHBoxLayout(); 

    buttonContainer->addWidget( defaultButton );
    buttonContainer->addWidget( sansSerifButton );
    buttonContainer->addWidget( italicsButton );

    int maxButtonHeight = defaultButton->height();

    if ( sansSerifButton->height() > maxButtonHeight )
	maxButtonHeight = sansSerifButton->height();
    if ( italicsButton->height() > maxButtonHeight )
        maxButtonHeight = italicsButton->height();

    defaultButton->setFixedHeight( maxButtonHeight );
    sansSerifButton->setFixedHeight( maxButtonHeight );
    italicsButton->setFixedHeight( maxButtonHeight );

    QVBoxLayout * container = new QVBoxLayout( this );
    container->addLayout( textViewContainer );
    container->addLayout( buttonContainer );

    resize( 400, 250 );
}                        

void Viewer::setDefault()
{
    QFont font( "Bavaria" );    
    font.setPointSize( 24 );

    font.setWeight( QFont::Bold );
    font.setUnderline( TRUE );

    textView->setFont( font );    

    showFontInfo( font );
}

void Viewer::setSansSerif()
{
    QFont font( "Newyork", 18 );    
    font.setStyleHint( QFont::SansSerif ); 
    textView->setFont( font );    

    showFontInfo( font );
}

void Viewer::setItalics()
{
    QFont font( "Tokyo" );
    font.setPointSize( 36 );
    font.setWeight( QFont::Bold );
    font.setItalic( TRUE );    

    textView->setFont( font );    

    showFontInfo( font );
}

void Viewer::showFontInfo( QFont & font )
{
    QString messageText;

    QFontInfo info( font );
    messageText = "Font requested: \"" + 
                  font.family() + "\" " + 
                  QString::number( font.pointSize() ) + "pt<BR>" +             
                  "Font used: \"" + 
                  info.family() + "\" " + 
                  QString::number( info.pointSize() ) + "pt<P>";

    QStringList substitutions = QFont::substitutes( font.family() );

    if ( ! substitutions.isEmpty() ){ 
	messageText += "The following substitutions exist for " + \
		       font.family() + ":<UL>";

	QStringList::Iterator i = substitutions.begin();
	while ( i != substitutions.end() ){
	    messageText += "<LI>\"" + (* i) + "\"";
	    i++;
	}
	 messageText += "</UL>";
    } else {
	messageText += "No substitutions exist for " + \
		       font.family() + ".";
    }

    fontInfo->setText( messageText );
}

void Viewer::setFontSubstitutions()
{
    QStringList substitutes;
    substitutes.append( "Times" );
    substitutes +=  "Mincho",
    substitutes << "Arabic Newspaper" << "crox";

    QFont::insertSubstitutions( "Bavaria", substitutes );

    QFont::insertSubstitution( "Tokyo", "Lucida" );
}
