/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qlayout.h>
#include <qslider.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qmessagebox.h>

#include <qaxbindable.h>
#include <qaxfactory.h>

class QSimpleAX : public QWidget, public QAxBindable
{
    Q_OBJECT
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( int value READ value WRITE setValue )
public:
    QSimpleAX(QWidget *parent = 0)
    : QWidget(parent)
    {
	QVBoxLayout *vbox = new QVBoxLayout( this );

        slider = new QSlider( Qt::Horizontal, this );
	LCD = new QLCDNumber( 3, this );
	edit = new QLineEdit( this );

	connect( slider, SIGNAL( valueChanged( int ) ), this, SLOT( setValue(int) ) );
	connect( edit, SIGNAL(textChanged(const QString&)), this, SLOT(setText(const QString&)) );

	vbox->addWidget( slider );
	vbox->addWidget( LCD );
	vbox->addWidget( edit );
    }

    QString text() const 
    { 
	return edit->text(); 
    }
    int value() const
    {
	return slider->value();
    }

signals:
    void someSignal();
    void valueChanged(int);
    void textChanged(const QString&);

public slots:
    void setText( const QString &string )
    {
	if ( !requestPropertyChange( "text" ) )
	    return;

	edit->blockSignals( TRUE );
	edit->setText( string );
	edit->blockSignals( FALSE );
	emit someSignal();
	emit textChanged( string );

	propertyChanged( "text" );
    }
    void about()
    {
	QMessageBox::information( this, "About QSimpleAX", "This is a Qt widget, and this slot has been\n"
							  "called through ActiveX/OLE automation!" );
    }
    void setValue( int i )
    {
	if ( !requestPropertyChange( "value" ) )
	    return;
	slider->blockSignals( TRUE );
	slider->setValue( i );
	slider->blockSignals( FALSE );
	LCD->display( i );
	emit valueChanged( i );

	propertyChanged( "value" );
    }

private:
    QSlider *slider;
    QLCDNumber *LCD;
    QLineEdit *edit;
};

#include "main.moc"

QAXFACTORY_DEFAULT(QSimpleAX,
	   "{DF16845C-92CD-4AAB-A982-EB9840E74669}",
	   "{616F620B-91C5-4410-A74E-6B81C76FFFE0}",
	   "{E1816BBA-BF5D-4A31-9855-D6BA432055FF}",
	   "{EC08F8FC-2754-47AB-8EFE-56A54057F34E}",	   
	   "{A095BA0C-224F-4933-A458-2DD7F6B85D8F}")
