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

#ifndef QWSCOMMAND_QWS_H
#define QWSCOMMAND_QWS_H

#include "QtCore/qbytearray.h"
#include "QtGui/qwsutils_qws.h"

#include "QtGui/qfont.h"
#include "QtCore/qdatastream.h"

#include "QtCore/qvariant.h"

#define QTE_PIPE "QtEmbedded-%1"

class QRect;

/*********************************************************************
 *
 * Functions to read/write commands on/from a socket
 *
 *********************************************************************/
#ifndef QT_NO_QWS_MULTIPROCESS
void qws_write_command(QWSSocket *socket, int type, char *simpleData, int simpleLen, char *rawData, int rawLen);
bool qws_read_command(QWSSocket *socket, char *&simpleData, int &simpleLen, char *&rawData, int &rawLen, int &bytesRead);
#endif
/*********************************************************************
 *
 * QWSCommand base class - only use derived classes from that
 *
 *********************************************************************/


struct QWSProtocolItem
{
    // ctor - dtor
    QWSProtocolItem(int t, int len, char *ptr) : type(t),
        simpleLen(len), rawLen(-1), deleteRaw(false), simpleDataPtr(ptr),
        rawDataPtr(0), bytesRead(0) { }
    virtual ~QWSProtocolItem();


    // data
    int type;
    int simpleLen;
    int rawLen;
    bool deleteRaw;

    // functions
#ifndef QT_NO_QWS_MULTIPROCESS
    void write(QWSSocket *s);
    bool read(QWSSocket *s);
#endif
    void copyFrom(const QWSProtocolItem *item);

    virtual void setData(const char *data, int len, bool allocateMem = true);

    char *simpleDataPtr;
    char *rawDataPtr;
    // temp variables
    int bytesRead;
};


struct QWSCommand : QWSProtocolItem
{
    QWSCommand(int t, int len, char *ptr) : QWSProtocolItem(t,len,ptr) {}

    enum Type {
        Unknown = 0,
        Create,
        Destroy,
        Region,
        RegionMove,
        RegionDestroy,
        SetProperty,
        AddProperty,
        RemoveProperty,
        GetProperty,
        SetSelectionOwner,
        ConvertSelection,
        RequestFocus,
        ChangeAltitude,
        SetOpacity,
        DefineCursor,
        SelectCursor,
        PositionCursor,
        GrabMouse,
        PlaySound,
        QCopRegisterChannel,
        QCopSend,
        RegionName,
        Identify,
        GrabKeyboard,
        RepaintRegion,
        IMMouse,
        IMUpdate,
        IMResponse
    };
    static QWSCommand *factory(int type);
};

#ifndef QT_NO_DEBUG
class QDebug;
QDebug &operator<<(QDebug &dbg, QWSCommand::Type tp);
#endif // QT_NO_DEBUG

/*********************************************************************
 *
 * Commands
 *
 *********************************************************************/

struct QWSIdentifyCommand : public QWSCommand
{
    QWSIdentifyCommand() :
        QWSCommand(QWSCommand::Identify,
                   sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem) {
        QWSCommand::setData(d, len, allocateMem);
        id = QString(reinterpret_cast<const QChar*>(d), simpleData.idLen);
    }

    void setId(const QString& i)
    {
        id = i;
        simpleData.idLen = id.length();
        setData(reinterpret_cast<const char*>(id.unicode()), simpleData.idLen*2, true);
    }

    struct SimpleData {
        int idLen;
    } simpleData;
    QString id;
};

struct QWSCreateCommand : public QWSCommand
{
    QWSCreateCommand() :
        QWSCommand(QWSCommand::Create, 0, 0) {}
};

struct QWSRegionNameCommand : public QWSCommand
{
    QWSRegionNameCommand() :
        QWSCommand(QWSCommand::RegionName,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem) {
        QWSCommand::setData(d, len, allocateMem);
        name = QString(reinterpret_cast<const QChar*>(d), simpleData.nameLen/2);
        d += simpleData.nameLen;
        caption = QString(reinterpret_cast<const QChar*>(d), simpleData.captionLen/2);
    }

    void setName(const QString& n, const QString &c)
    {
        name = n;
        caption = c;
        int l = simpleData.nameLen = name.length()*2;
        l += simpleData.captionLen = caption.length()*2;
        char *d = new char[l];
        memcpy(d, name.unicode(), simpleData.nameLen);
        memcpy(d+simpleData.nameLen, caption.unicode(), simpleData.captionLen);
        setData(d, l, true);
        delete[] d;
    }

    struct SimpleData {
        int windowid;
        int nameLen;
        int captionLen;
    } simpleData;
    QString name;
    QString caption;
};

