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

#ifndef QNP_H
#define QNP_H

#ifndef QT_H
#include "QtGui/qwidget.h"
#endif // QT_H


struct _NPInstance;
struct _NPStream;
class QNPInstance;

class QNPStream {
public:
    ~QNPStream();

    const char* url() const;
    uint end() const;
    uint lastModified() const;

    const char* type() const;
    bool seekable() const;
    bool okay() const;
    bool complete() const;

    void requestRead(int offset, uint length);
    int write( int len, void* buffer );

    QNPInstance* instance() { return inst; }
    QNPStream(QNPInstance*,const char*,_NPStream*,bool);
    void setOkay(bool);
    void setComplete(bool);

private:
    QNPInstance* inst;
    _NPStream* stream;
    QString mtype;
    int seek:1;
    int isokay:1;
    int iscomplete:1;
};

class QNPWidget : public QWidget {
    Q_OBJECT
public:
    QNPWidget();
    ~QNPWidget();
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);

    virtual void enterInstance();
    virtual void leaveInstance();

    QNPInstance* instance();

private:
    _NPInstance* pi;
};

class QNPInstance : public QObject {
    Q_OBJECT
public:
    ~QNPInstance();

    // Arguments passed to EMBED
    int argc() const;
    const char* argn(int) const;
    const char* argv(int) const;
    enum Reason {
        ReasonDone = 0,
        ReasonBreak = 1,
        ReasonError = 2,
        ReasonUnknown = -1
    };
    const char* arg(const char* name) const;
    enum InstanceMode { Embed=1, Full=2, Background=3 };
    InstanceMode mode() const;

    // The browser's name
    const char* userAgent() const;

    // Your window.
    virtual QNPWidget* newWindow();
    QNPWidget* widget();

    // Incoming streams (SRC=... tag).
    // Defaults ignore data.
    enum StreamMode { Normal=1, Seek=2, AsFile=3, AsFileOnly=4 };
    virtual bool newStreamCreated(QNPStream*, StreamMode& smode);
    virtual int writeReady(QNPStream*);
    virtual int write(QNPStream*, int offset, int len, void* buffer);
    virtual void streamDestroyed(QNPStream*);

    void status(const char* msg);
    void getURLNotify(const char* url, const char* window=0, void*data=0);

    void getURL(const char* url, const char* window=0);
    void postURL(const char* url, const char* window,
	     uint len, const char* buf, bool file);

    QNPStream* newStream(const char* mimetype, const char* window,
	bool as_file=FALSE);
    virtual void streamAsFile(QNPStream*, const char* fname);

    void* getJavaPeer() const;

    virtual void notifyURL(const char* url, Reason r, void* notifyData);
    virtual bool printFullPage();
    virtual void print(QPainter*);

protected:
    QNPInstance();

private:
    friend class QNPStream;
    _NPInstance* pi;
};


class QNPlugin {
public:
    // Write this to return your QNPlugin derived class.
    static QNPlugin* create();

    static QNPlugin* actual();

    virtual ~QNPlugin();

    void getVersionInfo(int& plugin_major, int& plugin_minor,
	     int& browser_major, int& browser_minor);

    virtual QNPInstance* newInstance()=0;
    virtual const char* getMIMEDescription() const=0;
    virtual const char* getPluginNameString() const=0;
    virtual const char* getPluginDescriptionString() const=0;

    virtual void* getJavaClass();
    virtual void unuseJavaClass();
    void* getJavaEnv() const;

protected:
    QNPlugin();
};


#endif  // QNP_H
