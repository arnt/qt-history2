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

#ifndef QAXWIDGET_H
#define QAXWIDGET_H

#include "qaxbase.h"
#include <qwidget.h>

class QAxHostWindow;
class QAxAggregated;

class QAxWidget : public QWidget, public QAxBase
{
public:
    const QMetaObject *metaObject() const;
    void* qt_metacast(const char*);
    int qt_metacall(QMetaObject::Call, int, void **);
    QObject* qObject() const { return (QWidget*)this; }
    const char *className() const;
    
    QAxWidget(QWidget* parent = 0, Qt::WFlags f = 0);
    QAxWidget(const QString &c, QWidget *parent = 0, Qt::WFlags f = 0);
    QAxWidget(IUnknown *iface, QWidget *parent = 0, Qt::WFlags f = 0);
    ~QAxWidget();
    
    void clear();
    
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    virtual QAxAggregated *createAggregate();

protected:
    bool initialize(IUnknown**);
    virtual bool createHostWindow(bool);
    
    void enabledChange(bool old);
    void paletteChange(const QPalette &old);
    void fontChange(const QFont &old);
    void windowActivationChange(bool old);
    
    void resizeEvent(QResizeEvent *);
    virtual bool translateKeyEvent(int message, int keycode) const;
    virtual void setStatusText(const QString &text);

    void connectNotify(const char *signal);
private:
    friend class QAxHostWindow;
    QAxHostWindow *container;
    
    const QMetaObject *parentMetaObject() const;
    static QMetaObject staticMetaObject;
};

#if defined Q_CC_MSVC && _MSC_VER < 1300
template <> inline QAxWidget *qt_cast_helper<QAxWidget*>(const QObject *o, QAxWidget *)
#else
template <> inline QAxWidget *qt_cast<QAxWidget*>(const QObject *o)
#endif
{
    void *result = o ? const_cast<QObject *>(o)->qt_metacast("QAxWidget") : 0;
    return (QAxWidget*)(result);
}

#if defined Q_CC_MSVC && _MSC_VER < 1300
template <> inline QAxWidget *qt_cast_helper<QAxWidget*>(QObject *o, QAxWidget *)
#else
template <> inline QAxWidget *qt_cast<QAxWidget*>(QObject *o)
#endif
{
    void *result = o ? o->qt_metacast("QAxWidget") : 0;
    return (QAxWidget*)(result);
}

#endif // QAXWIDGET_H
