#include <qressource.h>
#include <qfile.h>

#include <qdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

/***************************************
 *
 * XML Parser
 *
 ***************************************/

bool qXMLReadConnection( const QRessource& res, QObject* obj )
{
  if ( !res.hasAttrib( "signal" ) || !res.hasAttrib( "sender" ) ||
       !res.hasAttrib( "slot" ) || !res.hasAttrib( "receiver" ) )
    return false;

  QString signal = res.textAttrib( "signal" );
  QString sender = res.textAttrib( "sender" );
  QString receiver = res.textAttrib( "receiver" );
  QString slot = res.textAttrib( "slot" );

  QObject *s = obj->child( sender );
  if ( !s && sender == obj->name() )
    s = obj;
  QObject *r = obj->child( receiver );
  if ( !r && receiver == obj->name() )
    r = obj;

  if ( !s )
  {
    debug("Object %s unknown", sender.ascii() );
    return false;
  }
  if ( !r )
  {
    debug("Object %s unknown", receiver.ascii() );
    return false;
  }

  signal.prepend("2");
  slot.prepend("1");
  QObject::connect( s, signal, r, slot );

  return true;
}

bool qXMLReadColor( const QString& _name, const QRessource& res, QColor* color )
{
  if ( res.hasAttrib( _name ) )
  {
    *color = res.colorAttrib( _name );
    return true;
  }
  
  return false;
}

bool qXMLReadFont( const QString& _name, const QRessource& res, QFont* font )
{
  QRessource ch = res.child( _name );
  if ( !ch.isValid() )
    return false;

  debug("Did find QFont tag named %s", _name.ascii() );
  if ( ch.hasAttrib( "family" ) )
    font->setFamily( ch.textAttrib( "family" ) );
  if ( ch.hasAttrib( "size" ) )
    font->setPointSize( ch.intAttrib( "size" ) );
  if ( ch.hasAttrib( "weight" ) )
  {
    QString w = ch.textAttrib( "weight" );
    if ( w == "light" )
      font->setWeight( QFont::Light );
    else if ( w == "normal" )
      font->setWeight( QFont::Normal );
    else if ( w == "demibold" )
      font->setWeight( QFont::DemiBold );
    else if ( w == "bold" )
      font->setWeight( QFont::Bold );
    else if ( w == "black" )
      font->setWeight( QFont::Black );
  }
  if ( ch.hasAttrib( "italic" ) && ch.textAttrib( "italic" ) == "true" )
    font->setItalic( TRUE );
  if ( ch.hasAttrib( "underline" ) && ch.textAttrib( "underline" ) == "true" )
    font->setUnderline( TRUE );

  return true;
}

bool qXMLReadString( const QString& _name, const QRessource& _res, QString* _str )
{
  QRessource ch = _res.child( _name );
  if ( !ch.isValid() )
    return false;

  *_str = "";
  QRessource t = ch.firstChild();
  for( ; t.isValid(); t = t.nextSibling() )
  {
    if ( !t.isText() )
      return false;
    *_str =+ t.text();
  }
  
  return true;
  /*
  if ( _res.hasAttrib( _name ) )
  {
    *_str = _res.textAttrib( _name );
    return true;
  }
  
  return false; */
}

bool qXMLReadBool( const QString& _name, const QRessource& _res, bool* _b )
{
  if ( _res.hasAttrib( _name ) )
  {
    *_b = _res.boolAttrib( _name );
    return true;
  }
  
  return false;
}

bool qXMLReadInt( const QString& _name, const QRessource& _res, int* _b )
{
  if ( _res.hasAttrib( _name ) )
  {
    *_b = _res.intAttrib( _name );
    return true;
  }
  
  return false;
}

bool qXMLReadDouble( const QString& _name, const QRessource& _res, double* _b )
{
  if ( _res.hasAttrib( _name ) )
  {
    *_b = _res.doubleAttrib( _name );
    return true;
  }
  
  return false;
}

