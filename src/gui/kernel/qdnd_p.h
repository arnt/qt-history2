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

#ifndef QDND_P_H
#define QDND_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qobject.h"
#include "private/qobject_p.h"
#include "qmime.h"
#include "qdrag.h"
#include "qpixmap.h"
#include "qpoint.h"

class QEventLoop;

class QDragPrivate : public QObjectPrivate
{
public:
    QWidget *source;
    QWidget *target;
    QMimeData *data;
    QPixmap pixmap;
    QPoint hotspot;
    QDrag::DropActions possible_actions;
    QDrag::DropAction executed_action;
};

class QDropData : public QMimeData
{
    Q_OBJECT
public:
    QDropData();
    ~QDropData();

    bool hasFormat(const QString &mimetype) const;
    QStringList formats() const;
protected:
    QVariant retrieveData(const QString &mimetype, QVariant::Type) const;

#if defined(Q_WS_WIN)
public:
    LPDATAOBJECT currentDataObject;
#endif
};

class Q_GUI_EXPORT QDragManager: public QObject {
    Q_OBJECT

    QDragManager();
    ~QDragManager();
    // only friend classes can use QDragManager.
    friend class QDrag;
    friend class QDragMoveEvent;
    friend class QDropEvent;
    friend class QApplication;
#ifdef Q_WS_MAC
    friend class QWidgetPrivate; //dnd is implemented here
#endif

    bool eventFilter(QObject *, QEvent *);
    void timerEvent(QTimerEvent*);

public:
    QDrag::DropAction drag(QDrag *);

    void cancel(bool deleteSource = true);
    void move(const QPoint &);
    void drop();
    void updatePixmap();
    QWidget *source() { return object ? object->d_func()->source : 0; }
    QDragPrivate *dragPrivate() { return object ? object->d_func() : 0; }
    
    static QDragManager *self();
    QDrag::DropAction defaultAction(QDrag::DropActions possibleActions) const;

    QDrag *object;
    
#if defined(Q_WS_QWS)
    void updateMode(Qt::KeyboardModifiers newstate);  // #### get rid of me
#endif
    void updateCursor();

    bool beingCancelled;
    bool restoreCursor;
    bool willDrop;
    QEventLoop *eventLoop;

    QPixmap *pm_cursor;
    int n_cursor;

    QDropData *dropData;

    void emitActionChanged(QDrag::DropAction newAction) { if (object) emit object->actionChanged(newAction); }
    void emitTargetChanged(QWidget *newTarget) { if (object) emit object->targetChanged(newTarget); }
private:
    static QDragManager *instance;
    Q_DISABLE_COPY(QDragManager)
};


#if defined(Q_WS_WIN)

class QOleDataObject : public IDataObject
{
public:
    QOleDataObject(QMimeData *mimeData);

    void releaseQt();
    const QMimeData *mimeData() const;
    DWORD reportedPerformedEffect() const;

    // IUnknown methods 
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    // IDataObject methods 
    STDMETHOD(GetData)(LPFORMATETC pformatetcIn,  LPSTGMEDIUM pmedium);
    STDMETHOD(GetDataHere)(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium);
    STDMETHOD(QueryGetData)(LPFORMATETC pformatetc);
    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
    STDMETHOD(SetData)(LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium,
                       BOOL fRelease);
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);
    STDMETHOD(DAdvise)(FORMATETC FAR* pFormatetc, DWORD advf,
                      LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
    STDMETHOD(DUnadvise)(DWORD dwConnection);
    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR* ppenumAdvise);

private:
    ULONG m_refs;
    QMimeData *data;
    int CF_PERFORMEDDROPEFFECT;
    DWORD performedEffect;
};

#endif

#endif // QDND_P_H
