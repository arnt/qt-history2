void MainView::url_dataTransferProgress(int,int,QNetworkOperation*)
{
}

void MainView::init()
{
    connect( &urlOp, SIGNAL(connectionStateChanged( int, const QString&)),
    	SLOT(url_connectionStateChanged(int, const QString&)) );
    connect( &urlOp, SIGNAL(createdDirectory(const QUrlInfo&, QNetworkOperation*)),
    	SLOT(url_createdDirectory(const QUrlInfo&, QNetworkOperation*)) ); 
    connect( &urlOp, SIGNAL(data(const QByteArray&, QNetworkOperation*)),
    	SLOT(url_data(const QByteArray&, QNetworkOperation*)) );  
    connect( &urlOp, SIGNAL(dataTransferProgress(int,int,QNetworkOperation*)),
    	SLOT(url_dataTransferProgress(int,int, QNetworkOperation*)) );  
    connect( &urlOp, SIGNAL(finished(QNetworkOperation*)),
    	SLOT(url_finished(QNetworkOperation*)) );  
    connect( &urlOp, SIGNAL(itemChanged(QNetworkOperation*)),
    	SLOT(url_itemChanged(QNetworkOperation*)) );  
    connect( &urlOp, SIGNAL(newChildren(const QValueList<QUrlInfo>&,QNetworkOperation*)),
    	SLOT(url_newChildren(const QValueList<QUrlInfo>&, QNetworkOperation*)) );  
    connect( &urlOp, SIGNAL(removed (QNetworkOperation*)),
    	SLOT(url_removed(QNetworkOperation*)) );  
    connect( &urlOp, SIGNAL(start(QNetworkOperation*)),
    	SLOT(url_start(QNetworkOperation*)) );  
    connect( &urlOp, SIGNAL(startedNextCopy(const QPtrList<QNetworkOperation>&)),
    	SLOT(url_startedNextCopy(const QPtrList<QNetworkOperation>&)) ); 
}

void MainView::quit()
{
    QApplication::exit();
}

void MainView::start_copy()
{
    logWindow->append( "START: copy()\n" );
    urlOp = urlEdit->displayText(); 
    urlOp.copy( copyFrom->displayText(), copyTo->displayText(), copyMove->isChecked() );  
}

void MainView::start_get()
{
    logWindow->append( "START: get()\n" ); 
    urlOp = urlEdit->displayText();
    QString location = getLocation->displayText();
    if ( location.isEmpty() )
	urlOp.get();
    else
	urlOp.get( location );
}

void MainView::start_listChildren()
{
    logWindow->append( "START: listChildren()\n" );
    urlOp = urlEdit->displayText();
    urlOp.listChildren();
}

void MainView::start_mkdir()
{
    logWindow->append( "START: mkdir()\n" );
    urlOp = urlEdit->displayText();
    urlOp.mkdir( mkdirDirName->displayText() );
}

void MainView::start_put()
{
    logWindow->append( "START: put()\n" );
    urlOp = urlEdit->displayText(); 
    QString location = getLocation->displayText(); 
    QByteArray ba( 50 );
    for ( int i=0; i<50; i++ )
	ba[i] = 'a' + i%26;
    if ( location.isEmpty() ) 
	urlOp.put( ba ); 
    else 
	urlOp.put( ba, location ); 
}

void MainView::start_remove()
{
    logWindow->append( "START: remove()\n" );  
    urlOp = urlEdit->displayText(); 
    urlOp.remove( removeFileName->displayText() ); 
}

void MainView::start_rename()
{
    logWindow->append( "START: rename()\n" );  
    urlOp = urlEdit->displayText(); 
    urlOp.rename( renameOldName->displayText(), renameNewName->displayText() ); 
}

void MainView::stop()
{
    urlOp.stop();
}

void MainView::url_createdDirectory( const QUrlInfo&, QNetworkOperation* )
{
    logWindow->append( "SIGNAL: createdDirectory\n" ); 
}

void MainView::url_data(const QByteArray&, QNetworkOperation* )
{
    logWindow->append( "SIGNAL: data\n" ); 
}

void MainView::url_finished( QNetworkOperation* )
{
    logWindow->append( "SIGNAL: finished\n" ); 
}

void MainView::url_itemChanged( QNetworkOperation* )
{
    logWindow->append( "SIGNAL: itemChanged\n" ); 
}

void MainView::url_newChildren( const QValueList<QUrlInfo>&, QNetworkOperation* )
{
    logWindow->append( "SIGNAL: newChildren\n" ); 
}

void MainView::url_removed( QNetworkOperation* )
{
    logWindow->append( "SIGNAL: removed\n" ); 
}

void MainView::url_start( QNetworkOperation* )
{
    logWindow->append( "SIGNAL: start\n" ); 
}

void MainView::url_startedNextCopy( const QPtrList<QNetworkOperation>& )
{
    logWindow->append( "SIGNAL: startedNextCopy\n" );  
}

void MainView::url_connectionStateChanged( int i, const QString &s )
{
    QString enumStr;
    switch (i) {
	case QNetworkProtocol::ConHostFound:
	    enumStr = "ConHostFound";
	    break;
	case QNetworkProtocol::ConConnected:
	    enumStr = "ConConnected";
	    break;
	case QNetworkProtocol::ConClosed :
	    enumStr = "ConClosed";
	    break;
	default:
	    enumStr = "(Unknown)";
	    break;
    }
    logWindow->append( QString("SIGNAL: connectionStateChanged( %1, \"%2\" )\n").arg(enumStr).arg(s) );
}
