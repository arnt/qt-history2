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
#include "qmap.h"
#include "qmime.h"
#include "qdrag.h"
#include "qpixmap.h"
#include "qpoint.h"
#ifdef Q_WS_MAC
# include "private/qt_mac_p.h"
#endif

class QEventLoop;

class QDragPrivate : public QObjectPrivate
{
public:
    QWidget *source;
    QWidget *target;
    QMimeData *data;
    QPixmap pixmap;
    QPoint hotspot;
    Qt::DropActions possible_actions;
    Qt::DropAction executed_action;
    QMap<Qt::DropAction, QPixmap> customCursors;
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
    Qt::DropAction drag(QDrag *);

    void cancel(bool deleteSource = true);
    void move(const QPoint &);
    void drop();
    void updatePixmap();
    QWidget *source() const { return object ? object->d_func()->source : 0; }
    QDragPrivate *dragPrivate() const { return object ? object->d_func() : 0; }

    static QDragManager *self();
    Qt::DropAction defaultAction(Qt::DropActions possibleActions,
                                 Qt::KeyboardModifiers modifiers) const;

    QDrag *object;

    void updateCursor();

    bool beingCancelled;
    bool restoreCursor;
    bool willDrop;
    QEventLoop *eventLoop;

    QPixmap dragCursor(Qt::DropAction action) const;

    bool hasCustomDragCursors() const;

    QDropData *dropData;

    void emitActionChanged(Qt::DropAction newAction) { if (object) emit object->actionChanged(newAction); }
    void emitTargetChanged(QWidget *newTarget) { if (object) emit object->targetChanged(newTarget); }

#ifdef Q_WS_MAC
    static OSErr qt_mac_send_handler(FlavorType, void *, DragItemRef, DragRef); //qdnd_mac.cpp
#endif

private:
    QPixmap *pm_cursor;
    int n_cursor;

    static QDragManager *instance;
    Q_DISABLE_COPY(QDragManager)
};


#if defined(Q_WS_WIN)

class QOleDataObject : public IDataObject
{
public:
    explicit QOleDataObject(QMimeData *mimeData);

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
