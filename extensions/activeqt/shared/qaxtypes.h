/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TYPES_H
#define TYPES_H

#include <qcolor.h>
#include <qfont.h>
#include <qdatetime.h>
#include <qvariant.h>

extern GUID IID_IAxServerBase;
struct IAxServerBase : public IUnknown
{
    virtual IUnknown *clientSite() const = 0;
    virtual void emitPropertyChanged(const char*) = 0;
    virtual bool emitRequestPropertyChange(const char*) = 0;
    virtual QObject *qObject() const = 0;
};

#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   ((HIMETRIC_PER_INCH*(x) + ((ppli)>>1)) / (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   (((ppli)*(x) + HIMETRIC_PER_INCH/2) / HIMETRIC_PER_INCH)
#define QAX_NUM_PARAMS 8

static inline BSTR QStringToBSTR(const QString &str)
{
    return SysAllocStringLen((OLECHAR*)str.unicode(), str.length());
}

static inline uint QColorToOLEColor(const QColor &col)
{
    return qRgba(col.blue(), col.green(), col.red(), 0x00);
}

extern QColor OLEColorToQColor(uint col);

extern bool QVariantToVARIANT(const QVariant &var, VARIANT &arg, const QByteArray &typeName = 0, bool out = false);
extern QVariant VARIANTToQVariant(const VARIANT &arg, const QByteArray &typeName, uint type = 0);
extern bool QVariantToVoidStar(const QVariant &var, void *data, const QByteArray &typeName, uint type = 0);
extern void clearVARIANT(VARIANT *var);

#endif //TYPES_H
