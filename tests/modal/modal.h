#ifndef MODAL_H
#define MODAL_H

#include <qwidget.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlayout.h>

class Dialog : public QDialog
{
    Q_OBJECT
public:
    Dialog( QWidget* parent = 0, const char* name = 0 )
	: QDialog( parent, name, TRUE )
    {
	QVBoxLayout* l = new QVBoxLayout( this );
	QPushButton* button = new QPushButton( "next", this );
	l->addWidget( button );
	connect( button, SIGNAL( clicked() ), this, SLOT( buttonClicked() ) );
	button = new QPushButton( "done", this );
	l->addWidget( button );
	connect( button, SIGNAL( clicked() ), this, SLOT( accept() ) );
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
};

#endif
