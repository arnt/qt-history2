void PixmapCollectionEditor::init()
{
    project = 0;
    setChooserMode( FALSE );
}

void PixmapCollectionEditor::destroy()
{
}

void PixmapCollectionEditor::addPixmap()
{
    if ( !project )
	return;

    QString f;
    QStringList pixmaps = qChoosePixmaps( this );
    if ( pixmaps.isEmpty() )
	return;

    QString lastName;
    for ( QStringList::ConstIterator it = pixmaps.begin(); it != pixmaps.end(); ++it ) {
	QPixmap pm( *it );
	if ( pm.isNull() )
	    continue;
	PixmapCollection::Pixmap pixmap;
	pixmap.pix = pm;
	QFileInfo fi ( *it );
	pixmap.name = fi.fileName();
	pixmap.absname = fi.filePath();
	project->pixmapCollection()->addPixmap( pixmap );
	lastName = pixmap.name;
    }

    updateView();
    QIconViewItem *item = viewPixmaps->findItem( lastName );
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
	setCaption( tr( "Choose an image" ) );
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