// This one should be deleted
bool qXMLReadAlign( const QString& _name, const QRessource& _res, int* _align )
{
  if ( _res.hasAttrib( _name ) )
  {
    QString tmp = _res.textAttrib( _name );
    if ( tmp == "top" )
      *_align = Qt::AlignTop;
    else if ( tmp == "bottom" )
      *_align = Qt::AlignBottom;
    else if ( tmp == "center" )
      *_align = Qt::AlignCenter;
    else if ( tmp == "left" )
      *_align = Qt::AlignLeft;
    else if ( tmp == "right" )
      *_align = Qt::AlignRight;
    else
      return false;
    return true;
  }
  
  return false;
}

/***************************************
 *
 * QStdRessourceFactory
 *
 ***************************************/

QObject* qObjectFactory( const QRessource&, QObject* _parent )
{
  return new QObject( _parent );
}

bool qObjectConfig( const QRessource& res, QObject* _instance )
{
  if ( res.hasAttrib( "name" ) )
    _instance->setName( res.textAttrib( "name" ) );

  return true;
}

/*
QWidget* qWidgetFactory( const QRessource&, QWidget* _parent )
{
  return new QWidget( _parent );
}

bool qWidgetConfig( const QRessource& _res, QWidget* _instance )
{
  Q_RESSOURCE_ATTRIB( "bgcolor", QColor, _res, qXMLReadColor, _instance, setBackgroundColor );

  QRessource ch = _res.firstChild();
  for( ; ch.isValid(); ch = ch.nextSibling() )
  {
    if ( ch.type() == "Layout" )
    {
      QLayout* l = ch.incarnateLayout( _instance );
      if ( !l )
	return false;
    }
  }

  return true;
}

QWidget* qDialogFactory( const QRessource&, QWidget* _parent )
{
  return new QDialog( _parent );
}

bool qDialogConfig( const QRessource&, QWidget* )
{
  return true;
}

QWidget* qLabelFactory( const QRessource&, QWidget* _parent )
{
  return new QLabel( _parent );
}

bool qLabelConfig( const QRessource& res, QWidget* _instance )
{
  QLabel* l = (QLabel*)_instance;
  Q_RESSOURCE_ATTRIB( "text", QString, res, qXMLReadString, l, setText );

  return true;
}

QWidget* qPushButtonFactory( const QRessource&, QWidget* _parent )
{
  return new QPushButton( _parent );
}

bool qPushButtonConfig( const QRessource& _res, QWidget* _instance )
{
  QPushButton *b = (QPushButton*)_instance;
  Q_RESSOURCE_ATTRIB( "text", QString, _res, qXMLReadString, b, setText );

  return true;
}
*/

QLayout* qGridLayoutFactory( const QRessource& _res, QWidget* _parent )
{
  debug("NEW GridLayout\n");
  // Find out how many cells
  uint cols = 0;
  uint rows = 0;
  QRessource row = _res.firstChild();
  for( ; row.isValid(); row = row.nextSibling() )
  {
    if ( row.type() == "Row" )
    {
      uint c = 0;
      QRessource col = row.firstChild();
      while( col.isValid() )
      {
	if ( col.type() == "Cell" )
	  ++c;
	col = col.nextSibling();
      }
      if ( c > cols )
	cols = c;
      rows++;
    }
  }

  debug("Grid of size %i %i\n", rows, cols );

  int outsideBorder = 0;
  int insideSpacing = 0;
  if ( _res.hasAttrib( "border" ) )
    outsideBorder = _res.intAttrib( "border" );
  if ( _res.hasAttrib( "insidespacing" ) )
    insideSpacing = _res.intAttrib( "insidespacing" );

  return new QGridLayout( _parent, rows, cols, outsideBorder, insideSpacing );
}

