/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of pen input character setup
**
** Created : 20000414
**
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qcombobox.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include "qimpenwidget.h"
#include "qimpensetup.h"

class CharListItem : public QListBoxText
{
public:
    CharListItem( const QString &text, uint c )
	: QListBoxText( text )
    {
	_code = c;
    }

    uint code() const { return _code; }

protected:
    uint _code;
};

/*!
  \class QIMPenSetup qimpensetup.h

  Class to allow users to input totally useless character definitions
  which could match any number of the default set.
*/

QIMPenSetup::QIMPenSetup( QList<QIMPenCharSet> *cs, QWidget *parent,
                const char *name, bool modal, int WFlags )
    : QDialog( parent, name, modal, WFlags )
{
    charSets = cs;
    currentChar = 0;
    currentCode = 0;
    inputChar = new QIMPenChar();

    QVBoxLayout *tvb = new QVBoxLayout( this, 5 );

    QGridLayout *gl = new QGridLayout( tvb, 4, 2 );
    gl->setRowStretch( 1, 1 );
    gl->addRowSpacing( 2, 35 );
    gl->addRowSpacing( 3, 35 );

    QComboBox *cb = new QComboBox( this );
    gl->addMultiCellWidget( cb, 0, 0, 0, 1 );
    connect( cb, SIGNAL(activated(int)), SLOT(selectCharSet(int)));
    QListIterator<QIMPenCharSet> it( *cs );
    for ( ; it.current(); ++it ) {
        cb->insertItem( it.current()->description() );
    }

    charList = new QListBox( this );
    charList->setMinimumHeight( charList->sizeHint().height() );
    connect( charList, SIGNAL(highlighted(int)), SLOT(selectChar(int)) );
    gl->addWidget( charList, 1, 0 );

    pw = new QIMPenWidget( this );
    pw->setFixedHeight( 75 );
    gl->addMultiCellWidget( pw, 2, 3, 0, 0 );
    connect( pw, SIGNAL(stroke(QIMPenStroke *)),
                 SLOT(newStroke(QIMPenStroke *)) );

    QVBoxLayout *vb = new QVBoxLayout();
    gl->addLayout( vb, 1, 1 );
    QPushButton *pb = new QPushButton( "Add", this );
    connect( pb, SIGNAL(clicked()), SLOT(addChar()) );
    vb->addWidget( pb );

    pb = new QPushButton( "Remove", this );
    connect( pb, SIGNAL(clicked()), SLOT(removeChar()) );
    vb->addWidget( pb );

    pb = new QPushButton( "Default", this );
    connect( pb, SIGNAL(clicked()), SLOT(defaultChars()) );
    vb->addWidget( pb );

    QHBoxLayout *hb = new QHBoxLayout();
    gl->addLayout( hb, 2, 1 );
    prevBtn = new QPushButton( "<", this );
    connect( prevBtn, SIGNAL(clicked()), SLOT(prevChar()));
    hb->addWidget( prevBtn );

    nextBtn = new QPushButton( ">", this );
    connect( nextBtn, SIGNAL(clicked()), SLOT(nextChar()));
    hb->addWidget( nextBtn );

    pb = new QPushButton( "Clear", this );
    connect( pb, SIGNAL(clicked()), SLOT(clearChar()) );
    gl->addWidget( pb, 3, 1 );

    //--
    hb = new QHBoxLayout( tvb );
    pb = new QPushButton( "OK", this );
    connect( pb, SIGNAL(clicked()), SLOT(accept()) );
    hb->addWidget( pb );

    pb = new QPushButton( "Cancel", this );
    connect( pb, SIGNAL(clicked()), SLOT(reject()) );
    hb->addWidget( pb );

    selectCharSet( 0 );

    resize( minimumSize() );
}

/*!
  Fill the character list box with the characters.  Duplicates are not
  inserted.
*/
void QIMPenSetup::fillCharList()
{
    charList->clear();
    QList<QIMPenChar> chars = currentSet->characters();
    QListIterator<QIMPenChar> it(chars);
    CharListItem *li = 0;
    for ( ; it.current(); ++it ) {
	uint ch = it.current()->character();
        if ( (ch & 0x0000FFFF) == 0 ) {
            int code = it.current()->character() >> 16;
            for ( int i = 0; qimpen_specialKeys[i].code != Key_unknown; i++ ) {
                if ( qimpen_specialKeys[i].code == code ) {
		    li = new CharListItem( qimpen_specialKeys[i].name, ch );
		    break;
		}
            }
        } else {
	    li = new CharListItem( 
			QString( QChar(it.current()->character()) ), ch );
	}
	if ( li && !charList->findItem( li->text() ) )
	    charList->insertItem( li );
	else
	    delete li;
	li = 0;
    }
    currentChar = 0;
}