struct QWSRegionCommand : public QWSCommand
{
    QWSRegionCommand() :
        QWSCommand(QWSCommand::Region, sizeof(simpleData),
                    reinterpret_cast<char*>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSCommand::setData(d, len, allocateMem);
        rectangles = reinterpret_cast<QRect*>(rawDataPtr);
    }

    struct SimpleData {
        int windowid;
        int shmid;
        bool opaque;
        int nrectangles;
    } simpleData;

    QRect *rectangles;

};

struct QWSSetOpacityCommand : public QWSCommand
{
    QWSSetOpacityCommand() :
        QWSCommand(QWSCommand::SetOpacity, sizeof(simpleData),
                    reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        uchar opacity;
    } simpleData;
};

struct QWSRegionMoveCommand : public QWSCommand
{
    QWSRegionMoveCommand() :
        QWSCommand(QWSCommand::RegionMove, sizeof(simpleData),
                    reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        int dx;
        int dy;
    } simpleData;

};

struct QWSRegionDestroyCommand : public QWSCommand
{
    QWSRegionDestroyCommand() :
        QWSCommand(QWSCommand::RegionDestroy, sizeof(simpleData),
                    reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid;
    } simpleData;

};

struct QWSRequestFocusCommand : public QWSCommand
{
    QWSRequestFocusCommand() :
        QWSCommand(QWSCommand::RequestFocus, sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        int flag;
    } simpleData;
};

struct QWSChangeAltitudeCommand : public QWSCommand
{
    QWSChangeAltitudeCommand() :
        QWSCommand(QWSCommand::ChangeAltitude, sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        int altitude;
        bool fixed;
    } simpleData;

};


struct QWSAddPropertyCommand : public QWSCommand
{
    QWSAddPropertyCommand() :
        QWSCommand(QWSCommand::AddProperty, sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid, property;
    } simpleData;

};

struct QWSSetPropertyCommand : public QWSCommand
{
    QWSSetPropertyCommand() :
        QWSCommand(QWSCommand::SetProperty, sizeof(simpleData),
                    reinterpret_cast<char*>(&simpleData)) { data = 0; }

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSCommand::setData(d, len, allocateMem);
        data = rawDataPtr;
    }

    struct SimpleData {
        int windowid, property, mode;
    } simpleData;

    char *data;
};

struct QWSRepaintRegionCommand : public QWSCommand
{
    QWSRepaintRegionCommand() :
        QWSCommand(QWSCommand::RepaintRegion, sizeof(simpleData),
                    reinterpret_cast<char*>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSCommand::setData(d, len, allocateMem);
        rectangles = reinterpret_cast<QRect *>(rawDataPtr);
    }

    struct SimpleData {
        int windowid;
        bool opaque;
        int nrectangles;
    } simpleData;

    QRect * rectangles;

};

struct QWSRemovePropertyCommand : public QWSCommand
{
    QWSRemovePropertyCommand() :
        QWSCommand(QWSCommand::RemoveProperty, sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid, property;
    } simpleData;

};

struct QWSGetPropertyCommand : public QWSCommand
{
    QWSGetPropertyCommand() :
        QWSCommand(QWSCommand::GetProperty, sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid, property;
    } simpleData;

};

struct QWSSetSelectionOwnerCommand : public QWSCommand
{
    QWSSetSelectionOwnerCommand() :
        QWSCommand(QWSCommand::SetSelectionOwner,
                    sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        int hour, minute, sec, ms; // time
    } simpleData;

};

struct QWSConvertSelectionCommand : public QWSCommand
{
    QWSConvertSelectionCommand() :
        QWSCommand(QWSCommand::ConvertSelection,
                    sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int requestor; // requestor window of the selection
        int selection; // property on requestor into which the selection should be stored
        int mimeTypes; // property ion requestor in which the mimetypes, in which the selection may be, are stored
    } simpleData;

};

struct QWSDefineCursorCommand : public QWSCommand
{
    QWSDefineCursorCommand() :
        QWSCommand(QWSCommand::DefineCursor,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSCommand::setData(d, len, allocateMem);
        data = reinterpret_cast<unsigned char *>(rawDataPtr);
    }

    struct SimpleData {
        int width;
        int height;
        int hotX;
        int hotY;
        int id;
    } simpleData;

    unsigned char *data;
};

struct QWSSelectCursorCommand : public QWSCommand
{
    QWSSelectCursorCommand() :
        QWSCommand(QWSCommand::SelectCursor,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        int id;
    } simpleData;
};