bool qGridLayoutConfig( const QRessource& _res, QLayout* _instance )
{
  QGridLayout* grid = (QGridLayout*)_instance;

  QRessource irow = _res.firstChild();
  uint r = 0;
  for( ; irow.isValid(); irow = irow.nextSibling() )
  {
    if ( irow.type() == "Row" )
    {
      if ( irow.hasAttrib( "size" ) )
	grid->addRowSpacing( r, irow.intAttrib( "size" ) );
      if ( irow.hasAttrib( "stretch" ) )
	grid->setRowStretch( r, irow.intAttrib( "stretch" ) );

      QRessource icol = irow.firstChild();
      int c = 0;
      while( icol.isValid() )
      {
	if ( icol.type() == "Cell" )
	{
	  debug("QGridLayout child at %i %i", r, c );

	  int multicol = 1;
	  int multirow = 1;
	  if ( icol.hasAttrib( "multicol" ) )
	    multicol = icol.intAttrib( "multicol" );
	  if ( multicol < 1 )
	    return false;
	  if ( icol.hasAttrib( "multirow" ) )
	    multirow = icol.intAttrib( "multirow" );
	  if ( multirow < 1 )
	    return false;
	  int align = 0;
	  int x,y;
	  if ( qXMLReadAlign( "valign", icol, &y ) )
	    align |= y & ( Qt::AlignVCenter | Qt::AlignBottom | Qt::AlignTop );
	  if ( qXMLReadAlign( "halign", icol, &x ) )
	    align |= x & ~Qt::AlignVCenter;

	  QWidget* w = 0;
	  QRessource widget = icol.child( "Widget" );
	  if ( widget.isValid() )
	    w = widget.incarnateWidget( grid->mainWidget() );
	  else
	  {
	    QRessource layout = icol.child( "Layout" );
	    if ( layout.isValid() )
	    {
	      // #### Paul: Layout must be created, added, get child widgets
	      //            But I need: create, children, add
	      //            So I create intermediate widgets as a hack
	      w = new QWidget( grid->mainWidget() );
	      if ( !layout.incarnateLayout( w ) )
		return false;
	    }
	  }

	  // #### QGridLayout does not like empty cells
	  if ( !w )
	    w = new QWidget( grid->mainWidget() );

	  if ( w )
	  {
	    if ( multicol != 1 || multirow != 1 )
	      grid->addMultiCellWidget( w, r, r + multirow - 1, c, c + multicol - 1, align );
	    else
	      grid->addWidget( w, r, c, align );
	  }

	  if ( icol.hasAttrib( "size" ) )
	    grid->addColSpacing( c, icol.intAttrib( "size" ) );
	  if ( icol.hasAttrib( "stretch" ) )
	    {
	      debug("Setting stretch of col %i to %i",c,icol.intAttrib( "stretch" ) );
	    grid->setColStretch( c, icol.intAttrib( "stretch" ) );
	    }
	  
	  icol = icol.nextSibling();
	  ++c;
	}
	else
	  return false;
      }
      ++r;
    }
  }
  return true;
}

QLayout* qHBoxLayoutFactory( const QRessource& _res, QWidget* _parent )
{
  int outsideBorder = 0;
  int insideSpacing = 0;
  if ( _res.hasAttrib( "border" ) )
    outsideBorder = _res.intAttrib( "border" );
  if ( _res.hasAttrib( "insidespacing" ) )
    insideSpacing = _res.intAttrib( "insidespacing" );

  return new QHBoxLayout( _parent, outsideBorder, insideSpacing );
}

bool qHBoxLayoutConfig( const QRessource& _res, QLayout* _instance )
{
  QHBoxLayout* h = (QHBoxLayout*)_instance;

  QRessource widget = _res.firstChild();
  for( ; widget.isValid(); widget = widget.nextSibling() )
  {
    if ( widget.type() == "Widget" || widget.type() == "Layout" )
    {
      int stretch = 0;
      int align = 0;
      if ( widget.hasAttrib( "stretch" ) )
	stretch = widget.intAttrib( "stretch" );
      int x,y;
      if ( qXMLReadAlign( "valign", widget, &y ) )
	align |= y & ( Qt::AlignVCenter | Qt::AlignBottom | Qt::AlignTop );
      if ( qXMLReadAlign( "halign", widget, &x ) )
	align |= x & ~Qt::AlignVCenter;
      debug("Align=%i",align);
      
      if ( widget.type() == "Widget" )
      {
	QWidget* w = widget.incarnateWidget( h->mainWidget() );
	if ( !w )
	  return false;
	h->addWidget( w, stretch, align );
      }
      else // Layout
      {
	// #### Paul: Layout must be created, added, get child widgets
	//            But I need: create, children, add
	//            So I create intermediate widgets as a hack
	QWidget* w = new QWidget( h->mainWidget() );
	if ( !widget.incarnateLayout( w ) )
	  return false;
	h->addWidget( w, stretch, align );
      }
    }
    else if ( widget.type() == "Space" && widget.hasAttrib( "size" ) )
      h->addSpacing( widget.intAttrib( "size" ) );
    else if ( widget.type() == "Stretch" && widget.hasAttrib( "factor" ) )
      h->addStretch( widget.intAttrib( "factor" ) );
  }
  return true;
}

