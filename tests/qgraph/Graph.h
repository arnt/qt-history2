// Graph.h : Declaration of the CGraph

#ifndef __GRAPH_H_
#define __GRAPH_H_

#include "resource.h"       // main symbols
#include <qstring.h>
#include <qcolor.h>
#include <qvaluevector.h>

/////////////////////////////////////////////////////////////////////////////
// CGraph
class ATL_NO_VTABLE CGraph : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CGraph, &CLSID_Graph>,
    public IDispatchImpl<IGraph, &IID_IGraph, &LIBID_QGRAPHLib>
{
public:
    CGraph()
    {
	m_width = 160;
	m_height = 160;

	put_NumGraphs( 1 );
	put_NumValues( 100 );

	m_maxValue = 100;
	m_verticalGridSpacing = 10;
	m_horizontalGridSpacing = 10;

	m_showGraphTitle = true;
	m_showHorizontalGrid = true;
	m_showVerticalGrid = true;
	m_showVerticalScale = true;
	m_showHorizontalScale = true;
	m_showExplanation = true;

	m_graphColors[ 0 ] = Qt::black;
	m_graphNames[ 0 ] = QString::null;

	m_textColor = Qt::black;
	m_backgroundColor = Qt::white;
	m_gridColor = Qt::lightGray;
	m_worksheetColor = Qt::white;

	m_scaleFormat = "%1";
    }

DECLARE_REGISTRY_RESOURCEID(IDR_GRAPH)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CGraph)
    COM_INTERFACE_ENTRY(IGraph)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IGraph
public:
    STDMETHOD(get_WorksheetColor)(/*[out, retval]*/ OLE_COLOR *pVal);
    STDMETHOD(put_WorksheetColor)(/*[in]*/ OLE_COLOR newVal);
    STDMETHOD(get_ValueName)(long Index, /*[out, retval]*/ BSTR *pVal);
    STDMETHOD(put_ValueName)(long Index, /*[in]*/ BSTR newVal);
    STDMETHOD(get_scaleFormat)(/*[out, retval]*/ BSTR *pVal);
    STDMETHOD(put_scaleFormat)(/*[in]*/ BSTR newVal);
    STDMETHOD(get_showVerticalScale)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showVerticalScale)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_Value)(long GraphNum, long Index, /*[out, retval]*/ double *pVal);
    STDMETHOD(put_Value)(long GraphNum, long Index, /*[in]*/ double newVal);
    STDMETHOD(get_GraphName)(long GraphNumber, /*[out, retval]*/ BSTR *pVal);
    STDMETHOD(put_GraphName)(long GraphNumber, /*[in]*/ BSTR newVal);
    STDMETHOD(get_GraphColor)(/*[in]*/ long GraphNumber,/*[out, retval]*/ OLE_COLOR *pVal);
    STDMETHOD(put_GraphColor)(/*[in]*/ long GraphNumber,/*[in]*/ OLE_COLOR newVal);
    STDMETHOD(get_showExplanation)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showExplanation)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_MaxValue)(/*[out, retval]*/ double *pVal);
    STDMETHOD(put_MaxValue)(/*[in]*/ double newVal);
    STDMETHOD(get_NumGraphs)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_NumGraphs)(/*[in]*/ long newVal);
    STDMETHOD(get_NumValues)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_NumValues)(/*[in]*/ long newVal);
    STDMETHOD(get_HorizontalGridSpacing)(/*[out, retval]*/ double *pVal);
    STDMETHOD(put_HorizontalGridSpacing)(/*[in]*/ double newVal);
    STDMETHOD(get_MimeType)(/*[out, retval]*/ BSTR *pVal);
    STDMETHOD(get_showHorizontalScale)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showHorizontalScale)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_VerticalGridSpacing)(/*[out, retval]*/ double *pVal);
    STDMETHOD(put_VerticalGridSpacing)(/*[in]*/ double newVal);
    STDMETHOD(get_showVerticalGrid)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showVerticalGrid)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_showHorizontalGrid)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showHorizontalGrid)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_GridColor)(/*[out, retval]*/ OLE_COLOR *pVal);
    STDMETHOD(put_GridColor)(/*[in]*/ OLE_COLOR newVal);
    STDMETHOD(get_BackgroundColor)(/*[out, retval]*/ OLE_COLOR *pVal);
    STDMETHOD(put_BackgroundColor)(/*[in]*/ OLE_COLOR newVal);
    STDMETHOD(get_TextColor)(/*[out, retval]*/ OLE_COLOR *pVal);
    STDMETHOD(put_TextColor)(/*[in]*/ OLE_COLOR newVal);
    STDMETHOD(get_Picture)(/*[out, retval]*/ BSTR *pVal);
    STDMETHOD(Reset)();
    STDMETHOD(get_showGraphTitle)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showGraphTitle)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_GraphTitle)(/*[out, retval]*/ BSTR *pVal);
    STDMETHOD(put_GraphTitle)(/*[in]*/ BSTR newVal);
    STDMETHOD(get_Height)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_Height)(/*[in]*/ long newVal);
    STDMETHOD(get_Width)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_Width)(/*[in]*/ long newVal);

private:
    bool m_showGraphTitle;
    bool m_showHorizontalGrid;
    bool m_showVerticalGrid;
    bool m_showHorizontalScale;
    bool m_showVerticalScale;
    bool m_showExplanation;

    long m_numValues;
    long m_numGraphs;

    double m_maxValue;
    double m_verticalGridSpacing;
    double m_horizontalGridSpacing;

    QString m_graphTitle;
    QString m_scaleFormat;
    QValueVector<QColor> m_graphColors;
    QValueVector<QString> m_graphNames;
    QValueVector<double> m_values;
    QValueVector<QString> m_valueNames;
    QColor m_textColor;
    QColor m_backgroundColor;
    QColor m_gridColor;
    QColor m_worksheetColor;

    long m_width;
    long m_height;
};

#endif //__GRAPH_H_
