/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of pen input character set maintenance.
** Probably a developer-only tool.
**
** Created : 20000414
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include "qimpenwidget.h"
#include "qimpencreateset.h"


QIMPenInputCharDlg::QIMPenInputCharDlg( QWidget *parent, const char *name,
                                        bool modal, int WFlags)
    : QDialog( parent, name, modal, WFlags )
{
    uni = 0;

    QVBoxLayout *vb = new QVBoxLayout( this, 10 );

    QHBoxLayout *hb = new QHBoxLayout();
    vb->addLayout( hb );

    QLabel *label = new QLabel( "Character:", this );
    hb->addWidget( label );

    QComboBox *cb = new QComboBox( TRUE, this );
    connect( cb, SIGNAL(activated(int)), SLOT(setSpecial(int)) );
    connect( cb, SIGNAL(textChanged(const QString &)),
                 SLOT(setCharacter(const QString &)) );
    addSpecial( cb );
    cb->setEditText( "" );
    hb->addWidget( cb );

    hb = new QHBoxLayout();
    vb->addLayout( hb );

    QPushButton *pb = new QPushButton( "OK", this );
    connect( pb, SIGNAL(clicked()), SLOT(accept()));
    hb->addWidget( pb );
    pb = new QPushButton( "Cancel", this );
    connect( pb, SIGNAL(clicked()), SLOT(reject()));
    hb->addWidget( pb );

    cb->setFocus();
}

void QIMPenInputCharDlg::addSpecial( QComboBox *cb )
{
    int i = 0;
    while ( qimpen_specialKeys[i].code != Key_unknown ) {
        cb->insertItem( qimpen_specialKeys[i].name );
        i++;
    }
}

void QIMPenInputCharDlg::setSpecial( int sp )
{
    uni = qimpen_specialKeys[sp].code << 16;
}

void QIMPenInputCharDlg::setCharacter( const QString &string )
{
    uni = string[0].unicode();
}

//===========================================================================

QIMPenCreateCharSet::QIMPenCreateCharSet() : QWidget()
{
    charSet = new QIMPenCharSet;
    editChar = new QIMPenChar;
    penChar = 0;

    QHBoxLayout *hb = new QHBoxLayout( this, 10 );
    QVBoxLayout *vb = new QVBoxLayout( hb );

    QHBoxLayout *hb1 = new QHBoxLayout( vb );
    QLabel *label = new QLabel( "Title", this );
    hb1->addWidget( label );
    title = new QLineEdit( this );
    hb1->addWidget( title );

    hb1 = new QHBoxLayout( vb );
    label = new QLabel( "Desc.", this );
    hb1->addWidget( label );
    desc = new QLineEdit( this );
    hb1->addWidget( desc );

    charList = new QListBox( this );
    connect( charList, SIGNAL(highlighted(int)), SLOT(charSelected(int)) );
    charList->setMinimumHeight( charList->sizeHint().height() );
    vb->addWidget( charList, 1 );

    pw = new QIMPenWidget( this );
    connect( pw, SIGNAL(stroke(QIMPenStroke *)),
                 SLOT(newStroke(QIMPenStroke *)));
    pw->setFixedHeight( 75 );
    pw->setMinimumWidth( 75 );
    pw->addCharSet( charSet );
    vb->addWidget( pw );

    vb = new QVBoxLayout( hb );
    QPushButton *pb = new QPushButton( "&Load...", this );
    connect( pb, SIGNAL(clicked()), SLOT(load()));
    vb->addWidget( pb );

    pb = new QPushButton( "&Save...", this );
    connect( pb, SIGNAL(clicked()), SLOT(save()));
    vb->addWidget( pb );

    vb->addStretch( 1 );

    pb = new QPushButton( "&Up", this );
    pb->setAutoRepeat( TRUE );
    connect( pb, SIGNAL(clicked()), SLOT(up()));
    vb->addWidget( pb );

    pb = new QPushButton( "&Down", this );
    pb->setAutoRepeat( TRUE );
    connect( pb, SIGNAL(clicked()), SLOT(down()));
    vb->addWidget( pb );


    pb = new QPushButton( "&Add...", this );
    connect( pb, SIGNAL(clicked()), SLOT(add()));
    vb->addWidget( pb );

    pb = new QPushButton( "&Remove", this );
    connect( pb, SIGNAL(clicked()), SLOT(remove()));
    vb->addWidget( pb );

    pb = new QPushButton( "&Clear", this );
    connect( pb, SIGNAL(clicked()), SLOT(clear()));
    vb->addWidget( pb );
}

