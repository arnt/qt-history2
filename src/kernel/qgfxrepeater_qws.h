#ifndef QGFXREPEATER_QWS_H
#define QGFXREPEATER_QWS_H

#include "qgfx_qws.h"

#ifndef QT_NO_QWS_REPEATER

#include "qptrlist.h"

class QScreenRec;

class QRepeaterScreen : public QScreen
{
public:

    QRepeaterScreen(int);
    virtual ~QRepeaterScreen();

    virtual bool connect(const QString &);
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual bool initDevice();
    virtual void disconnect() {}
    virtual void setMode(int,int,int) {}
    virtual int initCursor(void *,bool=FALSE);
    virtual void setDirty(const QRect &);
    virtual int sharedRamSize(void *);
    QImage * readScreen(int,int,int,int,QRegion &);
    QRegion getRequiredUpdate(int,int,int,int,int,int);

private:

    bool sw_cursor_exists;

    QPtrList<QScreenRec> screens;

};

#endif // QT_NO_QWS_REPEATER

#endif // QGFXREPEATER_QWS_H
