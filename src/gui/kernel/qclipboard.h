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

#ifndef QCLIPBOARD_H
#define QCLIPBOARD_H

#include "qwindowdefs.h"
#include "qobject.h"

#ifndef QT_NO_CLIPBOARD

class QMimeSource;

class Q_GUI_EXPORT QClipboard : public QObject
{
    Q_OBJECT
private:
    QClipboard(QObject *parent);
    ~QClipboard();

public:
    enum Mode { Clipboard, Selection };

    void        clear(Mode mode = Clipboard);

    bool        supportsSelection() const;
    bool        ownsSelection() const;
    bool        ownsClipboard() const;

    QString     text(Mode mode = Clipboard)         const;
    QString     text(QString& subtype, Mode mode = Clipboard) const;
    void        setText(const QString &, Mode mode = Clipboard);

#ifndef QT_NO_MIMECLIPBOARD
    QMimeSource *data(Mode mode  = Clipboard) const;
    void setData(QMimeSource*, Mode mode  = Clipboard);

    QImage        image(Mode mode  = Clipboard) const;
    QPixmap        pixmap(Mode mode  = Clipboard) const;
    void        setImage(const QImage &, Mode mode  = Clipboard);
    void        setPixmap(const QPixmap &, Mode mode  = Clipboard);
#endif

signals:
    void        selectionChanged();
    void        dataChanged();

private slots:
    void        ownerDestroyed();

protected:
    void        connectNotify(const char *);
    bool        event(QEvent *);

    friend class QApplication;
    friend class QApplicationPrivate;
    friend class QBaseApplication;
    friend class QDragManager;
    friend class QMimeSource;

private:
    Q_DISABLE_COPY(QClipboard)

#if defined(Q_WS_MAC)
    void loadScrap(bool convert);
    void saveScrap();
#endif
};

#endif // QT_NO_CLIPBOARD

#endif // QCLIPBOARD_H
