// Graph.cpp : Implementation of CGraph
#include "stdafx.h"
#include "QGraph.h"
#include "Graph.h"
#include "ocidl.h"	// Added by ClassView

#include <qpixmap.h>
#include <qpainter.h>

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
// CGraph


STDMETHODIMP CGraph::get_Width(long *pVal)
{
    if( pVal )
	*pVal = m_width;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_Width(long newVal)
{
    if( newVal > 0 )
	m_width = newVal;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::get_Height(long *pVal)
{
    if( pVal )
	*pVal = m_height;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_Height(long newVal)
{
    if( newVal > 0 )
	m_height = newVal;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::get_GraphTitle(BSTR *pVal)
{
    if( pVal )
	*pVal = QString2BSTR( m_graphTitle );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_GraphTitle(BSTR newVal)
{
    m_graphTitle = BSTR2QString( newVal );

    return S_OK;
}

STDMETHODIMP CGraph::get_showGraphTitle(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showGraphTitle;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_showGraphTitle(BOOL newVal)
{
    m_showGraphTitle = newVal;

    return S_OK;
}

STDMETHODIMP CGraph::Reset()
{
    for( int g = 0; g < m_numGraphs; g++ ) {
	m_graphNames[ g ] = QString::null;
	for( int v = 0; v < m_numValues; v++ )
	    m_values[ g * m_numValues + v ] = 0.0;
    }
    for( int v = 0; v < m_numValues; v++ )
	m_valueNames[ v ] = QString::null;


    return S_OK;
}

STDMETHODIMP CGraph::get_TextColor(OLE_COLOR *pVal)
{
    if( pVal )
	*pVal = QColor2OLE( m_textColor );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_TextColor(OLE_COLOR newVal)
{
    m_textColor = OLE2QColor( newVal );

    return S_OK;
}

STDMETHODIMP CGraph::get_BackgroundColor(OLE_COLOR *pVal)
{
    if( pVal )
	*pVal = QColor2OLE( m_backgroundColor );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_BackgroundColor(OLE_COLOR newVal)
{
    m_backgroundColor = newVal;

    return S_OK;
}

STDMETHODIMP CGraph::get_GridColor(OLE_COLOR *pVal)
{
    if( pVal )
	*pVal = QColor2OLE( m_gridColor );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_GridColor(OLE_COLOR newVal)
{
    m_gridColor = OLE2QColor( newVal );

    return S_OK;
}

STDMETHODIMP CGraph::get_showHorizontalGrid(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showHorizontalGrid;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_showHorizontalGrid(BOOL newVal)
{
    m_showHorizontalGrid = newVal;

    return S_OK;
}

STDMETHODIMP CGraph::get_showVerticalGrid(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showVerticalGrid;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_showVerticalGrid(BOOL newVal)
{
    m_showVerticalGrid = newVal;

    return S_OK;
}

STDMETHODIMP CGraph::get_VerticalGridSpacing(double *pVal)
{
    if( pVal )
	*pVal = m_verticalGridSpacing;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_VerticalGridSpacing(double newVal)
{
    if( newVal > 0 )
	m_verticalGridSpacing = newVal;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::get_showHorizontalScale(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showHorizontalScale;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_showHorizontalScale(BOOL newVal)
{
    m_showHorizontalScale = newVal;

    return S_OK;
}

STDMETHODIMP CGraph::get_MimeType(BSTR *pVal)
{
    if( pVal )
	*pVal = QString2BSTR( "image/png" );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::get_HorizontalGridSpacing(double *pVal)
{
    if( pVal )
	*pVal = m_horizontalGridSpacing;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_HorizontalGridSpacing(double newVal)
{
    if( newVal > 0 )
	m_horizontalGridSpacing = newVal;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::get_NumValues(long *pVal)
{
    if( pVal )
	*pVal = m_numValues;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_NumValues(long newVal)
{
    if( newVal > 1 ) {
	m_numValues = newVal;
	m_values.resize( m_numValues * m_numGraphs );
	m_valueNames.resize( m_numValues );
    }
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::get_NumGraphs(long *pVal)
{
    if( pVal )
	*pVal = m_numGraphs;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_NumGraphs(long newVal)
{
    if( newVal > 0 ) {
	m_numGraphs = newVal;
	m_graphColors.resize( m_numGraphs );
	m_graphNames.resize( m_numGraphs );
	m_values.resize( m_numGraphs * m_numValues );
    }

    return S_OK;
}

STDMETHODIMP CGraph::get_MaxValue(double *pVal)
{
    if( pVal )
	*pVal = m_maxValue;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_MaxValue(double newVal)
{
    if( newVal > 0 )
	m_maxValue = newVal;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::get_showExplanation(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showExplanation;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_showExplanation(BOOL newVal)
{
    m_showExplanation = newVal;

    return S_OK;
}

STDMETHODIMP CGraph::get_GraphColor( long GraphNumber, OLE_COLOR *pVal)
{
    if( pVal && ( GraphNumber < m_numGraphs ) )
	*pVal = QColor2OLE( m_graphColors[ GraphNumber ] );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_GraphColor( long GraphNumber, OLE_COLOR newVal)
{
    if( GraphNumber < m_numGraphs )
	m_graphColors[ GraphNumber ] = OLE2QColor( newVal );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::get_GraphName(long GraphNumber, BSTR *pVal)
{
    if( pVal && ( GraphNumber < m_numGraphs ) )
	*pVal = QString2BSTR( m_graphNames[ GraphNumber ] );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_GraphName(long GraphNumber, BSTR newVal)
{
    if( GraphNumber < m_numGraphs )
	m_graphNames[ GraphNumber ] = BSTR2QString( newVal );

    return S_OK;
}

STDMETHODIMP CGraph::get_Value(long GraphNum, long Index, double *pVal)
{
    if( pVal && ( GraphNum < m_numGraphs ) && ( Index < m_numValues ) )
	*pVal = m_values[ GraphNum * m_numValues + Index ];
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_Value(long GraphNum, long Index, double newVal)
{
    if( ( GraphNum < m_numGraphs ) && ( Index < m_numValues ) )
	m_values[ GraphNum * m_numValues + Index ] = newVal;

    return S_OK;
}

STDMETHODIMP CGraph::get_showVerticalScale(BOOL *pVal)
{
    if( pVal )
	*pVal = m_showVerticalScale;
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_showVerticalScale(BOOL newVal)
{
    m_showVerticalScale = newVal;

    return S_OK;
}

STDMETHODIMP CGraph::get_scaleFormat(BSTR *pVal)
{
    if( pVal )
	*pVal = QString2BSTR( m_scaleFormat );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_scaleFormat(BSTR newVal)
{
    m_scaleFormat = BSTR2QString( newVal );

    return S_OK;
}

STDMETHODIMP CGraph::get_ValueName(long Index, BSTR *pVal)
{
    if( pVal && ( Index < m_numValues ) )
	*pVal = QString2BSTR( m_valueNames[ Index ] );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_ValueName(long Index, BSTR newVal)
{
    if( Index < m_numValues )
	m_valueNames[ Index ] = BSTR2QString( newVal );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::get_WorksheetColor(OLE_COLOR *pVal)
{
    if( pVal )
	*pVal = QColor2OLE( m_worksheetColor );
    else
	return E_FAIL;

    return S_OK;
}

STDMETHODIMP CGraph::put_WorksheetColor(OLE_COLOR newVal)
{
    m_worksheetColor = OLE2QColor( newVal );

    return S_OK;
}

STDMETHODIMP CGraph::get_Picture(BSTR *pVal)
{
    if( !pVal ) {
	return E_FAIL;
    }

    QPixmap px( m_width, m_height );
    QPainter* p = new QPainter( &px );

    px.fill( m_backgroundColor );

    
    QRect graphRect( 0, 0, m_width, m_height );
    
    if( m_showGraphTitle )
	graphRect.setTop( graphRect.top() + 32 );
    if( m_showHorizontalScale )
	graphRect.setBottom( graphRect.bottom() - 16 );
    if( m_showExplanation )
	graphRect.setBottom( graphRect.bottom() - ( m_numGraphs + 1 ) * 16 );

    int g;
    if( m_showVerticalScale ) {
	p->setPen( m_textColor );
	int maxWidth( 0 );
	double val( m_verticalGridSpacing );
	while( val < m_maxValue ) {
	    QString label = QString( m_scaleFormat ).arg( val, 0, 'f', 0 );
	    QRect r = p->boundingRect( graphRect.left(), graphRect.bottom() - ( ( val + m_verticalGridSpacing ) / m_maxValue ) * graphRect.height(),
				       graphRect.width(), ( m_verticalGridSpacing * 2 / m_maxValue ) * graphRect.height(),
				       Qt::AlignAuto | Qt::SingleLine, label );
	    if( r.width() > maxWidth )
		maxWidth = r.width();
	    p->drawText( graphRect.left(), graphRect.bottom() - ( ( val + m_verticalGridSpacing ) / m_maxValue ) * graphRect.height(),
			 graphRect.width(), ( m_verticalGridSpacing * 2 / m_maxValue ) * graphRect.height(),
			 Qt::AlignAuto | Qt::SingleLine | Qt::AlignVCenter, label );

	    val += m_verticalGridSpacing;
	}
	graphRect.setLeft( graphRect.left() + maxWidth + 5 );
    }
    if( m_showHorizontalScale ) {
	p->setPen( m_textColor );
	double val( m_horizontalGridSpacing );
	while( val < m_numValues ) {
	    p->drawText( graphRect.left() + ( val - m_horizontalGridSpacing ) * graphRect.width() / m_numValues, graphRect.bottom(),
		         ( m_horizontalGridSpacing * 2 ) * graphRect.width() / m_numValues, 16, Qt::AlignCenter, m_valueNames[ int( val ) ] );

	    val += m_horizontalGridSpacing;
	}
    }
    if( m_showGraphTitle )
    {
	const QFont oldFont = p->font();
	p->setFont( QFont( "Arial", 16 ) );
	p->setPen( m_textColor );
	p->drawText( graphRect.left(), 0, graphRect.width(), 32, Qt::AlignCenter, m_graphTitle );
	p->setFont( oldFont );
    }
    p->fillRect( graphRect, m_worksheetColor );
    if( m_showHorizontalGrid ) {
	p->setPen( m_gridColor );
	double val( m_verticalGridSpacing );
	while( val < m_maxValue ) {
	    p->drawLine( graphRect.left(), graphRect.bottom() - ( val / m_maxValue ) * graphRect.height(),
			 graphRect.right(), graphRect.bottom() - ( val / m_maxValue ) * graphRect.height() );

	    val += m_verticalGridSpacing;
	}
    }
    if( m_showVerticalGrid ) {
	p->setPen( m_gridColor );
	double val( m_horizontalGridSpacing );
	while( val < m_numValues ) {
	    p->drawLine( graphRect.left() + val * graphRect.width() / m_numValues, graphRect.top(),
			 graphRect.left() + val * graphRect.width() / m_numValues, graphRect.bottom() );
	    val += m_horizontalGridSpacing;
	}
    }

    for( g = 0; g < m_numGraphs; g++ ) {
	p->setPen( m_graphColors[ g ] );
	QPoint lastPoint;
	for( int i = 0; i < m_numValues; i++ ) {
	    if( !i )
		lastPoint = QPoint( graphRect.left(), graphRect.bottom() - ( m_values[ g * m_numValues + i ] / m_maxValue ) * graphRect.height() );
	    else {
		QPoint newPoint( graphRect.left() + i * graphRect.width() / m_numValues, graphRect.bottom() - ( m_values[ g * m_numValues + i ] / m_maxValue ) * graphRect.height() );
		p->drawLine( lastPoint, newPoint );
		lastPoint = newPoint;
	    }
	}
    }
    if( m_showExplanation ) {
	int maxWidth( 0 );
	p->setPen( m_textColor );
	for( g = 0; g < m_numGraphs; g++ ) {
	    QRect rc = p->boundingRect( 0, graphRect.bottom() + ( g + 1 ) * 16, m_width, 16, Qt::AlignAuto | Qt::SingleLine, m_graphNames[ g ] );
	    if( rc.width() > maxWidth )
		maxWidth = rc.width();
	    p->drawText( 0, graphRect.bottom() + ( g + 1 ) * 16, m_width, 16, Qt::AlignAuto | Qt::AlignVCenter, m_graphNames[ g ] );
	}
	for( g = 0; g < m_numGraphs; g++ ) {
	    p->setPen( m_graphColors[ g ] );
	    p->drawLine( maxWidth + 5, graphRect.bottom() + ( g + 1 ) * 16 + 8, maxWidth + 85, graphRect.bottom() + ( g + 1 ) * 16 + 8 );
	}
    }
    p->setPen( m_textColor );
    p->drawRect( graphRect );

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

