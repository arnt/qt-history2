/*! \fn void QListView::selectionChanged()

    This is a Qt signal.
*/

/*! \fn QListViewItem *QListView::currentItem() const

    \target 1
    This is a Qt plain function.

    \target 2
    A few constructs: QApplication::setEnabled(TRUE); qApp->filter().
    QCString.

  \target 3
  \code
    QString boolToString( bool b )
    {
	QString result;

	if ( b )
	    result = "True";
	else
	    result = "False";
	return result;
    }
  \endcode

  \target 4
  \code
    QString func( const QString& input )
    {
	QString output = input;
	// process output
	return output;
    }
  \endcode

  \target 5
  \code
    QString b( a.unicode(), b.length() );
    QString a("abcd");
  \endcode

  \target 6
  \code
    QString a;          // a.unicode() == 0, a.length() == 0
    a.isNull();         // TRUE, because a.unicode() == 0
    a.isEmpty();        // TRUE
  \endcode

  \target 7
  \code
    QString s = "truncate me";
    s.truncate( 5 );            // s == "trunc"
  \endcode

    \code
    QLabel *splashScreen = new QLabel( 0, "mySplashScreen",
				WStyle_Customize | WStyle_NoBorder |
				WStyle_Tool );
    \endcode

    \code
	width = baseSize().width() + i * sizeIncrement().width();
	height = baseSize().height() + j * sizeIncrement().height();
    \endcode

    \code
	aWidget->topLevelWidget()->setCaption( "New Caption" );
    \endcode

    \code
    setBackgroundMode( PaletteBase );
    \endcode

    \code
    QFont f( "Helvetica", 12, QFont::Bold );
    setFont( f );
    \endcode

    \code
	setCursor( IbeamCursor );
    \endcode

    \code
	setTabOrder( a, b ); // a to b
	setTabOrder( b, c ); // a to b to c
	setTabOrder( c, d ); // a to b to c to d
    \endcode

    \code
	setUpdatesEnabled( FALSE );
	bigVisualChanges();
	setUpdatesEnabled( TRUE );
	repaint();
    \endcode

    \code
	repaint( w->visibleRect(), FALSE );
    \endcode

    \code
	if ( autoMask() )
	    updateMask();
    \endcode

    \code
    QSqlQuery query( "SELECT name FROM customer" );
    while ( query.next() ) {
	QString name = query.value(0).toString();
	doSomething( name );
    }
    \endcode

    \code
    // Binding values using named placeholders (e.g. in Oracle)
    QSqlQuery q;
    q.prepare( "insert into mytable (id, name, lastname) values (:id, :name, :lname)" );
    q.bindValue( ":id", 0 );
    q.bindValue( ":name", "Testname" );
    q.bindValue( ":lname", "Lastname" );
    q.exec();

    // Binding values using positional placeholders (e.g. in ODBC)
    QSqlQuery q;
    q.prepare( "insert into mytable (id, name, lastname) values (?, ?, ?)" );
    q.bindValue( 0, 0 );
    q.bindValue( 1, "Testname" );
    q.bindValue( 2, "Lastname" );
    q.exec();
    
    // or alternatively
    q.prepare( "insert into mytable (id, name, lastname) values (?, ?, ?)" );
    q.addBindValue( 0 );
    q.addBindValue( "Testname" );
    q.addBindValue( "Lastname" );
    q.exec();
    
    // and then for repeated runs:
    q.bindValue( 0, 1 );
    q.bindValue( 1, "John" );
    q.bindValue( 2, "Doe" );    
    q.exec();
    \endcode
*/

/*! \fn QListViewItem *QListView::lastItem() const

    This is a Qt plain function. This documentation should not
    be overridden.
*/


/*NOT DOCUMENTED! \fn QListViewItem *QuickListViewInterface::currentItem() const

    This is a Quick slot for a documented Qt function.
*/
