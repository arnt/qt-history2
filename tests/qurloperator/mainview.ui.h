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
    logMessage( "START: copy()", 0 );
    urlOp = urlEdit->displayText(); 
    urlOp.copy( copyFrom->displayText(), copyTo->displayText(), copyMove->isChecked() );  
}

void MainView::start_get()
{
    logMessage( "START: get()", 0 ); 
    urlOp = urlEdit->displayText();
    QString location = getLocation->displayText();
    if ( location.isEmpty() )
	urlOp.get();
    else
	urlOp.get( location );
}

void MainView::start_listChildren()
{
    logMessage( "START: listChildren()", 0 );
    urlOp = urlEdit->displayText();
    urlOp.listChildren();
}

void MainView::start_mkdir()
{
    logMessage( "START: mkdir()", 0 );
    urlOp = urlEdit->displayText();
    urlOp.mkdir( mkdirDirName->displayText() );
}

void MainView::start_put()
{
    logMessage( "START: put()", 0 );
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
    logMessage( "START: remove()", 0 );  
    urlOp = urlEdit->displayText(); 
    urlOp.remove( removeFileName->displayText() ); 
}

void MainView::start_rename()
{
    logMessage( "START: rename()", 0 );  
    urlOp = urlEdit->displayText(); 
    urlOp.rename( renameOldName->displayText(), renameNewName->displayText() ); 
}

void MainView::stop()
{
    urlOp.stop();
}

void MainView::url_createdDirectory( const QUrlInfo&, QNetworkOperation *no )
{
    logMessage( "SIGNAL: createdDirectory", no ); 
}

void MainView::url_data(const QByteArray&, QNetworkOperation *no )
{
    logMessage( "SIGNAL: data", no ); 
}

void MainView::url_dataTransferProgress( int i, int j, QNetworkOperation *no)
{
    logMessage( QString("SIGNAL: dataTransferProgress( %1, %2 )").arg(i).arg(j), no ); 
}

void MainView::url_finished( QNetworkOperation *no )
{
    logMessage( "SIGNAL: finished", no ); 
}

void MainView::url_itemChanged( QNetworkOperation *no )
{
    logMessage( "SIGNAL: itemChanged", no ); 
}

void MainView::url_newChildren( const QValueList<QUrlInfo>&, QNetworkOperation *no )
{
    logMessage( "SIGNAL: newChildren", no ); 
}

void MainView::url_removed( QNetworkOperation *no )
{
    logMessage( "SIGNAL: removed", no ); 
}

void MainView::url_start( QNetworkOperation *no )
{
    logMessage( "SIGNAL: start", no ); 
}

void MainView::url_startedNextCopy( const QPtrList<QNetworkOperation>& )
{
    logMessage( "SIGNAL: startedNextCopy", 0 );  
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
    logMessage( QString("SIGNAL: connectionStateChanged( %1, \"%2\" )").arg(enumStr).arg(s), 0 );
}


void MainView::logMessage( const QString &msg, QNetworkOperation * )
{
    logWindow->append( msg + "\n" );
}
