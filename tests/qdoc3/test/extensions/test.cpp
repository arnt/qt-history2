/*! \fn void QListView::selectionChanged()

    This is a Qt signal.
*/

/*! \fn QListViewItem *QListView::currentItem() const

    This is a Qt plain function.

    A few constructs: QApplication::setEnabled(TRUE); qApp->filter().
    QCString.

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

  \code
    QString func( const QString& input )
    {
	QString output = input;
	// process output
	return output;
    }
  \endcode

  \code
    QString a("abcd");
    QString b( a.unicode(), b.length() );
  \endcode

  \code
    QString a;          // a.unicode() == 0, a.length() == 0
    a.isNull();         // TRUE, because a.unicode() == 0
    a.isEmpty();        // TRUE
  \endcode

  \code
    QString s = "truncate me";
    s.truncate( 5 );            // s == "trunc"
  \endcode
*/

/*! \fn QListViewItem *QListView::lastItem() const

    This is a Qt plain function. This documentation should not
    be overridden.
*/


/*NOT DOCUMENTED! \fn QListViewItem *QuickListViewInterface::currentItem() const

    This is a Quick slot for a documented Qt function.
*/
