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
    if(QObjectList chldn = groupBox->queryList()) {
	QObjectList::ConstIterator obj = chldn.begin();
	while(*obj) {
	    if((*obj)->isWidgetType()) {
		QWidget *w = (QWidget *)(*obj);
		if(!w->isTopLevel())
		    w->setPalette(palette(), FALSE);
	    }
	    ++obj;
	}
    }
}

void WidgetsBase::setColor( const QString & color )
{
    groupBox->setPalette( QColor( color ), FALSE );
    if(QObjectList chldn = groupBox->queryList()) {
	QObjectList::ConstIterator obj = chldn.begin();
	while(*obj) {
	    if((*obj)->isWidgetType()) {
		QWidget *w = (QWidget *)(*obj);
		if(!w->isTopLevel())
		    w->setPalette(QColor(color), FALSE);
	    }
	    ++obj;
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
	colorTest->setPalette( QColor( color ) );
}

void WidgetsBase::updateDateTimeString()
{
	QDateTime dt;
    	dt.setDate( dateEdit->date() );
    	dt.setTime( timeEdit->time() );
    	dateTimeLabel->setText( dt.toString() );
}

