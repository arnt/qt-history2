/****************************************************************************
**
** Implementation of type conversion routines.
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#define QT_NO_CAST_TO_ASCII

#include <ocidl.h>
#include <olectl.h>

#include "types.h"
#include <qpixmap.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qobject.h>
#ifdef QAX_SERVER
#include <qaxfactory.h>
#include <qlibrary.h>
extern ITypeLib *qAxTypeLibrary;

CLSID CLSID_QRect = { 0x34030f30, 0xe359, 0x4fe6, {0xab, 0x82, 0x39, 0x76, 0x6f, 0x5d, 0x91, 0xee } };
CLSID CLSID_QSize = { 0xcb5f84b3, 0x29e5, 0x491d, {0xba, 0x18, 0x54, 0x72, 0x48, 0x8e, 0xef, 0xba } };
CLSID CLSID_QPoint = { 0x3be838a3, 0x3fac, 0xbfc4, {0x4c, 0x6c, 0x37, 0xc4, 0x4d, 0x03, 0x02, 0x52 } };

GUID IID_IAxServerBase = { 0xbd2ec165, 0xdfc9, 0x4319, { 0x8b, 0x9b, 0x60, 0xa5, 0x74, 0x78, 0xe9, 0xe3} };
#endif

IFontDisp *QFontToIFont(const QFont &font)
{
    FONTDESC fdesc;
    memset(&fdesc, 0, sizeof(fdesc));
    fdesc.cbSizeofstruct = sizeof(FONTDESC);
    fdesc.cySize.Lo = font.pointSize() * 10000;
    fdesc.fItalic = font.italic();
    fdesc.fStrikethrough = font.strikeOut();
    fdesc.fUnderline = font.underline();
    fdesc.lpstrName = QStringToBSTR(font.family());
    fdesc.sWeight = font.weight() * 10;
    
    IFontDisp *f;
    HRESULT res = OleCreateFontIndirect(&fdesc, IID_IFontDisp, (void**)&f);
    if (res != S_OK) {
        if (f) f->Release();
        f = 0;
#if defined(QT_CHECK_STATE)
        qWarning("QFontToIFont: Failed to create IFont");
#endif
    }
    return f;
}

QFont IFontToQFont(IFont *f)
{
    BSTR name;
    BOOL bold;
    SHORT charset;
    BOOL italic;
    CY size;
    BOOL strike;
    BOOL underline;
    SHORT weight;
    f->get_Name(&name);
    f->get_Bold(&bold);
    f->get_Charset(&charset);
    f->get_Italic(&italic);
    f->get_Size(&size);
    f->get_Strikethrough(&strike);
    f->get_Underline(&underline);
    f->get_Weight(&weight);
    QFont font(BSTRToQString(name), size.Lo/9750, weight / 97, italic);
    font.setBold(bold);
    font.setStrikeOut(strike);
    font.setUnderline(underline);
    SysFreeString(name);
    
    return font;
}

IPictureDisp *QPixmapToIPicture(const QPixmap &pixmap)
{
    IPictureDisp *pic = 0;
    
    PICTDESC desc;
    desc.cbSizeofstruct = sizeof(PICTDESC);
    desc.picType = PICTYPE_BITMAP;
    
    desc.bmp.hbitmap = 0;
    desc.bmp.hpal = QColor::hPal();
    
    if (!pixmap.isNull()) {
        HDC hdc = ::CreateCompatibleDC(pixmap.handle());
        if (!hdc) {
#if defined(QT_CHECK_STATE)
            qSystemWarning("QPixmapToIPicture: Failed to create compatible device context");
#endif
            return 0;
        }
        HBITMAP hbm = ::CreateCompatibleBitmap(pixmap.handle(), pixmap.width(), pixmap.height());
        if (!hbm) {
#if defined(QT_CHECK_STATE)
            qSystemWarning("QPixmapToIPicture: Failed to create compatible bitmap");
#endif
            return 0;
        }
        ::SelectObject(hdc, hbm);
        BOOL res = ::BitBlt(hdc, 0, 0, pixmap.width(), pixmap.height(), pixmap.handle(), 0, 0, SRCCOPY);
        
        ::DeleteObject(hdc);
        
        if (!res) {
#if defined(QT_CHECK_STATE)
            qSystemWarning("QPixmapToIPicture: Failed to BitBlt bitmap");
#endif
            return 0;
        }
        
        desc.bmp.hbitmap = hbm;
    }
    
    HRESULT res = OleCreatePictureIndirect(&desc, IID_IPictureDisp, true, (void**)&pic);
    if (res != S_OK) {
        if (pic) pic->Release();
        pic = 0;
#if defined(QT_CHECK_STATE)
        qWarning("QPixmapToIPicture: Failed to create IPicture");
#endif
    }
    return pic;
}

QPixmap IPictureToQPixmap(IPicture *ipic)
{
    SHORT type;
    ipic->get_Type(&type);
    if (type != PICTYPE_BITMAP)
        return QPixmap();
    
    QPixmap pm(1,1);
    QPaintDeviceMetrics pdm(&pm);
    OLE_XSIZE_HIMETRIC pWidth, pHeight;
    ipic->get_Width(&pWidth);
    ipic->get_Height(&pHeight);
    QSize sz(MAP_LOGHIM_TO_PIX(pWidth, pdm.logicalDpiX()),
        MAP_LOGHIM_TO_PIX(pHeight, pdm.logicalDpiY()));
    
    pm.resize(sz);
    ipic->Render(pm.handle(), 0, 0, pm.width(), pm.height(), 0, pHeight, pWidth, -pHeight, 0);
    
    return pm;
}

QDateTime DATEToQDateTime(DATE ole)
{
    SYSTEMTIME stime;
    if (ole >= 949998 || VariantTimeToSystemTime(ole, &stime) == false)
        return QDateTime();
    
    QDate date(stime.wYear, stime.wMonth, stime.wDay);
    QTime time(stime.wHour, stime.wMinute, stime.wSecond, stime.wMilliseconds);
    return QDateTime(date, time);
}

DATE QDateTimeToDATE(const QDateTime &dt)
{
    if (!dt.isValid() || dt.isNull())
        return 949998;
    
    SYSTEMTIME stime;
    memset(&stime, 0, sizeof(stime));
    QDate date = dt.date();
    QTime time = dt.time();
    if (date.isValid() && !date.isNull()) {
        stime.wDay = date.day();
        stime.wMonth = date.month();
        stime.wYear = date.year();
    }
    if (time.isValid() && !time.isNull()) {
        stime.wMilliseconds = time.msec();
        stime.wSecond = time.second();
        stime.wMinute = time.minute();
        stime.wHour = time.hour();
    }
    
    double vtime;
    SystemTimeToVariantTime(&stime, &vtime);
    
    return vtime;
}

uint QColorToOLEColor(const QColor &col)
{
    return qRgba(col.blue(), col.green(), col.red(), 0x00);
}

QColor OLEColorToQColor(uint col)
{
    COLORREF cref;
    OleTranslateColor(col, QColor::hPal(), &cref);
    return QColor(GetRValue(cref),GetGValue(cref),GetBValue(cref));
}

/*
Converts \a var to \a arg, and tries to coerce \a arg to \a type.

  Used by
  
    QAxServerBase:
    - IDispatch::Invoke(PROPERTYGET)
    - IPersistPropertyBag::Save
    
      QAxBase
      - QAxBase::qt_property(PropertySet)
      - QAxBase::internalInvoke(properties)
      - IPropertyBag::Read.
*/
bool QVariantToVARIANT(const QVariant &var, VARIANT &arg, const QString &type, bool out)
{
    QVariant qvar = var;
    // "type" is the expected type, so coerce if necessary
    QVariant::Type proptype = (!type.isEmpty()) ? QVariant::nameToType(type.latin1()) : QVariant::Invalid;
    if (proptype == QVariant::Invalid) {
        if (type == "short" || type == "char")
            proptype = QVariant::Int;
        else if (type == "float")
            proptype = QVariant::Double;
    }
    if (proptype != QVariant::Invalid && proptype != qvar.type()) {
        if (qvar.canCast(proptype))
            qvar.cast(proptype);
        else
            qvar = QVariant(proptype);
    }
    
    switch ((int)qvar.type()) {
    case QVariant::String:
        if (out && arg.vt == (VT_BSTR|VT_BYREF)) {
            *arg.pbstrVal = QStringToBSTR(qvar.toString());
            arg.vt = VT_BSTR|VT_BYREF;
        } else {
            arg.vt = VT_BSTR;
            arg.bstrVal = QStringToBSTR(qvar.toString());
            if (out)
                arg.pbstrVal = new BSTR(arg.bstrVal);
        }
        break;
        
    case QVariant::Int:
        if (out && arg.vt == (VT_I4|VT_BYREF)) {
            *arg.plVal = qvar.toInt();
        } else {
            arg.vt = VT_I4;
            arg.lVal = qvar.toInt();
            if (out) {
                if (type == "short") {
                    arg.vt = VT_I2;
                    arg.piVal = new short(arg.lVal);
                } else if (type == "char") {
                    arg.vt = VT_I1;
                    arg.pcVal= new char(arg.lVal);
                } else
                    arg.plVal = new long(arg.lVal);
            }
        }
        break;
        
    case QVariant::UInt:
        if (out && arg.vt == (VT_UINT|VT_BYREF)) {
            *arg.puintVal = qvar.toUInt();
        } else {
            arg.vt = VT_UINT;
            arg.uintVal = qvar.toUInt();
            if (out)
                arg.puintVal = new uint(arg.uintVal);
        }
        break;
        
    case QVariant::LongLong:
        if (out && arg.vt == (VT_CY|VT_BYREF)) {
            arg.pcyVal->int64 = qvar.toLongLong();
        } else {
            arg.vt = VT_CY;
            arg.cyVal.int64 = qvar.toLongLong();
            if (out)
                arg.pcyVal = new CY(arg.cyVal);
        }
        break;
        
    case QVariant::ULongLong:
        if (out && arg.vt == (VT_CY|VT_BYREF)) {
            arg.pcyVal->int64 = qvar.toULongLong();
        } else {
            arg.vt = VT_CY;
            arg.cyVal.int64 = qvar.toULongLong();
            if (out)
                arg.pcyVal = new CY(arg.cyVal);
        }
        break;
        
    case QVariant::Bool:
        if (out && arg.vt == (VT_BOOL|VT_BYREF)) {
            *arg.pboolVal = qvar.toBool() ? VARIANT_TRUE : VARIANT_FALSE;
        } else {
            arg.vt = VT_BOOL;
            arg.boolVal = qvar.toBool() ? VARIANT_TRUE : VARIANT_FALSE;
            if (out)
                arg.pboolVal = new short(arg.boolVal);
        }
        break;
    case QVariant::Double:
        if (out && arg.vt == (VT_R8|VT_BYREF)) {
            *arg.pdblVal = qvar.toDouble();
        } else {
            arg.vt = VT_R8;
            arg.dblVal = qvar.toDouble();
            if (out) {
                if (type == "float") {
                    arg.vt = VT_R4;
                    arg.pfltVal = new float(arg.dblVal);                    
                } else {
                    arg.pdblVal = new double(arg.dblVal);
                }
            }
        }
        break;
    case QVariant::Color:
        if (out && arg.vt == (VT_COLOR|VT_BYREF)) {
            *arg.plVal = QColorToOLEColor(qvar.toColor());
        } else {
            arg.vt = VT_COLOR;
            arg.lVal = QColorToOLEColor(qvar.toColor());
            if (out)
                arg.plVal = new long(arg.lVal);
        }
        break;
        
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
        if (out && arg.vt == (VT_DATE|VT_BYREF)) {
            *arg.pdate = QDateTimeToDATE(qvar.toDateTime());
        } else {
            arg.vt = VT_DATE;
            arg.date = QDateTimeToDATE(qvar.toDateTime());
            if (out)
                arg.pdate = new DATE(arg.date);
        }
        break;
    case QVariant::Font:
        if (out && arg.vt == (VT_DISPATCH|VT_BYREF)) {
            *arg.ppdispVal = QFontToIFont(qvar.toFont());
        } else {
            arg.vt = VT_DISPATCH;
            arg.pdispVal = QFontToIFont(qvar.toFont());
            if (out)
                arg.ppdispVal = new IDispatch*(arg.pdispVal);
        }
        break;
        
    case QVariant::Pixmap:
        if (out && arg.vt == (VT_DISPATCH|VT_BYREF)) {
            *arg.ppdispVal = QPixmapToIPicture(qvar.toPixmap());
        } else {
            arg.vt = VT_DISPATCH;
            arg.pdispVal = QPixmapToIPicture(qvar.toPixmap());
            if (out)
                arg.ppdispVal = new IDispatch*(arg.pdispVal);
        }
        break;
        
    case QVariant::List:
        {
            const QList<QCoreVariant> list = qvar.toList();
            const int count = list.count();
            SAFEARRAY *array = SafeArrayCreateVector(VT_VARIANT, 0, count);
            for (LONG index = 0; index < count; ++index) {
                QVariant elem = list.at(index);
                VARIANT var;
                VariantInit(&var);
                QVariantToVARIANT(elem, var, elem.typeName());
                SafeArrayPutElement(array, &index, &var);
                clearVARIANT(&var);
            }
            
            if (out && arg.vt == (VT_ARRAY|VT_VARIANT|VT_BYREF)) {
                *arg.pparray = array;
            } else {
                arg.vt = VT_ARRAY|VT_VARIANT;
                arg.parray = array;
                if (out)
                    arg.pparray = new SAFEARRAY*(arg.parray);
            }
        }
        break;
        
    case QVariant::StringList:
        {
            const QStringList list = qvar.toStringList();
            const int count = list.count();
            SAFEARRAY *array = SafeArrayCreateVector(VT_BSTR, 0, count);
            for (LONG index = 0; index < count; ++index) {
                QString elem = list.at(index);
                BSTR bstr = QStringToBSTR(elem);
                SafeArrayPutElement(array, &index, bstr);
                SysFreeString(bstr);
            }
            
            if (out && arg.vt == (VT_ARRAY|VT_BSTR|VT_BYREF)) {
                *arg.pparray = array;
            } else {
                arg.vt = VT_ARRAY|VT_BSTR;
                arg.parray = array;
                if (out)
                    arg.pparray = new SAFEARRAY*(arg.parray);
            }
        }
        break;
        
    case QVariant::ByteArray:
        {
            const QByteArray bytes = qvar.toByteArray();
            const uint count = bytes.count();
            SAFEARRAY *array =	SafeArrayCreateVector(VT_UI1, 0, count);
            if (count) {
                const char *data = bytes.constData();
                char *dest;
                SafeArrayAccessData(array, (void **)&dest);
                memcpy(dest, data, count);
                SafeArrayUnaccessData(array);
            }
            
            if (out && arg.vt == (VT_ARRAY|VT_UI1|VT_BYREF)) {
                *arg.pparray = array;
            } else {
                arg.vt = VT_ARRAY|VT_UI1;
                arg.parray = array;
                if (out)
                    arg.pparray = new SAFEARRAY*(arg.parray);
            }
        }
        break;
        
#ifdef QAX_SERVER
    case QVariant::Rect:
    case QVariant::Size:
    case QVariant::Point:
        {
            typedef HRESULT(WINAPI* PGetRecordInfoFromTypeInfo)(ITypeInfo *, IRecordInfo **);
            static PGetRecordInfoFromTypeInfo pGetRecordInfoFromTypeInfo = 0;
            static bool resolved = false;
            if (!resolved) {
                resolved = true;
                pGetRecordInfoFromTypeInfo = (PGetRecordInfoFromTypeInfo)QLibrary::resolve("oleaut32", "GetRecordInfoFromTypeInfo");
            }
            if (!pGetRecordInfoFromTypeInfo)
                break;
            
            ITypeInfo *typeInfo = 0;
            IRecordInfo *recordInfo = 0;
            CLSID clsid = qvar.type() == QVariant::Rect ? CLSID_QRect
                :qvar.type() == QVariant::Size ? CLSID_QSize
                :CLSID_QPoint;
            qAxTypeLibrary->GetTypeInfoOfGuid(clsid, &typeInfo);
            if (!typeInfo)
                break;
            pGetRecordInfoFromTypeInfo(typeInfo, &recordInfo);
            typeInfo->Release();
            if (!recordInfo)
                break;
            
            void *record = 0;
            switch (qvar.type()) {
            case QVariant::Rect:
                {
                    QRect qrect(qvar.toRect());
                    recordInfo->RecordCreateCopy(&qrect, &record);
                }
                break;
            case QVariant::Size:
                {
                    QSize qsize(qvar.toSize());
                    recordInfo->RecordCreateCopy(&qsize, &record);
                }
                break;
            case QVariant::Point:
                {
                    QPoint qpoint(qvar.toPoint());
                    recordInfo->RecordCreateCopy(&qpoint, &record);
                }
                break;
            }
            
            arg.vt = VT_RECORD;
            arg.pRecInfo = recordInfo,
                arg.pvRecord = record;
        }
        break;
#endif // QAX_SERVER
    case QVariant::UserType:
        {
            QVariant::UserData userData(qvar.toUserType());
#ifdef QAX_SERVER
            QString subType = userData.description();
            if (subType.endsWith("*"))
                subType.truncate(subType.length() - 1);
#endif
            if (userData.description() == "IDispatch*") {
                arg.vt = VT_DISPATCH;
                arg.pdispVal = (IDispatch*)userData.data();
                if (arg.pdispVal)
                    arg.pdispVal->AddRef();
            } else if (userData.description() == "IUnknown*") {
                arg.vt = VT_UNKNOWN;
                arg.punkVal = (IUnknown*)userData.data();
                if (arg.punkVal)
                    arg.punkVal->AddRef();
#ifdef QAX_SERVER
            } else if (qAxFactory()->metaObject(subType)) {
                arg.vt = VT_DISPATCH;
                if (!qvar.constData()) {
                    arg.pdispVal = 0;
                } else {
                    qAxFactory()->createObjectWrapper(static_cast<QObject*>(userData.data()), &arg.pdispVal);
                }
#endif
            } else {
                return false;
            }
        }
        break;
    default:
        return false;
    }
    
    if (out)
        arg.vt |= VT_BYREF;
    
    return true;
}

