/*
  classhierarchy.ui.h
*/

#include <qfiledialog.h>
#include <qregexp.h>

void ClassHierarchy::addSearchPath()
{
    QString path = QFileDialog::getExistingDirectory(
	    QDir::homeDirPath(), this, 0, "Select a Directory" );
    if ( !path.isEmpty() && searchPathBox->findItem(path, ExactMatch) == 0 )
	searchPathBox->insertItem( path );
}

void ClassHierarchy::removeSearchPath()
{
    searchPathBox->removeItem( searchPathBox->currentItem() );
}

void ClassHierarchy::updateHierarchy()
{
    QString fileNameFilter;
    QRegExp classDef;

    if ( language->currentText() == "C++" ) {
	fileNameFilter = "*.h";
	classDef.setPattern( "\\bclass\\s+([A-Z_a-z0-9]+)\\s*"
			     "(?:\\{|:\\s*public\\s+([A-Z_a-z0-9]+))" );
    } else if ( language->currentText() == "Java" ) {
	fileNameFilter = "*.java";
	classDef.setPattern( "\\bclass\\s+([A-Z_a-z0-9]+)\\s+extends\\s*"
			     "([A-Z_a-z0-9]+)" );
    }

    dict.clear();
    listView->clear();

    for ( int i = 0; i < searchPathBox->count(); i++ ) {
	QDir dir = searchPathBox->text( i );
	QStringList names = dir.entryList( fileNameFilter );

	for ( int j = 0; j < names.count(); j++ ) {
	    QFile file( dir.filePath(names[j]) );
	    if ( file.open(IO_ReadOnly) ) {
		QString content = file.readAll();
		int k = 0;
		while ( (k = classDef.search(content, k)) != -1 ) {
		    processClassDef( classDef.cap(1), classDef.cap(2), names[j] );
		    k++;
		}
	    }
	}
    }
}

void ClassHierarchy::processClassDef( const QString& derived,
	const QString& base, const QString& sourceFile )
{
    QListViewItem *derivedItem = insertClass( derived, sourceFile );

    if ( !base.isEmpty() ) {
	QListViewItem *baseItem = insertClass( base, "" );
	if ( derivedItem->parent() == 0 ) {
	    listView->takeItem( derivedItem );
	    baseItem->insertItem( derivedItem );
	    derivedItem->setText( 1, sourceFile );
	}
    }
}

QListViewItem *ClassHierarchy::insertClass( const QString& name,
					    const QString& sourceFile )
{
    if ( dict[name] == 0 ) {
	QListViewItem *item = new QListViewItem( listView, name, sourceFile );
	item->setOpen( TRUE );
	dict.insert( name, item );
    }
    return dict[name];
}