/***************************************
 *
 * QRessourceFactory
 *
 ***************************************/

QRessourceFactory* QRessourceFactory::self = 0;

/*
QRessourceFactory::QRessourceFactory()
{
  addFactory( "QWidget", qWidgetFactory, qWidgetConfig );
  addFactory( "QDialog", qDialogFactory, qDialogConfig );
  addFactory( "QLabel", qLabelFactory, qLabelConfig );
  addFactory( "QPushButton", qPushButtonFactory, qPushButtonConfig );
  addFactory( "QGridLayout", qGridLayoutFactory, qGridLayoutConfig );
  addFactory( "QHBoxLayout", qHBoxLayoutFactory, qHBoxLayoutConfig );
  addFactory( "QObject", qObjectFactory, qObjectConfig );
};
*/

QRessourceFactory::~QRessourceFactory()
{
}

QObject* QRessourceFactory::incarnateObject( const QRessource& _res, QObject* _parent ) const
{
  QMap<QString,ObjectFactory>::ConstIterator it = objectFactories.find( _res.className() );
  if ( it == objectFactories.end() )
  {
    debug("Could not find factory for class %s", _res.className().ascii() );
    return 0;
  }

  QObject* o = it.data().creator( _res, _parent );
  if ( !o )
    return 0;

  QStringList super = o->superClasses( true );
  ASSERT( super.count() > 0 );
  QStringList::Iterator sit = super.last(); 
  for( ; sit != super.end(); --sit )
  {
    QMap<QString,ObjectFactory>::ConstIterator oit = objectFactories.find( *sit );
    if ( oit != objectFactories.end() )
    {
      if ( !oit.data().configurator( _res, o ) )
      {
	delete o;
	return 0;
      }
    }
  }

  QRessource ch = _res.firstChild();
  for( ; ch.isValid(); ch = ch.nextSibling() )
  {
    if ( ch.type() == "Connect" )
      if ( !qXMLReadConnection( ch, o ) )
      {
	delete o;
	return 0;
      }
  }

  return o;
}

QWidget* QRessourceFactory::incarnateWidget( const QRessource& _res, QWidget* _parent ) const
{
  QMap<QString,WidgetFactory>::ConstIterator it = widgetFactories.find( _res.className() );
  if ( it == widgetFactories.end() )
  {
    debug("Could not find factory for class %s", _res.className().ascii() );
    return 0;
  }

  QWidget* w = it.data().creator( _res, _parent );
  if ( !w )
    return 0;

  QStringList super = w->superClasses( true );
  ASSERT( super.count() > 0 );
  QStringList::Iterator sit = super.last(); 
  for( ; sit != super.end(); --sit )
  {
    QMap<QString,WidgetFactory>::ConstIterator fit = widgetFactories.find( *sit );
    if ( fit != widgetFactories.end() )
    {
      if ( !fit.data().configurator( _res, w ) )
      {
	delete w;
	return 0;
      }
    }
    else
    {
      QMap<QString,ObjectFactory>::ConstIterator oit = objectFactories.find( *sit );
      if ( oit != objectFactories.end() )
      {
	if ( !oit.data().configurator( _res, w ) )
	{
	  delete w;
	  return 0;
	}
      }
    }
  }

  QRessource ch = _res.firstChild();
  for( ; ch.isValid(); ch = ch.nextSibling() )
  {
    if ( ch.type() == "Connect" )
      if ( !qXMLReadConnection( ch, w ) )
      {
	delete w;
	return 0;
      }
  }

  return w;
}