/*!
    Copies the data in \a var into \a data.

    Used by:
    QAxBase:
        - internalProperty(ReadProperty)
*/
bool QVariantToVoidStar(const QVariant &var, void *data)
{
    switch (var.type()) {
    case QVariant::String:
        *(QString*)data = var.toString();
        break;
    case QVariant::Int:
        *(int*)data = var.toInt();
        break;
    case QVariant::UInt:
        *(uint*)data = var.toUInt();
        break;
    case QVariant::Bool:
        *(bool*)data = var.toBool();
        break;
    case QVariant::Double:
        *(double*)data = var.toDouble();
        break;
    case QVariant::Color:
        *(QColor*)data = var.toColor();
        break;
    case QVariant::Date:
        *(QDate*)data = var.toDate();
        break;
    case QVariant::Time:
        *(QTime*)data = var.toTime();
        break;
    case QVariant::DateTime:
        *(QDateTime*)data = var.toDateTime();
        break;
    case QVariant::Font:
        *(QFont*)data = var.toFont();
        break;
    case QVariant::Pixmap:
        *(QPixmap*)data = var.toPixmap();
        break;
    case QVariant::List:
        *(QList<QCoreVariant>*)data = var.toList();
        break;
    case QVariant::StringList:
        *(QStringList*)data = var.toStringList();
        break;
    case QVariant::ByteArray:
        *(QByteArray*)data = var.toByteArray();
        break;
    case QVariant::LongLong:
        *(Q_LLONG*)data = var.toLongLong();
        break;
    case QVariant::ULongLong:
        *(Q_ULLONG*)data = var.toULongLong();
        break;
    case QVariant::Rect:
        *(QRect*)data = var.toRect();
        break;
    case QVariant::Size:
        *(QSize*)data = var.toSize();
        break;
    case QVariant::Point:
        *(QPoint*)data = var.toPoint();
        break;
    default:
        return false;
    }
    
    return true;
}
  