/*!
  Find the previous character with the same code as the current one.
  returns 0 if there is no previous character.
*/
QIMPenChar *QIMPenSetup::findPrev()
{
    if ( !currentChar )
	return 0;
    QList<QIMPenChar> chars = currentSet->characters();
    QListIterator<QIMPenChar> it(chars);
    bool found = FALSE;
    for ( it.toLast(); it.current(); --it ) {
        if ( !found && it.current() == currentChar )
            found = TRUE;
        else if ( found && it.current()->character() == currentCode &&
                !it.current()->testFlag( QIMPenChar::Deleted ) ) {
            return it.current();
        }
    }

    return 0;
}

/*!
  Find the next character with the same code as the current one.
  returns 0 if there is no next character.
*/
QIMPenChar *QIMPenSetup::findNext()
{
    if ( !currentChar )
	return 0;
    QList<QIMPenChar> chars = currentSet->characters();
    QListIterator<QIMPenChar> it(chars);
    bool found = FALSE;
    for ( ; it.current(); ++it ) {
	if ( !found && it.current() == currentChar )
	    found = TRUE;
	else if ( found && it.current()->character() == currentCode &&
		    !it.current()->testFlag( QIMPenChar::Deleted ) ) {
	    return it.current();
	}
    }

    return 0;
}

void QIMPenSetup::setCurrentChar( QIMPenChar *pc )
{
    currentChar = pc;
    pw->showCharacter( currentChar );
    if ( currentChar ) {
	prevBtn->setEnabled( findPrev() != 0 );
	nextBtn->setEnabled( findNext() != 0 );
    }
}

void QIMPenSetup::prevChar()
{
    QIMPenChar *pc = findPrev();
    if ( pc )
	setCurrentChar( pc );
}

void QIMPenSetup::nextChar()
{
    QIMPenChar *pc = findNext();
    if ( pc )
	setCurrentChar( pc );
}

void QIMPenSetup::clearChar()
{
    inputChar->clear();
    pw->clear();
}

void QIMPenSetup::selectChar( int i )
{
    currentChar = 0;
    currentCode = ((CharListItem *)charList->item(i))->code();
    QList<QIMPenChar> chars = currentSet->characters();
    QListIterator<QIMPenChar> it(chars);
    for ( ; it.current(); ++it ) {
	if ( it.current()->character() == currentCode &&
	     !it.current()->testFlag( QIMPenChar::Deleted ) ) {
	    setCurrentChar( it.current() );
	    break;
	}
    }
    if ( !it.current() )
	setCurrentChar( 0 );
    inputChar->clear();
}

void QIMPenSetup::selectCharSet( int i )
{
    if ( currentSet )
        pw->removeCharSet( currentSet );
    currentSet = charSets->at( i );
    fillCharList();
    pw->addCharSet( currentSet );
    inputChar->clear();
    if ( charList->count() ) {
        charList->setSelected( 0, TRUE );
        selectChar(0);
    }
}

void QIMPenSetup::addChar()
{
    if ( !inputChar->isEmpty() ) {
        QIMPenChar *pc = new QIMPenChar( *inputChar );
        pc->setCharacter( currentCode );
        currentSet->addChar( pc );
        setCurrentChar( pc );
        inputChar->clear();
    }
}

void QIMPenSetup::removeChar()
{
    if ( currentChar ) {
	QIMPenChar *pc = findPrev();
	if ( !pc ) pc = findNext();
	if ( currentChar->testFlag( QIMPenChar::System ) )
	    currentChar->setFlag( QIMPenChar::Deleted );
	else
	    currentSet->removeChar( currentChar );
	setCurrentChar( pc );
    }
}

void QIMPenSetup::defaultChars()
{
    if ( currentCode ) {
	currentChar = 0;
	QList<QIMPenChar> chars = currentSet->characters();
	QListIterator<QIMPenChar> it(chars);
	for ( ; it.current(); ++it ) {
	    if ( it.current()->character() == currentCode ) {
		if ( it.current()->testFlag( QIMPenChar::System ) ) {
		    it.current()->clearFlag( QIMPenChar::Deleted );
		    if ( !currentChar )
			currentChar = it.current();
		}
		else
		    currentSet->removeChar( it.current() );
	    }
	}
	setCurrentChar( currentChar );
    }
}

void QIMPenSetup::newStroke( QIMPenStroke *st )
{
    inputChar->addStroke( st );
}

