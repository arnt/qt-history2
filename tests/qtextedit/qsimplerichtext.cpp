#include "qsimplerichtext.h"
#include "qrichtext_p.h"

class QSimpleRichTextData
{
public:
    QTextDocument *doc;
};

QSimpleRichText::QSimpleRichText( const QString& text, const QFont& fnt,
				  const QString& context, const QStyleSheet* sheet )
{
    d = new QSimpleRichTextData;
    d->doc = new QTextDocument( 0 );
    d->doc->setFormatter( new QTextFormatterBreakWords( d->doc ) );
    d->doc->setDefaultFont( fnt );
    d->doc->setStyleSheet( sheet );
    d->doc->setText( text, context );
}

QSimpleRichText::QSimpleRichText( const QString& text, const QFont& fnt,
				  const QString& context,  const QStyleSheet* sheet,
				  const QMimeSourceFactory* factory, int verticalBreak,
				  const QColor& linkColor, bool linkUnderline )
{
    d = new QSimpleRichTextData;
    d->doc = new QTextDocument( 0 );
    d->doc->setFormatter( new QTextFormatterBreakWords( d->doc ) );
    d->doc->setDefaultFont( fnt );
    d->doc->flow()->pagesize = verticalBreak;
    d->doc->setVerticalBreak( TRUE );
    d->doc->setStyleSheet( sheet );
    d->doc->setMimeSourceFactory( factory );
    d->doc->setLinkColor( linkColor );
    d->doc->setUnderlineLinks( linkUnderline );
    d->doc->setText( text, context );
}

QSimpleRichText::~QSimpleRichText()
{
}

void QSimpleRichText::setWidth( int w )
{
    d->doc->doLayout( 0, w );
}

void QSimpleRichText::setWidth( QPainter *p, int w )
{
    d->doc->doLayout( p, w );
}

int QSimpleRichText::width() const
{
    return d->doc->width();
}

int QSimpleRichText::widthUsed() const
{
    ASSERT( 0 );
    return 0;
}

int QSimpleRichText::height() const
{
    return d->doc->height();
}

void QSimpleRichText::adjustSize()
{
    ASSERT( 0 );
}

void QSimpleRichText::draw( QPainter *p,  int x, int y, const QRegion& clipRegion,
			    const QPalette& pal, const QBrush* paper ) const
{
    ASSERT( 0 );

    Q_UNUSED( p ); // #### use them
    Q_UNUSED( x );
    Q_UNUSED( y );
    Q_CONST_UNUSED( clipRegion );
    Q_CONST_UNUSED( pal );
    Q_UNUSED( paper );
}

void QSimpleRichText::draw( QPainter *p,  int x, int y, const QRegion& clipRegion,
			    const QColorGroup& cg, const QBrush* paper ) const
{
    if ( paper )
	d->doc->setPaper( paper );
    QColorGroup g = cg;
    if ( d->doc->paper() )
	g.setBrush( QColorGroup::Base, *d->doc->paper() );

    p->translate( x, y );
    d->doc->draw( p, clipRegion, g, paper );
    p->translate( -x, -y );
}

QString QSimpleRichText::context() const
{
    ASSERT( 0 );
    return QString::null;
}

QString QSimpleRichText::anchorAt( const QPoint& pos ) const
{
    ASSERT( 0 );

    Q_CONST_UNUSED( pos ); // ### use iz

    return QString::null;
}

QString QSimpleRichText::anchor( QPainter* p, const QPoint& pos )
{
    ASSERT( 0 );

    Q_UNUSED( p ); // ### use them
    Q_CONST_UNUSED( pos );

    return QString::null;
}

bool QSimpleRichText::inText( const QPoint& pos ) const
{
    ASSERT( 0 );

    Q_CONST_UNUSED( pos ); // #### use it

    return FALSE;
}
