#ifndef MODAL_H
#define MODAL_H

#include <qwidget.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qtoolbutton.h>	

class Dialog : public QDialog
{
    Q_OBJECT
public:
    Dialog( QWidget* parent = 0, const char* name = 0 )
	: QDialog( parent, name, TRUE )
    {
	QVBoxLayout* l = new QVBoxLayout( this );
	QPushButton* button = new QPushButton( "next", this );
	QWhatsThis::add( button, "Example whats this text");
	l->addWidget( button );
	connect( button, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );
	button = new QPushButton( "done", this );
	l->addWidget( button );
	connect( button, SIGNAL( clicked() ), this, SLOT( accept() ) );
	l->addWidget( QWhatsThis::whatsThisButton( this ) );
    }
    ~Dialog()
    {
    }

public slots:
    void buttonClicked()
    {	
	Dialog d( this );
	d.exec();
    }
    
private:
};

#endif
