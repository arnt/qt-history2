/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qaxfactory.h>
#include <qfontmetrics.h>
#include <qmessagebox.h>
#include <qpainter.h>

class QSimpleAX : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( int value READ value WRITE setValue )
public:
    QSimpleAX( QWidget *parent = 0, const char *name = 0 )
    : QWidget( parent, name ), number(0)
    {
    }

    QString text() const
    {
	return string;
    }
    int value() const
    {
	return number;
    }

    QSize sizeHint() const
    {
	QFontMetrics fm(font());
	int w = QMAX(fm.width(string), fm.width(QString::number(number))) + 10;
	int h = 2*fm.height() + 6;
	return QSize(w,h);
    }

signals:
    void valueChanged(int);
    void textChanged(const QString&);

public slots:
    void about()
    {
	QMessageBox::information( this, "About QSimpleAX", "This is a Qt widget, and this slot has been\n"
							  "called through ActiveX/OLE automation!" );
    }
    void setText( const QString &t )
    {
	string = t;
	repaint();

	updateGeometry();
	emit textChanged( string );
    }
    void setValue( int i )
    {
	number = i;
	repaint();

	updateGeometry();
	emit valueChanged( i );
    }

protected:
    void paintEvent(QPaintEvent *e)
    {
	QPainter p(this);
	QFontMetrics fm(font());

	int y = fm.height() + 3;
	p.drawText(5, y, string);
	y += fm.lineSpacing();
	p.drawText(5, y, QString::number(number));
    }

private:
    QString string;
    int number;
};

#include "main.moc"

QAXFACTORY_DEFAULT(QSimpleAX,
	   "{DF16845C-92CD-4AAB-A982-EB9840E74669}",
	   "{616F620B-91C5-4410-A74E-6B81C76FFFE0}",
	   "{E1816BBA-BF5D-4A31-9855-D6BA432055FF}",
	   "{EC08F8FC-2754-47AB-8EFE-56A54057F34E}",	   
	   "{A095BA0C-224F-4933-A458-2DD7F6B85D8F}")