struct QWSPositionCursorCommand : public QWSCommand
{
    QWSPositionCursorCommand() :
        QWSCommand(QWSCommand::PositionCursor,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    struct SimpleData {
        int newX;
        int newY;
    } simpleData;
};

struct QWSGrabMouseCommand : public QWSCommand
{
    QWSGrabMouseCommand() :
        QWSCommand(QWSCommand::GrabMouse,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        bool grab;  // grab or ungrab?
    } simpleData;
};

struct QWSGrabKeyboardCommand : public QWSCommand
{
    QWSGrabKeyboardCommand() :
        QWSCommand(QWSCommand::GrabKeyboard,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        bool grab;  // grab or ungrab?
    } simpleData;
};

#ifndef QT_NO_SOUND
struct QWSPlaySoundCommand : public QWSCommand
{
    QWSPlaySoundCommand() :
        QWSCommand(QWSCommand::PlaySound,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem) {
        QWSCommand::setData(d, len, allocateMem);
        filename = QString(reinterpret_cast<QChar*>(rawDataPtr),len/2);
    }
    void setFileName(const QString& n)
    {
        setData(reinterpret_cast<const char*>(n.unicode()), n.length()*2, true);
    }

    struct SimpleData {
        int windowid;
    } simpleData;
    QString filename;
};
#endif


#ifndef QT_NO_COP
struct QWSQCopRegisterChannelCommand : public QWSCommand
{
    QWSQCopRegisterChannelCommand() :
        QWSCommand(QWSCommand::QCopRegisterChannel,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem) {
        QWSCommand::setData(d, len, allocateMem);
        channel = QString(reinterpret_cast<const QChar*>(d), simpleData.chLen);
    }

    void setChannel(const QString& n)
    {
        channel = n;
        simpleData.chLen = channel.length();
        setData(reinterpret_cast<const char*>(channel.unicode()), simpleData.chLen*2, true);
    }

    struct SimpleData {
        int chLen;
    } simpleData;
    QString channel;
};

struct QWSQCopSendCommand : public QWSCommand
{
    QWSQCopSendCommand() :
        QWSCommand(QWSCommand::QCopSend,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem) {
        QWSCommand::setData(d, len, allocateMem);
        const QChar *cd = reinterpret_cast<const QChar*>(d);
        channel = QString(cd,simpleData.clen); cd += simpleData.clen;
        message = QString(cd,simpleData.mlen);
        d += simpleData.clen*sizeof(QChar) + simpleData.mlen*sizeof(QChar);
        data = QByteArray(d, simpleData.dlen);
    }

    void setMessage(const QString &c, const QString &m,
                     const QByteArray &data)
    {
        simpleData.clen = c.length();
        simpleData.mlen = m.length();
        simpleData.dlen = data.size();
        int l = simpleData.clen*sizeof(QChar);
        l += simpleData.mlen*sizeof(QChar);
        l += simpleData.dlen;
        char *tmp = new char[l];
        char *d = tmp;
        memcpy(d, c.unicode(), simpleData.clen*sizeof(QChar));
        d += simpleData.clen*sizeof(QChar);
        memcpy(d, m.unicode(), simpleData.mlen*sizeof(QChar));
        d += simpleData.mlen*sizeof(QChar);
        memcpy(d, data.data(), simpleData.dlen);
        setData(tmp, l, true);
        delete[] tmp;
    }

    struct SimpleData {
        int clen;
        int mlen;
        int dlen;
    } simpleData;
    QString channel;
    QString message;
    QByteArray data;
};

#endif


#ifndef QT_NO_QWS_IM

struct QWSIMMouseCommand : public QWSCommand
{
    QWSIMMouseCommand() :
        QWSCommand(QWSCommand::IMMouse,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        int state;
        int index;
    } simpleData;
};


struct QWSIMResponseCommand : public QWSCommand
{
    QWSIMResponseCommand() :
        QWSCommand(QWSCommand::IMResponse,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem) {
        QWSCommand::setData(d, len, allocateMem);

        QByteArray tmp = QByteArray::fromRawData(d, len);
        QDataStream s(tmp);
        s >> result;
    }

    void setResult(const QVariant & v)
    {
        QByteArray tmp;
        QDataStream s(&tmp, QIODevice::WriteOnly);
        s << v;
        setData(tmp.data(), tmp.size(), true);
    }

    struct SimpleData {
        int windowid;
        int property;
    } simpleData;

    QVariant result;
};

struct QWSIMUpdateCommand: public QWSCommand
{
    QWSIMUpdateCommand() :
        QWSCommand(QWSCommand::IMUpdate,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    enum UpdateType {Update, FocusIn, FocusOut, Reset, Destroyed};
    struct SimpleData {
        int windowid;
        int type;
        int widgetid;
    } simpleData;
};

#endif

#endif // QWSCOMMAND_QWS_H