/*!
    Returns \a arg as a QVariant of type \a hint.

    Used by:
    QAxBase
        - QAxBase::qt_property(PropertyGet)
        - QAxBase::internalInvoke(update out parameters)
        - QAxBase::dynamicCall(return value)
        - IPropertyBag::Write

    QAxServerBase::
        - IDispatch::Invoke(PropertyPut)
        - IPersistPropertyBag::Load
*/
QVariant VARIANTToQVariant(const VARIANT &arg, const QString &hint)
{
    QVariant var;
    switch(arg.vt) {
    case VT_BSTR:
        var = BSTRToQString(arg.bstrVal);
        break;
    case VT_BSTR|VT_BYREF:
        var = BSTRToQString(*arg.pbstrVal);
        break;
    case VT_BOOL:
        var = QVariant((bool)arg.boolVal);
        break;
    case VT_BOOL|VT_BYREF:
        var = QVariant((bool)*arg.pboolVal);
        break;
    case VT_I1:
        var = arg.cVal;
        break;
    case VT_I1|VT_BYREF:
        var = *arg.pcVal;
        break;
    case VT_I2:
        var = arg.iVal;
        break;
    case VT_I2|VT_BYREF:
        var = *arg.piVal;
        break;
    case VT_I4:
        if (hint == "QColor")
            var = OLEColorToQColor(arg.lVal);
        else
            var = (int)arg.lVal;
        break;
    case VT_I4|VT_BYREF:
        if (hint == "QColor")
            var = OLEColorToQColor((int)*arg.plVal);
        else
            var = (int)*arg.plVal;
        break;
    case VT_INT:
        var = arg.intVal;
        break;
    case VT_INT|VT_BYREF:
        var = *arg.pintVal;
        break;
    case VT_UI1:
        var = arg.bVal;
        break;
    case VT_UI1|VT_BYREF:
        var = *arg.pbVal;
        break;
    case VT_UI2:
        var = arg.uiVal;
        break;
    case VT_UI2|VT_BYREF:
        var = *arg.puiVal;
        break;
    case VT_UI4:
        if (hint == "QColor")
            var = OLEColorToQColor(arg.ulVal);
        else
            var = (int)arg.ulVal;
        break;
    case VT_UI4|VT_BYREF:
        if (hint == "QColor")
            var = OLEColorToQColor((uint)*arg.pulVal);
        else
            var = (int)*arg.pulVal;
        break;
    case VT_UINT:
        var = arg.uintVal;
        break;
    case VT_UINT|VT_BYREF:
        var = *arg.puintVal;
        break;
    case VT_CY:
        var = arg.cyVal.int64;
        break;
    case VT_CY|VT_BYREF:
        var = arg.pcyVal->int64;
        break;
    case VT_R4:
        var = arg.fltVal;
        break;
    case VT_R4|VT_BYREF:
        var = *arg.pfltVal;
        break;
    case VT_R8:
        var = arg.dblVal;
        break;
    case VT_R8|VT_BYREF:
        var = *arg.pdblVal;
        break;
    case VT_DATE:
        var = DATEToQDateTime(arg.date);
        break;
    case VT_DATE|VT_BYREF:
        var = DATEToQDateTime(*arg.pdate);
        break;
    case VT_VARIANT:
    case VT_VARIANT|VT_BYREF:
        if (arg.pvarVal)
            var = VARIANTToQVariant(*arg.pvarVal, hint);
        break;
        
    case VT_DISPATCH:
    case VT_DISPATCH|VT_BYREF:
        {
            // pdispVal and ppdispVal are a union
            IDispatch *disp = 0;
            if (arg.vt & VT_BYREF)
                disp = *arg.ppdispVal;
            else
                disp = arg.pdispVal;
            if (hint == "QFont" || hint == "QFont*") {
                IFont *ifont = 0;
                if (disp)
                    disp->QueryInterface(IID_IFont, (void**)&ifont);
                if (ifont) {
                    var = IFontToQFont(ifont);
                    ifont->Release();
                } else {
                    var = QFont();
                }
            } else if (hint == "QPixmap" || hint == "QPixmap*") {
                IPicture *ipic = 0;
                if (disp)
                    disp->QueryInterface(IID_IPicture, (void**)&ipic);
                if (ipic) {
                    var = IPictureToQPixmap(ipic);
                    ipic->Release();
                } else {
                    var = QPixmap();
                }
            } else {
#ifdef QAX_SERVER
                IAxServerBase *iface = 0;
                if (disp && !hint.startsWith("IDispatch*"))
                    disp->QueryInterface(IID_IAxServerBase, (void**)&iface);
                if (iface) {
                    QObject *qObj = iface->qObject();
                    iface->Release();
                    var = QVariant::UserData(qObj, qObj->className());                    
                } else
#endif
                    var = QVariant::UserData(disp, hint.latin1());
            }
        }
        break;
    case VT_UNKNOWN:
    case VT_UNKNOWN|VT_BYREF:
        {
            IUnknown *unkn = 0;
            if (arg.vt & VT_BYREF)
                unkn = *arg.ppunkVal;
            else
                unkn = arg.punkVal;
            var = QVariant(QVariant::UserData(unkn, "IUnknown*"));
        }
        break;
    case VT_ARRAY|VT_VARIANT:
    case VT_ARRAY|VT_VARIANT|VT_BYREF:
        {
            SAFEARRAY *array = 0;
            if (arg.vt & VT_BYREF)
                array = *arg.pparray;
            else
                array = arg.parray;
            
            QList<QVariant> list;
            if (!array || array->cDims != 1) {
                var = list;
                break;
            }
            
            long lBound, uBound;
            SafeArrayGetLBound(array, 1, &lBound);
            SafeArrayGetUBound(array, 1, &uBound);
            
            for (long i = lBound; i <= uBound; ++i) {
                VARIANT var;
                VariantInit(&var);
                SafeArrayGetElement(array, &i, &var);
                
                QVariant qvar = VARIANTToQVariant(var, 0);
                clearVARIANT(&var);
                list << qvar;
            }
            
            var = list;
        }
        break;
        
    case VT_ARRAY|VT_BSTR:
    case VT_ARRAY|VT_BSTR|VT_BYREF:
        {
            SAFEARRAY *array = 0;
            if (arg.vt & VT_BYREF)
                array = *arg.pparray;
            else
                array = arg.parray;
            
            QStringList strings;
            if (!array || array->cDims != 1) {
                var = strings;
                break;
            }
            
            long lBound, uBound;
            SafeArrayGetLBound(array, 1, &lBound);
            SafeArrayGetUBound(array, 1, &uBound);
            
            for (long i = lBound; i <= uBound; ++i) {
                BSTR bstr;
                SafeArrayGetElement(array, &i, &bstr);
                QString str = BSTRToQString(bstr);
                strings << str;
            }
            
            var = strings;
        }
        break;
        
    case VT_ARRAY|VT_UI1:
    case VT_ARRAY|VT_UI1|VT_BYREF:
        {
            SAFEARRAY *array = 0;
            if (arg.vt & VT_BYREF)
                array = *arg.pparray;
            else
                array = arg.parray;
            
            QByteArray bytes;
            if (!array || array->cDims != 1) {
                var = bytes;
                break;
            }
            
            long lBound, uBound;
            SafeArrayGetLBound(array, 1, &lBound);
            SafeArrayGetUBound(array, 1, &uBound);
            
            if (uBound != -1) { // non-empty array
                bytes.resize(uBound - lBound + 1);
                char *data = bytes.data();
                char *src;
                SafeArrayAccessData(array, (void**)&src);
                memcpy(data, src, bytes.size());
                SafeArrayUnaccessData(array);
            }
            
            var = bytes;
        }
        break;
        
#if defined(QAX_SERVER)
    case VT_RECORD:
    case VT_RECORD|VT_BYREF:
        if (arg.pvRecord && arg.pRecInfo) {
            IRecordInfo *recordInfo = arg.pRecInfo;
            void *record = arg.pvRecord;
            GUID guid;
            recordInfo->GetGuid(&guid);
            
            if (guid == CLSID_QRect) {
                QRect qrect;
                recordInfo->RecordCopy(record, &qrect);
                var = qrect;
            } else if (guid == CLSID_QSize) {
                QSize qsize;
                recordInfo->RecordCopy(record, &qsize);
                var = qsize;
            } else if (guid == CLSID_QPoint) {
                QPoint qpoint;
                recordInfo->RecordCopy(record, &qpoint);
                var = qpoint;
            }
        }
        break;
#endif // QAX_SERVER
    default:
        break;
    }
    
    QVariant::Type proptype = (!hint.isEmpty()) ? QVariant::nameToType(hint.latin1()) : QVariant::Invalid;
    if (proptype != QVariant::Invalid && var.type() != proptype) {
        if (var.canCast(proptype)) {
            var.cast(proptype);
        } else if (proptype == QVariant::StringList && var.type() == QVariant::List) {
            bool allStrings = true;
            QStringList strings;
            const QList<QCoreVariant> list(var.toList());
            for (QList<QCoreVariant>::ConstIterator it(list.begin()); it != list.end(); ++it) {
                QVariant variant = *it;
                if (variant.canCast(QVariant::String))
                    strings << variant.toString();
                else
                    allStrings = false;
            }
            if (allStrings)
                var = strings;
        }
    }
    return var;
}

