#include "qcategorywidget.h"
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qlayout.h>
#include <qscrollview.h>

QCategoryWidget::QCategoryWidget( QWidget *parent, const char *name )
    :  QWidget( parent, name )
{
    currentPage = 0;
    layout = new QVBoxLayout( this );
}

void QCategoryWidget::addCategory( const QString &name, QWidget *page )
{
    QToolButton *button = new QToolButton( this, name.latin1() );
    button->setText( name );
    button->setFixedHeight( button->sizeHint().height() );
    connect( button, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );
    QScrollView *sv = new QScrollView( this );
    sv->setResizePolicy( QScrollView::AutoOneFit );
    sv->addChild( page );
    sv->setFrameStyle( QFrame::NoFrame );
    page->show();
    pages.insert( button, sv );
    layout->addWidget( button );
    layout->addWidget( sv );
    button->show();
    if ( pages.count() == 1 ) {
	currentPage = sv;
	sv->show();
    } else {
	sv->hide();
    }
}

void QCategoryWidget::buttonClicked()
{
    QToolButton *tb = (QToolButton*)sender();
    QWidget *page = pages.find( tb );
    if ( !page )
	return;
    if ( currentPage )
	currentPage->hide();
    currentPage = page;
    currentPage->show();
}