QLayout* QRessourceFactory::incarnateLayout( const QRessource& _res, QWidget* _parent ) const
{
  QMap<QString,LayoutFactory>::ConstIterator it = layoutFactories.find( _res.className() );
  if ( it == layoutFactories.end() )
  {
    debug("Could not find factory for class %s", _res.className().ascii() );
    return 0;
  }

  QLayout *l = it.data().creator( _res, _parent );
  if ( !l )
    return 0;

  QStringList super = l->superClasses( true );
  ASSERT( super.count() > 0 );
  QStringList::Iterator sit = super.last(); 
  for( ; sit != super.end(); --sit )
  {
    QMap<QString,LayoutFactory>::ConstIterator fit = layoutFactories.find( *sit );
    if ( fit != layoutFactories.end() )
    {
      if ( !fit.data().configurator( _res, l ) )
      {
	delete l;
	return 0;
      }
    }
    else
    {
      QMap<QString,ObjectFactory>::ConstIterator oit = objectFactories.find( *sit );
      if ( oit != objectFactories.end() )
      {
	if ( !oit.data().configurator( _res, l ) )
	{
	  delete l;
	  return 0;
	}
      }
    }
  }

  return l;
}

QWidget* QRessourceFactory::createWidget( const QString& _class, QWidget* _parent ) const
{
  QMap<QString,WidgetFactory>::ConstIterator it = widgetFactories.find( _class );
  if ( it == widgetFactories.end() )
  {
    debug("Could not find factory for class %s", _class.ascii() );
    return 0;
  }

  QRessource res;
  return it.data().creator( res, _parent );
}

QObject* QRessourceFactory::createObject( const QString& _class, QObject* _parent ) const
{
  QMap<QString,ObjectFactory>::ConstIterator it = objectFactories.find( _class );
  if ( it == objectFactories.end() )
  {
    debug("Could not find factory for class %s", _class.ascii() );
    return 0;
  }

  QRessource res;
  return it.data().creator( res, _parent );
}

QLayout* QRessourceFactory::createLayout( const QString& _class, QWidget* _parent ) const
{
  QMap<QString,LayoutFactory>::ConstIterator it = layoutFactories.find( _class );
  if ( it == layoutFactories.end() )
  {
    debug("Could not find factory for class %s", _class.ascii() );
    return 0;
  }

  QRessource res;
  return it.data().creator( res, _parent );
}

bool QRessourceFactory::supportsType( const QString& _type ) const
{
  if ( widgetFactories.contains( _type ) )
    return true;
  return layoutFactories.contains( _type );
}

QRessourceFactory* QRessourceFactory::factory()
{
  if ( self == 0 )
    self = new QRessourceFactory;

  return self;
}

void QRessourceFactory::addFactory( const QString& _type, WidgetCreator _create, WidgetConfigurator _cfg)
{
  widgetFactories.insert( _type, WidgetFactory( _create, _cfg ) );
}

void QRessourceFactory::addFactory( const QString& _type, LayoutCreator _create, LayoutConfigurator _cfg )
{
  layoutFactories.insert( _type, LayoutFactory( _create, _cfg ) );
}

void QRessourceFactory::addFactory( const QString& _type, ObjectCreator _create, ObjectConfigurator _cfg )
{
  objectFactories.insert( _type, ObjectFactory( _create, _cfg ) );
}

/***************************************
 *
 * QRessource
 *
 ***************************************/

QRessource::QRessource() : it( 0 )
{
  sh = new QXMLParseTree;
}

QRessource::QRessource( const QRessource& _r, QXMLIterator _it ) : it( _it )
{
  sh = _r.sh;
  sh->ref();
}

QRessource::QRessource( const QRessource& _res ) : it( _res.it )
{
  sh = _res.sh;
  sh->ref();
}

QRessource::QRessource( const QString& _filename ) : it( 0 )
{
  sh = new QXMLParseTree;

  QFile file( _filename );
  if ( !file.open( IO_ReadOnly ) )
    return;

  uint size = file.size();
  char* buffer = new char[ size + 1 ];
  file.readBlock( buffer, size );
  file.close();
  buffer[ size ] = 0;
  QString text( buffer, size + 1 );
  delete[] buffer;

  setContent( text );
}

