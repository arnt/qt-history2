/*
  classhierarchy.ui.h
*/

#include <qfiledialog.h>
#include <qregexp.h>

void ClassHierarchy::addSearchPath()
{
    QString path = QFileDialog::getExistingDirectory( QDir::homeDirPath(),
						      this, 0,
						      "Select a Directory" );
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
    QString classDef;

    if ( languageCombo->currentText() == "C++" ) {
	fileNameFilter = "*.h";
	classDef = "\\bclass\\s+([A-Z_a-z0-9]+)\\s*"
		   "(?:\\{|:\\s*public\\s+([A-Z_a-z0-9]+))";
    } else if ( languageCombo->currentText() == "Java" ) {
	fileNameFilter = "*.java";
	classDef = "\\bclass\\s+([A-Z_a-z0-9]+)\\s+extends"
		   "\\s+([A-Z_a-z0-9]+)";
    } else if ( languageCombo->currentText() == "Python" ) {
	fileNameFilter = "*.py";
	classDef = "\\bclass\\s+([A-Z_a-z0-9]+)"
		   "\\s*\\(\\s*([A-Z_a-z0-9]+)\\s*\\)\\s*:";
    }
    QRegExp classDefRegExp( classDef );

    for ( int i = 0; i < searchPathBox->count(); i++ ) {
	QDir dir = searchPathBox->text( i );
	dir.setNameFilter( fileNameFilter );
	QStringList names = dir.entryList();

	for ( int j = 0; j < names.count(); j++ ) {
	    QFile file( dir.filePath(names[j]) );
	    if ( file.open(IO_ReadOnly) ) {
		QString content = file.readAll();
		int k = 0;
		while ( (k = classDefRegExp.search(content, k)) != -1 ) {
		    QString derivedClass = classDefRegExp.cap( 1 );
		    QString baseClass = classDefRegExp.cap( 2 );
		    baseClassMap[derivedClass] = baseClass;
		    sourceFileMap[derivedClass] = names[j];
		    k += classDefRegExp.matchedLength();
		}
	    }
	}
    }

    QMap<QString, QStringList> derivedClassMap;
    QMap<QString, QString>::ConstIterator b = baseClassMap.begin();
    while ( b != baseClassMap.end() ) {
	if ( *b == "" || baseClassMap.contains(*b) )
	    derivedClassMap[*b].push_back( b.key() );
	++b;
    }

    hierarchyView->clear();
    populateLevel( derivedClassMap, "", 0 );
}

void ClassHierarchy::populateLevel(
	const QMap<QString, QStringList>& derivedClassMap,
	const QString& baseClass, QListViewItem *parentItem ) const
{
    QStringList derivedClasses = derivedClassMap[baseClass];
    for ( int i = 0; i < derivedClasses.count(); i++ ) {
	QListViewItem *item;
	if ( parentItem == 0 ) {
	    item = new QListViewItem( hierarchyView, derivedClasses[i], "foo.h" );
	} else {
	    item = new QListViewItem( parentItem, derivedClasses[i], "foo.h" );
	}
	item->setOpen( TRUE );
	populateLevel( derivedClassMap, derivedClasses[i], item );
    }
}
