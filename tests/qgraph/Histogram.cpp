// Histogram.cpp : Implementation of CHistogram
#include "stdafx.h"
#include "QGraph.h"
#include "Histogram.h"
#include <qpixmap.h>
#include <qpainter.h>
#include <qapplication.h>
#include "ocidl.h"	// Added by ClassView

/*
** Helper functions to convert to and from OLE datatypes
*/
static QColor OLE2QColor( OLE_COLOR oColor )
{
    COLORREF cRef;

    OleTranslateColor( oColor, NULL, &cRef );

    return QColor( GetRValue( cRef ), GetGValue( cRef ), GetBValue( cRef ) );
}

static OLE_COLOR QColor2OLE( QColor col )
{
    return RGB( col.red(), col.green(), col.blue() );
}

static QString BSTR2QString( BSTR bstr )
{
    QString str;

    for( int i = 0; i < SysStringLen( bstr ); i++ )
	str += QChar( bstr[ i ] );
//    SysFreeString( bstr );

    return str;
}

static BSTR QString2BSTR( QString str )
{
    BSTR bstr = SysAllocStringLen( NULL, str.length() );

    if( bstr ) {
	for( int i = 0; i < str.length(); i++ ) {
	    const QChar c = str[ i ];
	    bstr[ i ] = c.unicode();
	}
    }

    return bstr;
}

static BSTR QByteArray2BSTR( QByteArray a )
{
    return SysAllocStringLen( (OLECHAR*)a.data(), a.size() / 2 + 1 );
}

/////////////////////////////////////////////////////////////////////////////
// CHistogram

