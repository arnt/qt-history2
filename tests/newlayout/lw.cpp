/****************************************************************************
** $Id: //depot/qt/main/tests/newlayout/lw.cpp#1 $
**
** QGridLayout example
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/
#include <qapp.h>
#include <qlabel.h>
#include <qmlined.h>
#include <qcolor.h>
#include <qgrpbox.h> 
#include <qpushbt.h>
#include <qlayout.h>



RCSTAG("$Id: //depot/qt/main/tests/newlayout/lw.cpp#1 $");


#include <qwidget.h>
class QHBox : public QWidget
{
    Q_OBJECT
public:
    QHBox( QWidget *parent=0, const char *name=0 );
    bool event( QEvent * );
protected:
    virtual void childEvent( QChildEvent * );
private:
    QHBoxLayout *lay;
};

/*!
  Constructs an hbox widget with parent \a parent and name \a name
 */
QHBox::QHBox( QWidget *parent, const char *name )
    :QWidget( parent, name )
{
    lay = new QHBoxLayout( this, 5, -1, name ); //### border
}

void QHBox::childEvent( QChildEvent *c )
{
    if ( !c->inserted() ) 
	return;
    QWidget *w = c->child();
    w->setAutoMinimumSize( TRUE );
    w->setMinimumSize( w->sizeHint() );
    lay->addWidget( w );
    if ( isVisible() )
	lay->activate();
}


/*!
  Provides childEvent() while waiting for Qt 2.0.
 */
bool QHBox::event( QEvent *e ) {
    switch ( e->type() ) {
    case Event_ChildInserted:
    case Event_ChildRemoved:
	childEvent( (QChildEvent*)e );
	return TRUE;
    default:
	return QWidget::event( e );
    }
}


class QGrid : public QWidget
{
    Q_OBJECT
public:
    QGrid( int rows, int cols, QWidget *parent=0, const char *name=0 );
    bool event( QEvent * );
protected:
    virtual void childEvent( QChildEvent * );
private:
    QGridLayout *lay;
    int row;
    int col;
    int nRows, nCols;

};

/*!
  Constructs a \a rows x \a cols grid widget with parent \a parent and name \a name
 */
QGrid::QGrid( int rows, int cols, QWidget *parent, const char *name )
    :QWidget( parent, name )
{
    lay = new QGridLayout( this, rows, cols, 5, -1, name ); //### border
    nCols = cols;
    nRows = rows;
    row = col = 0;
}

void QGrid::childEvent( QChildEvent *c )
{
    if ( !c->inserted() ) 
	return;
    QWidget *w = c->child();
    w->setAutoMinimumSize( TRUE );
    w->setMinimumSize( w->sizeHint() );
    lay->addWidget( w, row, col );
    debug( "adding %d,%d", row, col );
    if ( col+1 < nCols ) {
	col++;
    } else if ( row+1 < nRows ) {
	col = 0;
	row++;
    }
    if ( isVisible() )
	lay->activate();
}


/*!
  Provides childEvent() while waiting for Qt 2.0.
 */
bool QGrid::event( QEvent *e ) {
    switch ( e->type() ) {
    case Event_ChildInserted:
    case Event_ChildRemoved:
	childEvent( (QChildEvent*)e );
	return TRUE;
    default:
	return QWidget::event( e );
    }
}


#include "lw.moc"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    //QHBox *f = new QHBox;
	QGrid *f = new QGrid( 2, 2 );

    QLabel* l1 = new QLabel(f);
    l1->setText("This is label 1.");
    l1->setBackgroundColor( yellow );

    QLabel* l2 = new QLabel(f);
    l2->setText("This\nis\nlabel\ntoo.");
    l2->setBackgroundColor( red );

    QLabel* l3 = new QLabel(f);
    l3->setText("This is label III.");
    l3->setBackgroundColor( red );

    QLabel* l4 = new QLabel(f);
    l4->setText("More label.");
    l4->setBackgroundColor( cyan );

    f->show();

    a.setMainWidget(f);
    a.exec();
}


#if 0


    f->setFrameStyle( QFrame::Panel  | QFrame::Sunken );

    QGridLayout *gm = new QGridLayout( f, 4, 4, 5 );


    gm->addColSpacing(3,25);
    gm->setColStretch(2,10);
    gm->setRowStretch(2,5);
    /////////////////////////////////////////////////////////

    QPushButton* qb = new QPushButton( "Quit", f );
    a.connect( qb, SIGNAL(clicked()), SLOT(quit()) );
    qb->setFixedSize( qb->size() );
    gm->addWidget( qb, 0, 2, AlignCenter );
    

    QMultiLineEdit *ed = new QMultiLineEdit(f);
    ed->setText("This is supposed to be a large window\n you know.");
    ed->setMinimumSize( 150, 150 );
    //gm->addWidget( ed, 1, 1 );
    gm->addMultiCellWidget( ed, 1, 1, 1, 2 );


    ////////////////////////////////////////////////////////
    gm->activate();
    //gm->freeze();
#endif