bool QRessource::setContent( const QString& _text )
{
  QXMLParser parser;
  int result = parser.parse( _text, sh );
  if ( result != -1 )
  {
    it = QXMLIterator( 0 );
    debug("@@%s@@\n",_text.ascii());
    debug("QRessource: Error parsing XML at position %i\n", result );
    return false;
  }

  it = QXMLIterator( sh->root()->firstChild );
  return true;
}

QRessource::~QRessource()
{
  if ( sh->deref() ) delete sh;
}

QObject* QRessource::incarnateObject( QObject* _parent ) const
{
  if ( !it.isValid() )
    return 0;

  return QRessourceFactory::factory()->incarnateObject( *this, _parent );
}

QWidget* QRessource::incarnateWidget( QWidget* _parent ) const
{
  if ( !it.isValid() )
    return 0;

  return QRessourceFactory::factory()->incarnateWidget( *this, _parent );
}

QLayout* QRessource::incarnateLayout( QWidget* _parent ) const
{
  if ( !it.isValid() )
    return 0;

  return QRessourceFactory::factory()->incarnateLayout( *this, _parent );
}

QPixmap QRessource::incarnatePixmap() const
{
  // Not implemented yet
  return QPixmap();
}

QRessource QRessource::ressource( const QString& _name ) const
{
  QXMLIterator j = findRessource( _name );
  return QRessource( *this, j );
}

bool QRessource::isValid() const
{
  return it.isValid();
}

QXMLIterator QRessource::findRessource( const QString& _name ) const
{
  if ( !it.isValid() )
    return it;
  
  if ( it.hasAttrib( "name" ) && it.textAttrib( "name" ) == _name )
    return it;

  // First look at the direct children before we do preorder
  // searching on the entire tree to speed thing up in 90%
  QXMLIterator j = it.firstChild();

  while( j.isValid() )
  {
    if ( j.hasAttrib( "name" ) && j.textAttrib( "name" ) == _name )
      return j;
    j++;
  }

  j = it.firstChild();
  while( j.isValid() )
  {
    QXMLIterator ret = findRessource( j.firstChild(), _name );
    if ( ret.isValid() )
      return ret;
    j++;
  }

  return j;
}

QXMLIterator QRessource::findRessource( QXMLIterator j, const QString& _name ) const
{
  if ( !j.isValid() )
    return j;

  if ( j.hasAttrib( "name" ) && j.textAttrib( "name" ) == _name )
    return j;

  // Test the children
  j = j.firstChild();
  while( j.isValid() )
  {
    QXMLIterator ret = findRessource( j.firstChild(), _name );
    if ( ret.isValid() )
      return ret;
    j++;
  }

  return j;
}

QXMLIterator QRessource::root() const
{
  return it;
}

QXMLParseTree* QRessource::parseTree()
{
  return sh;
}

bool QRessource::hasParent() const
{
  return ( it.parent().isValid() );
}

bool QRessource::hasChildren() const
{
  return ( it.firstChild().isValid() );
}

bool QRessource::hasSibling() const
{
  return ( it.nextSibling().isValid() );
}

QRessource QRessource::firstChild() const
{
  return QRessource( *this, it.firstChild() );
}

QRessource QRessource::nextSibling() const
{
  return QRessource( *this, it.nextSibling() );
}

QRessource QRessource::parent() const
{
  return QRessource( *this, it.parent() );
}

QString QRessource::name() const
{
  if ( it.isValid() && it.hasAttrib( "name" ) )
    return it.textAttrib( "name" );
  return QString();
}

QString QRessource::type() const
{
  if ( !it.isValid() || it.isText() )
    return QString();
  return it->name;
}

QString QRessource::className() const
{
  if ( it.isValid() && it.hasAttrib( "class" ) )
    return it.textAttrib( "class" );
  return QString();
}

QRessource QRessource::child( const QString& _type ) const
{
  QXMLIterator j = it.firstChild();
  while( j.isValid() )
  {
    if ( j->name == _type )
      return QRessource( *this, j );
    ++j;
  }

  return QRessource( *this, j );
}
