void PixmapCollectionEditor::init()
{
    project = 0;
    setChooserMode( FALSE );
    changed = FALSE;
}

void PixmapCollectionEditor::destroy()
{
}

void PixmapCollectionEditor::save()
{
    if ( changed )
	project->pixmapCollection()->createCppFile();     
}

void PixmapCollectionEditor::addPixmap()
{
    if ( !project )
	return;
    
    QString f;
    QPixmap pix = qChoosePixmap( this, 0, QPixmap(), &f ); 
    if ( pix.isNull() )
	return;
    PixmapCollection::Pixmap pixmap;
    pixmap.pix = pix;
    pixmap.name = QFileInfo( f ).baseName();
    project->pixmapCollection()->addPixmap( pixmap );
    updateView();
    changed = TRUE;
    QIconViewItem *item = viewPixmaps->findItem( pixmap.name );
    if ( item ) {
	viewPixmaps->setCurrentItem( item );
	viewPixmaps->ensureItemVisible( item );
    }
}

void PixmapCollectionEditor::removePixmap()
{
    if ( !project || !viewPixmaps->currentItem() )
	return;
    project->pixmapCollection()->removePixmap( viewPixmaps->currentItem()->text() );
    updateView();
    changed = TRUE;
}

void PixmapCollectionEditor::updateView()
{
    if ( !project )
	return;
 
    viewPixmaps->clear();
    
    QValueList<PixmapCollection::Pixmap> pixmaps = project->pixmapCollection()->pixmaps();
    for ( QValueList<PixmapCollection::Pixmap>::Iterator it = pixmaps.begin(); it != pixmaps.end(); ++it ) {
	// #### might need to scale down the pixmap
	QIconViewItem *item = new QIconViewItem( viewPixmaps, (*it).name, scaledPixmap( (*it).pix ) );
	//item->setRenameEnabled( TRUE ); // this will be a bit harder to implement
	item->setDragEnabled( FALSE );
	item->setDropEnabled( FALSE );
    }
    viewPixmaps->setCurrentItem( viewPixmaps->firstItem() );
    currentChanged( viewPixmaps->firstItem() );
}

void PixmapCollectionEditor::currentChanged( QIconViewItem * i )
{
    buttonOk->setEnabled( !!i );
}

void PixmapCollectionEditor::setChooserMode( bool c )
{
    chooser = c;
    if ( chooser ) {
	buttonClose->hide();
	buttonOk->show(); 
	buttonCancel->show(); 
	buttonOk->setEnabled( FALSE );
	buttonOk->setDefault( TRUE );
	connect( viewPixmaps, SIGNAL( doubleClicked( QIconViewItem * ) ), buttonOk, SIGNAL( clicked() ) );
	connect( viewPixmaps, SIGNAL( returnPressed( QIconViewItem * ) ), buttonOk, SIGNAL( clicked() ) ); 
	setCaption( tr( "Choose a Pixmap" ) );
    } else {
	buttonClose->show();
	buttonOk->hide();
	buttonCancel->hide();
	buttonClose->setDefault( TRUE );
    }
    updateView();
}

void PixmapCollectionEditor::setCurrentItem( const QString & name )
{
    QIconViewItem *i = viewPixmaps->findItem( name );
    if ( i ) {
	viewPixmaps->setCurrentItem( i );
	currentChanged( i );
    }
}

void PixmapCollectionEditor::setProject( Project * pro )
{
    project = pro;
    updateView();
}

QPixmap PixmapCollectionEditor::scaledPixmap( const QPixmap & p )
{
    QPixmap pix( p );
    if ( pix.width() < 50 && pix.height() < 50 )
	return pix;
    QImage img;
    img = pix;
    img = img.smoothScale( 50, 50 );
    pix.convertFromImage( img );
    return pix;
}

