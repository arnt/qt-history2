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


void WidgetsBase::init()
{
    timeEdit->setTime( QTime::currentTime() );
    dateEdit->setDate( QDate::currentDate() );
}

void WidgetsBase::destroy()
{

}

void WidgetsBase::resetColors()
{
    groupBox->setPalette( palette(), FALSE );
    QObjectList chldn = groupBox->queryList();
    for (int i = 0; i < chldn.size(); ++i) {
	QObject *obj = chldn.at(i);
	if(obj->isWidgetType()) {
	    QWidget *w = (QWidget *)obj;
	    if(!w->isTopLevel())
		w->setPalette(palette(), FALSE);
	}
    }
}

void WidgetsBase::setColor( const QString & color )
{
    groupBox->setPalette( QColor( color ), FALSE );
    QObjectList chldn = groupBox->queryList();
    for (int i = 0; i < chldn.size(); ++i) {
	QObject *obj = chldn.at(i);
	if(obj->isWidgetType()) {
	    QWidget *w = (QWidget *)obj;
	    if(!w->isTopLevel())
		w->setPalette(QColor(color), FALSE);
	}
    }
}

void WidgetsBase::setColor()
{
    setColor( lineEdit->text() );
}

void WidgetsBase::updateClock()
{
    clock->setTime( timeEdit->time() );
}

void WidgetsBase::updateColorTest( const QString & color )
{
    colorTest->setPalette( QColor( color ), TRUE);
}

void WidgetsBase::updateDateTimeString()
{
    QDateTime dt;
    dt.setDate( dateEdit->date() );
    dt.setTime( timeEdit->time() );
    dateTimeLabel->setText( dt.toString() );
}

