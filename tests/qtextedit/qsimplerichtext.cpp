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
    d->doc = new QTextDocument;
    d->doc->setFormatter( new QTextFormatterBreakWords( d->doc ) );
    d->doc->setDefaultFont( fnt );
    d->doc->setText( text );
}

QSimpleRichText::QSimpleRichText( const QString& text, const QFont& fnt,
				  const QString& context,  const QStyleSheet* sheet,
				  const QMimeSourceFactory* factory, int verticalBreak,
				  const QColor& linkColor, bool linkUnderline )
{
    d = new QSimpleRichTextData;
    d->doc = new QTextDocument;
    d->doc->setFormatter( new QTextFormatterBreakWords( d->doc ) );
    d->doc->setDefaultFont( fnt );
    d->doc->flow()->pagesize = verticalBreak;
    d->doc->setVerticalBreak( TRUE );
    d->doc->setText( text );
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
}

void QSimpleRichText::draw( QPainter *p,  int x, int y, const QRegion& clipRegion,
			    const QColorGroup& cg, const QBrush* paper ) const
{
    p->translate( x, y );
    d->doc->draw( p, clipRegion, cg );
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
    return QString::null;
}

QString QSimpleRichText::anchor( QPainter* p, const QPoint& pos )
{
    ASSERT( 0 );
    return QString::null;
}

bool QSimpleRichText::inText( const QPoint& pos ) const
{
    ASSERT( 0 );
    return FALSE;
}