STDMETHODIMP CHistogram::get_NumBars(long *pVal)
{
    if( pVal )
	*pVal = m_numBars;
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::put_NumBars(long newVal)
{
    if( newVal > 0 ) {
	m_numBars = newVal;
	m_barNames.resize( m_numBars );
	m_values.resize( m_numBars * m_numSegments );
    }
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::get_NumSegments(long *pVal)
{
    if( pVal )
	*pVal = m_numSegments;
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::put_NumSegments(long newVal)
{
    if( newVal > 0 ) {
	m_numSegments = newVal;
	m_segmentNames.resize( m_numSegments );
	m_values.resize( m_numBars * m_numSegments );
    }
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::get_MaxValue(double *pVal)
{
    if( pVal )
	*pVal = m_maxValue;
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::put_MaxValue(double newVal)
{
    if( newVal > 0 )
	m_maxValue = newVal;
    else {
	return E_FAIL;
    }
    return S_OK;
}

STDMETHODIMP CHistogram::get_SegmentColor(long SegmentNum, OLE_COLOR *pVal)
{
    if( pVal && ( SegmentNum < m_numSegments ) )
	*pVal = QColor2OLE( m_segmentColors[ SegmentNum ] );
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::put_SegmentColor(long SegmentNum, OLE_COLOR newVal)
{
    if( SegmentNum < m_numSegments )
	m_segmentColors[ SegmentNum ] = OLE2QColor( newVal );
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::get_Value(long BarNumber, long SegmentNumber, double *pVal)
{
    if( pVal && ( BarNumber < m_numBars ) && ( SegmentNumber < m_numSegments ) )
	*pVal = m_values[ BarNumber * m_numSegments + SegmentNumber ];
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::put_Value(long BarNumber, long SegmentNumber, double newVal)
{
    if( ( BarNumber < m_numBars ) && ( SegmentNumber < m_numSegments ) )
	m_values[ BarNumber * m_numSegments + SegmentNumber ] = newVal;
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::get_Width(long *pVal)
{
    if( pVal )
	*pVal = m_width;
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::put_Width(long newVal)
{
    if( newVal > 0 )
	m_width = newVal;
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::get_Height(long *pVal)
{
    if( pVal ) {
	int height = m_numBars * m_barWidth +
		    ( m_barWidth * ( m_numSegments + 1 ) * m_showExplanation ) +
		    ( m_barWidth * m_showScale ) +
		    ( 32 * m_showGraphTitle );
	*pVal = height;
    }
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::get_BarWidth(long *pVal)
{
    if( pVal )
	*pVal = m_barWidth;
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::put_BarWidth(long newVal)
{
    if( newVal > 0 )
	m_barWidth = newVal;
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::get_showExplanation(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showExplanation;
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::put_showExplanation(BOOL newVal)
{
    m_showExplanation = newVal;

    return S_OK;
}

STDMETHODIMP CHistogram::get_SegmentName(long SegmentNumber, BSTR *pVal)
{
    if( pVal && ( SegmentNumber < m_numSegments ) )
	*pVal = QString2BSTR( m_segmentNames[ SegmentNumber ] );
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::put_SegmentName(long SegmentNumber, BSTR newVal)
{
    if( SegmentNumber < m_numSegments ) {
	m_segmentNames[ SegmentNumber ] = BSTR2QString( newVal );
    }
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::get_BarName(long BarNumber, BSTR *pVal)
{
    if( pVal && ( BarNumber < m_numBars ) )
	*pVal = QString2BSTR( m_barNames[ BarNumber ] );
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::put_BarName(long BarNumber, BSTR newVal)
{
    if( BarNumber < m_numBars ) {
	m_barNames[ BarNumber ] = BSTR2QString( newVal );
    }
    else {
	return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CHistogram::get_showBarNames(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showBarNames;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_showBarNames(BOOL newVal)
{
    m_showBarNames = newVal;

    return S_OK;
}

STDMETHODIMP CHistogram::get_BackgroundColor(OLE_COLOR *pVal)
{
    if( pVal )
	*pVal = QColor2OLE( m_backgroundColor );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_BackgroundColor(OLE_COLOR newVal)
{
    m_backgroundColor = OLE2QColor( newVal );

    return S_OK;
}

STDMETHODIMP CHistogram::get_showHorizontalGrid(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showHorizontalGrid;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_showHorizontalGrid(BOOL newVal)
{
    m_showHorizontalGrid = newVal;

    return S_OK;
}

STDMETHODIMP CHistogram::get_GridColor(OLE_COLOR *pVal)
{
    if( pVal )
	*pVal = QColor2OLE( m_gridColor );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_GridColor(OLE_COLOR newVal)
{
    m_gridColor = OLE2QColor( newVal );

    return S_OK;
}

STDMETHODIMP CHistogram::get_showVerticalGrid(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showVerticalGrid;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_showVerticalGrid(BOOL newVal)
{
    m_showVerticalGrid = newVal;

    return S_OK;
}

STDMETHODIMP CHistogram::get_ShowValues(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showValues;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_ShowValues(BOOL newVal)
{
    m_showValues = newVal;

    return S_OK;
}

STDMETHODIMP CHistogram::get_TextColor(OLE_COLOR *pVal)
{
    if( pVal )
	*pVal = QColor2OLE( m_textColor );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_TextColor(OLE_COLOR newVal)
{
    m_textColor = OLE2QColor( newVal );

    return S_OK;
}

STDMETHODIMP CHistogram::get_MimeType(BSTR *pVal)
{
    if( pVal )
	*pVal = QString2BSTR( "image/png" );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::get_GridSpacing(double *pVal)
{
    if( pVal )
	*pVal = m_gridSpacing;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_GridSpacing(double newVal)
{
    if( newVal )
	m_gridSpacing = newVal;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::get_showScale(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showScale;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_showScale(BOOL newVal)
{
    m_showScale = newVal;

    return S_OK;
}

STDMETHODIMP CHistogram::get_totalForBar(long BarNumber, double *pVal)
{
    if( pVal ) {
	*pVal = 0.0;
	for( int s = 0; s < m_numSegments; s++ )
	*pVal += m_values[ BarNumber * m_numSegments + s ];
    }
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::get_ScaleFormat(BSTR *pVal)
{
    if( pVal )
	*pVal = QString2BSTR( m_scaleFormat );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_ScaleFormat(BSTR newVal)
{
    m_scaleFormat = BSTR2QString( newVal );

    return S_OK;
}

STDMETHODIMP CHistogram::get_GraphTitle(BSTR *pVal)
{
    if( pVal )
	*pVal = QString2BSTR( m_graphTitle );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_GraphTitle(BSTR newVal)
{
    m_graphTitle = BSTR2QString( newVal );

    return S_OK;
}

STDMETHODIMP CHistogram::get_showGraphTitle(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showGraphTitle;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_showGraphTitle(BOOL newVal)
{
    m_showGraphTitle = newVal;

    return S_OK;
}

STDMETHODIMP CHistogram::get_showTotal(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showTotal;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_showTotal(BOOL newVal)
{
    m_showTotal = newVal;

    return S_OK;
}

STDMETHODIMP CHistogram::get_WorksheetColor(OLE_COLOR *pVal)
{
    if( pVal )
	*pVal = QColor2OLE( m_worksheetColor );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_WorksheetColor(OLE_COLOR newVal)
{
    m_worksheetColor = OLE2QColor( newVal );

    return S_OK;
}


STDMETHODIMP CHistogram::get_DisplayMode(long *pVal)
{
    if( pVal )
	*pVal = m_displayMode;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CHistogram::put_DisplayMode(long newVal)
{
    m_displayMode = newVal;

    return S_OK;
}
/*
** Render the graph
*/
extern Q_EXPORT QApplication* qApp;

STDMETHODIMP CHistogram::get_Picture(BSTR *pVal)
{
    if( !pVal ) {
	return E_FAIL;
    }

    long height;

    get_Height( &height );
    QPixmap px( m_width, height );
    QPainter* p = new QPainter( &px );
    QRect graphRect( 0, 0, m_width, m_numBars * m_barWidth );

    px.fill( m_backgroundColor );

    if( m_showGraphTitle ) {
	graphRect.setTop( graphRect.top() + 32 );
	graphRect.setBottom( graphRect.bottom() + 32 );
    }

    if( m_showBarNames ) {
	int maxWidth( 0 );
	p->setPen( m_textColor );
	for( int i = 0; i < m_numBars; i++ ) {
	    QRect r = p->boundingRect( graphRect.left(), graphRect.top() + i * m_barWidth, graphRect.width(), m_barWidth, Qt::AlignAuto | Qt::SingleLine, m_barNames[ i ] );
	    if( r.width() > maxWidth )
		maxWidth = r.width();
	    p->drawText( graphRect.left(), graphRect.top() + i * m_barWidth, graphRect.width(), m_barWidth, Qt::AlignAuto | Qt::AlignVCenter | Qt::SingleLine, m_barNames[ i ] );
	}
	graphRect.setLeft( graphRect.left() + maxWidth + 5 );
    }
    if( m_showTotal ) {
	p->setPen( m_textColor );
	int maxWidth( 0 );
	for( int b = 0; b < m_numBars; b++ ) {
	    double total;
	    get_totalForBar( b, &total );
	    QString totalStr;
	    switch( m_displayMode ) {
	    case 0:
		totalStr = m_scaleFormat.arg( total, 0, 'f', 0 );
		break;
	    case 1:
		totalStr= QString( "%1" ).arg( total, 0, 'f', 0 );
		break;
	    }
	    QRect r = p->boundingRect( graphRect.left(), graphRect.top() + b * m_barWidth, graphRect.width(), m_barWidth, Qt::AlignAuto | Qt::SingleLine, totalStr );
	    if( r.width() > maxWidth )
		maxWidth = r.width();
	    p->drawText( graphRect.left(), graphRect.top() + b * m_barWidth, graphRect.width(), m_barWidth, Qt::AlignAuto | Qt::AlignVCenter | Qt::SingleLine, totalStr );
	}
	graphRect.setLeft( graphRect.left() + maxWidth + 5 );
    }
    if( m_showScale ) {
	p->setPen( m_textColor );
	
	double scaleVal( m_gridSpacing );
	while( scaleVal < m_maxValue ) {
	    int scalePos = graphRect.left() + scaleVal * graphRect.width() / m_maxValue;
	    p->drawText( scalePos - ( m_gridSpacing * graphRect.width() / m_maxValue ) / 2,
			graphRect.bottom(),
			( m_gridSpacing * graphRect.width() / m_maxValue ),
			m_barWidth, Qt::AlignCenter, m_scaleFormat.arg( scaleVal, 0, 'f', 0 ) );

	    scaleVal += m_gridSpacing;
	}
    }

    if( m_showGraphTitle ) {
	const QFont oldFont = p->font();
	p->setFont( QFont( "Arial", 16 ) );
	p->setPen( m_textColor );
	p->drawText( graphRect.left(), 0, graphRect.width(), 32, Qt::AlignCenter, m_graphTitle );
	p->setFont( oldFont );
    }

    p->fillRect( graphRect, m_worksheetColor );

    if( m_showHorizontalGrid ) {
	p->setPen( m_gridColor );
	for( int i = 1; i < m_numBars; i++ )
	    p->drawLine( graphRect.left(), graphRect.top() + i * m_barWidth - 1, graphRect.right(), graphRect.top() + i * m_barWidth - 1 );
    }
    if( m_showVerticalGrid ) {
	p->setPen( m_gridColor );
	double gridVal( m_gridSpacing );    // The left edge is already drawn, so we can start directly on the first gridline
	while( gridVal < m_maxValue ) {
	    int gridPos = graphRect.left() + gridVal * graphRect.width() / m_maxValue;
	    p->drawLine( gridPos, graphRect.top(), gridPos, graphRect.bottom() );
	    gridVal += m_gridSpacing;
	}
    }

    for( int b = 0; b < m_numBars; b++ ) {
	p->setPen( m_textColor );
	QRect rc( graphRect.left(), graphRect.top() + b * m_barWidth, 1, m_barWidth );
	QRect valRect;
	int s;
	double tmp;
	switch( m_displayMode ) {
	case 0:
	    for( s = 0; s < m_numSegments; s++ ) {
		rc.setLeft( rc.right() );
		rc.setWidth( graphRect.width() * m_values[ b * m_numSegments + s ] / m_maxValue );
		p->fillRect( rc, m_segmentColors[ s ] );
		if( m_showValues ) {
		    valRect = p->boundingRect( rc.left(), rc.top(), graphRect.width(), graphRect.height(), Qt::AlignAuto | Qt::SingleLine, m_scaleFormat.arg( m_values[ b * m_numSegments + s ] / tmp * 100, 0, 'f', 0 ) );
		    if( valRect.width() < rc.width() )
			p->drawText( rc.left(), rc.top(), rc.width(), rc.height(), Qt::AlignCenter | Qt::SingleLine, m_scaleFormat.arg( m_values[ b * m_numSegments + s ], 0, 'f', 0 ) );
		}
	    }
	    break;
	case 1:
	    for( s = 0; s < m_numSegments; s++ ) {
		get_totalForBar( b, &tmp );
		rc.setLeft( rc.right() );
		rc.setWidth( m_values[ b * m_numSegments + s ] / tmp * graphRect.width() + 0.5 );
		p->fillRect( rc, m_segmentColors[ s ] );
		if( m_showValues ) {
		    valRect = p->boundingRect( rc.left(), rc.top(), graphRect.width(), graphRect.height(), Qt::AlignAuto | Qt::SingleLine, m_scaleFormat.arg( m_values[ b * m_numSegments + s ] / tmp * 100, 0, 'f', 0 ) );
		    if( valRect.width() < rc.width() )
			p->drawText( rc.left(), rc.top(), rc.width(), rc.height(), Qt::AlignCenter | Qt::SingleLine, m_scaleFormat.arg( m_values[ b * m_numSegments + s ] / tmp * 100, 0, 'f', 0 ) );
		}
	    }
	    break;
	}
    }

    p->setPen( m_textColor );
    p->drawRect( graphRect );

    if( m_showExplanation ) {
	p->setPen( m_textColor );
	graphRect.setLeft( 0 );
	graphRect.setTop( graphRect.bottom() + m_barWidth * m_showScale );
	graphRect.setBottom( height );
	int maxWidth( 0 );
	for( int i = 0; i < m_numSegments; i++ ) {
	    QRect r = p->boundingRect( 0, graphRect.top() + ( i + 1 ) * m_barWidth, m_width, m_barWidth, Qt::AlignAuto | Qt::SingleLine, m_segmentNames[ i ] );
	    if( r.width() > maxWidth )
		maxWidth = r.width();
	    p->drawText( 0, graphRect.top() + ( i + 1 ) * m_barWidth, graphRect.width(), m_barWidth, Qt::AlignAuto | Qt::AlignVCenter | Qt::SingleLine, m_segmentNames[ i ] );
	}
	graphRect.setLeft( maxWidth + 5 );
	for( int s = 0; s < m_numSegments; s++ ) {
	    QRect rc( graphRect.left(), graphRect.top() + ( s + 1 ) * m_barWidth, graphRect.width() / 4, m_barWidth );
	    p->fillRect( rc, m_segmentColors[ s ] );
	    p->drawRect( rc );
	}
    }

    delete p;
    
    /*
    ** Save the pixmap as PNG to a buffer, and convert this to a BSTR
    */
    QByteArray a;
    QDataStream ds( a, IO_WriteOnly );
    ds << px;

    *pVal = QByteArray2BSTR( a );

    return S_OK;
}

STDMETHODIMP CHistogram::Reset()
{
    for( int i = 0; i < ( m_numSegments * m_numBars ); i++ )
	m_values[ i ] = 0;
    for( int s = 0; s < m_numSegments; s++ )
	m_segmentNames[ s ] = QString::null;
    for( int b = 0; b < m_numBars; b++ )
	m_barNames[ b ] = QString::null;

    m_graphTitle = QString::null;
    m_scaleFormat = "%1";
    m_displayMode = 0;
    return S_OK;
}
