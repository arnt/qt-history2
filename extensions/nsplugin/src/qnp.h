#ifndef QNPBIND_H
#define QNPBIND_H

#include <qwidget.h>


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

    void requestRead(int offset, uint length);
    int write( int len, void* buffer );

    QNPInstance* instance() { return inst; }
    QNPStream(QNPInstance*,const char*,_NPStream*,bool);

private:
    QNPInstance* inst;
    _NPStream* stream;
    QString mtype;
    bool seek;
};

class QNPWidget : public QWidget {
    Q_OBJECT
public:
    QNPWidget();
    ~QNPWidget();

    void setWindow();
    void unsetWindow();

    virtual void enterInstance();
    virtual void leaveInstance();

    QNPInstance* instance();

private:
    WId saveWId;
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

    void getURL(const char* url, const char* window=0);
    void postURL(const char* url, const char* window,
	     uint len, const char* buf, bool file);

    QNPStream* newStream(const char* mimetype, const char* window,
	bool as_file);
    virtual void streamAsFile(QNPStream*, const char* fname);

    void* getJavaPeer() const;

protected:
    QNPInstance();

private:
    friend QNPStream;
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

    virtual void* getJavaClass() const;
    void* getJavaEnv() const;

protected:
    QNPlugin();
};



#endif