QIMPenCreateCharSet::~QIMPenCreateCharSet()
{
    delete charSet;
    delete editChar;
}

void QIMPenCreateCharSet::load()
{
    QString fn = QFileDialog::getOpenFileName( filename );
    if ( !fn.isEmpty() ) {
        filename = fn;
        charSet->load( filename );
        pw->removeCharSet( charSet );
        pw->addCharSet( charSet );
        fillList();
        title->setText( charSet->title() );
        desc->setText( charSet->description() );
        setCaption( filename );
    }
}

void QIMPenCreateCharSet::save()
{
    QString fn = QFileDialog::getSaveFileName( filename );
    if ( !fn.isEmpty() ) {
        filename = fn;
        charSet->setFilename( filename );
        charSet->setTitle( title->text() );
        charSet->setDescription( desc->text() );
        charSet->save();
    }
}

void QIMPenCreateCharSet::up()
{
    if ( !penChar )
        return;

    charSet->up( penChar );
    fillList();
    int idx = charSet->characters().findRef( penChar );
    charList->setSelected( idx, TRUE );
    editChar->clear();
}

void QIMPenCreateCharSet::down()
{
    if ( !penChar )
        return;

    charSet->down( penChar );
    fillList();
    int idx = charSet->characters().findRef( penChar );
    charList->setSelected( idx, TRUE );
    editChar->clear();
}

void QIMPenCreateCharSet::add()
{
    if ( !penChar )
        return;

    QIMPenInputCharDlg dlg( 0, 0, TRUE );
    if ( dlg.exec() ) {
        QIMPenChar *pc = new QIMPenChar( *penChar );
        pc->setCharacter( dlg.unicode() );
        pc->setFlag( QIMPenChar::System );
        charSet->addChar( pc );
        penChar = pc;
        fillList();
        charList->setSelected( charList->count() - 1, TRUE );
        editChar->clear();
    }
}

void QIMPenCreateCharSet::remove()
{
    int idx = charSet->characters().findRef( penChar );
    if ( idx >= 0 ) {
        charSet->removeChar( penChar );
        charList->removeItem( idx );
        penChar = 0;
    }
    editChar->clear();
}

void QIMPenCreateCharSet::clear()
{
    editChar->clear();
    pw->clear();
}

void QIMPenCreateCharSet::newStroke( QIMPenStroke *st )
{
    penChar = editChar;
    editChar->addStroke( st );
    qDebug("new stroke");
}

void QIMPenCreateCharSet::charSelected( int idx )
{
    penChar = charSet->characters().at( idx );
    pw->showCharacter( penChar );
}

void QIMPenCreateCharSet::fillList()
{
    charList->clear();
    QList<QIMPenChar> chars = charSet->characters();
    QListIterator<QIMPenChar> it(chars);
    for ( ; it.current(); ++it ) {
        if ( (it.current()->character() & 0x0000FFFF) == 0 ) {
            int code = it.current()->character() >> 16;
            for ( int i = 0; qimpen_specialKeys[i].code != Key_unknown; i++ ) {
                if ( qimpen_specialKeys[i].code == code ) {
                    charList->insertItem( qimpen_specialKeys[i].name );
		    break;
		}
            }
        } else {
            charList->insertItem( QString( QChar(it.current()->character()) ) );
        }
    }
}

//===========================================================================

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    QIMPenCreateCharSet w;
    w.show();

    app.setMainWidget( &w );
    app.exec();
}