void clearVARIANT(VARIANT *var)
{
    if (var->vt & VT_BYREF) {
        switch(var->vt) {
        case VT_BSTR|VT_BYREF:
            SysFreeString(*var->pbstrVal);
            delete var->pbstrVal;
            break;
        case VT_BOOL|VT_BYREF:
            delete var->pboolVal;
            break;
        case VT_I1|VT_BYREF:
            delete var->pcVal;
            break;
        case VT_I2|VT_BYREF:
            delete var->piVal;
            break;
        case VT_I4|VT_BYREF:
            delete var->plVal;
            break;
        case VT_INT|VT_BYREF:
            delete var->pintVal;
            break;
        case VT_UI1|VT_BYREF:
            delete var->pbVal;
            break;
        case VT_UI2|VT_BYREF:
            delete var->puiVal;
            break;
        case VT_UI4|VT_BYREF:
            delete var->pulVal;
            break;
        case VT_UINT|VT_BYREF:
            delete var->puintVal;
            break;
        case VT_CY|VT_BYREF:
            delete var->pcyVal;
            break;
        case VT_R4|VT_BYREF:
            delete var->pfltVal;
            break;
        case VT_R8|VT_BYREF:
            delete var->pdblVal;
            break;
        case VT_DATE|VT_BYREF:
            delete var->pdate;
            break;
        case VT_DISPATCH|VT_BYREF:
            (*var->ppdispVal)->Release();
            delete var->ppdispVal;
            break;
        case VT_ARRAY|VT_VARIANT|VT_BYREF:
        case VT_ARRAY|VT_UI1|VT_BYREF:
        case VT_ARRAY|VT_BSTR|VT_BYREF:
            SafeArrayDestroy(*var->pparray);
            delete var->pparray;
            break;
        case VT_VARIANT|VT_BYREF:
            delete var->pvarVal;
            break;
        }
        VariantInit(var);
    } else {
        VariantClear(var);
    }
}
