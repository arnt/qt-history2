#include "search.h"
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qdatetimeedit.h>
#include <qsqltable.h>
#include <qsqlcursor.h>
#include <qsqlrecord.h>
#include <qapplication.h>

Search::Search( QWidget * parent, const char * name )
    : SearchBase( parent, name )
{
    cursor = new QSqlCursor( "key_test" );
    cursor->select( cursor->primaryIndex() );
    cursor->next();
    QSqlRecord * r = cursor->editBuffer();
    table->setCursor( cursor );
}


void Search::close()
{
    qApp->quit();
}

void Search::search()
{
    table->find( searchString->text(), caseSensitive->isChecked(), 
		 backwards->isChecked() );
}

