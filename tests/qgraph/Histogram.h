// Histogram.h : Declaration of the CHistogram

#ifndef __HISTOGRAM_H_
#define __HISTOGRAM_H_

#include "resource.h"       // main symbols

#include <qvaluevector.h>
#include <qcolor.h>
#include <qstring.h>

/////////////////////////////////////////////////////////////////////////////
// CHistogram
class ATL_NO_VTABLE CHistogram : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CHistogram, &CLSID_Histogram>,
    public IDispatchImpl<IHistogram, &IID_IHistogram, &LIBID_QGRAPHLib>
{
public:

CHistogram()
{
    m_numSegments = 1;
    m_numBars = 1;
    m_barWidth = 16;
    m_width = 160;
    m_maxValue = 100.0;
    m_gridSpacing = 25.0;
    m_showValues = true;
    m_showBarNames = true;
    m_showExplanation = true;
    m_showVerticalGrid = true;
    m_showHorizontalGrid = true;
    m_showScale = true;
    m_showGraphTitle = true;
    m_showTotal = true;
    m_gridColor = Qt::lightGray;
    m_backgroundColor = Qt::white;
    m_textColor = Qt::black;
    m_worksheetColor = Qt::white;
    m_graphTitle = QString::null;
    m_scaleFormat = "%1";
    m_displayMode = 0;

    m_values.resize( m_numBars * m_numSegments );
    m_barNames.resize( m_numBars );

    m_segmentColors.resize( m_numSegments );
    m_segmentColors[ 0 ] = Qt::blue;
    m_segmentNames.resize( m_numSegments );
}

DECLARE_REGISTRY_RESOURCEID(IDR_HISTOGRAM)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CHistogram)
    COM_INTERFACE_ENTRY(IHistogram)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IHistogram
public:
    STDMETHOD(get_DisplayMode)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_DisplayMode)(/*[in]*/ long newVal);
    STDMETHOD(get_WorksheetColor)(/*[out, retval]*/ OLE_COLOR *pVal);
    STDMETHOD(put_WorksheetColor)(/*[in]*/ OLE_COLOR newVal);
    STDMETHOD(get_showTotal)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showTotal)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_showGraphTitle)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showGraphTitle)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_GraphTitle)(/*[out, retval]*/ BSTR *pVal);
    STDMETHOD(put_GraphTitle)(/*[in]*/ BSTR newVal);
    STDMETHOD(get_ScaleFormat)(/*[out, retval]*/ BSTR *pVal);
    STDMETHOD(put_ScaleFormat)(/*[in]*/ BSTR newVal);
    STDMETHOD(get_totalForBar)(/*[in]*/ long BarNumber,/*[out, retval]*/ double *pVal);
    STDMETHOD(get_showScale)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showScale)(/*[in]*/ BOOL newVal);
    STDMETHOD(Reset)();
    STDMETHOD(get_GridSpacing)(/*[out, retval]*/ double *pVal);
    STDMETHOD(put_GridSpacing)(/*[in]*/ double newVal);
    STDMETHOD(get_MimeType)(/*[out, retval]*/ BSTR *pVal);
    STDMETHOD(get_TextColor)(/*[out, retval]*/ OLE_COLOR *pVal);
    STDMETHOD(put_TextColor)(/*[in]*/ OLE_COLOR newVal);
    STDMETHOD(get_ShowValues)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_ShowValues)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_showVerticalGrid)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showVerticalGrid)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_GridColor)(/*[out, retval]*/ OLE_COLOR *pVal);
    STDMETHOD(put_GridColor)(/*[in]*/ OLE_COLOR newVal);
    STDMETHOD(get_showHorizontalGrid)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showHorizontalGrid)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_BackgroundColor)(/*[out, retval]*/ OLE_COLOR *pVal);
    STDMETHOD(put_BackgroundColor)(/*[in]*/ OLE_COLOR newVal);
    STDMETHOD(get_showBarNames)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showBarNames)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_BarName)(long BarNumber, /*[out, retval]*/ BSTR *pVal);
    STDMETHOD(put_BarName)(long BarNumber, /*[in]*/ BSTR newVal);
    STDMETHOD(get_SegmentName)(long SegmentNumber, /*[out, retval]*/ BSTR *pVal);
    STDMETHOD(put_SegmentName)(long SegmentNumber, /*[in]*/ BSTR newVal);
    STDMETHOD(get_showExplanation)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_showExplanation)(/*[in]*/ BOOL newVal);
    STDMETHOD(get_Picture)(/*[out, retval]*/ BSTR *pVal);
    STDMETHOD(get_BarWidth)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_BarWidth)(/*[in]*/ long newVal);
    STDMETHOD(get_Height)(/*[out, retval]*/ long *pVal);
    STDMETHOD(get_Width)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_Width)(/*[in]*/ long newVal);
    STDMETHOD(get_Value)(long BarNumber, long SegmentNumber, /*[out, retval]*/ double *pVal);
    STDMETHOD(put_Value)(long BarNumber, long SegmentNumber, /*[in]*/ double newVal);
    STDMETHOD(get_SegmentColor)(long SegmentNum, /*[out, retval]*/ OLE_COLOR *pVal);
    STDMETHOD(put_SegmentColor)(long SegmentNum, /*[in]*/ OLE_COLOR newVal);
    STDMETHOD(get_MaxValue)(/*[out, retval]*/ double *pVal);
    STDMETHOD(put_MaxValue)(/*[in]*/ double newVal);
    STDMETHOD(get_NumSegments)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_NumSegments)(/*[in]*/ long newVal);
    STDMETHOD(get_NumBars)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_NumBars)(/*[in]*/ long newVal);
private:
    long m_width;
    long m_barWidth;
    long m_numBars;
    long m_numSegments;
    long m_displayMode;
    double m_maxValue;
    double m_gridSpacing;

    QValueVector<double> m_values;
    QValueVector<QColor> m_segmentColors;
    QValueVector<QString> m_segmentNames;
    QValueVector<QString> m_barNames;
    QColor m_backgroundColor;
    QColor m_gridColor;
    QColor m_textColor;
    QColor m_worksheetColor;
    bool m_showHorizontalGrid;
    bool m_showVerticalGrid;
    bool m_showBarNames;
    bool m_showExplanation;
    bool m_showValues;
    bool m_showScale;
    bool m_showGraphTitle;
    bool m_showTotal;
    QString m_scaleFormat;
    QString m_graphTitle;
};

#endif //__HISTOGRAM_H_
