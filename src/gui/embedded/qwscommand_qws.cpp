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

#include "qwscommand_qws.h"

// #define QWSCOMMAND_DEBUG // Uncomment to debug client/server communication

#ifdef QWSCOMMAND_DEBUG
# include <qdebug.h>
# include "qfile.h"
# include <ctype.h>

// QWSHexDump -[ start ]---------------------------------------------
# define QWSHEXDUMP_MAX 32
class QWSHexDump
{
public:

    QWSHexDump(const void *adress, int len, int wrapAt = 16)
        : wrap(wrapAt), dataSize(len)
    {
        init();
        data = reinterpret_cast<const char*>(adress);
        if (len < 0)
            dataSize = 0;
    }

    QWSHexDump(const char *str, int len = -1, int wrapAt = 16)
        : wrap(wrapAt), dataSize(len)
    {
        init();
        data = str;
        if (len == -1)
            dataSize = str ? strlen(str) : 0;
    }

    QWSHexDump(const QByteArray &array, int wrapAt = 16)
        : wrap(wrapAt)
    {
        init();
        data = array.data();
        dataSize = array.size();
    }

    // Sets a customized prefix for the hexdump
    void setPrefix(const char *str) { prefix = str; }

    // Sets number of bytes to cluster together
    void setClusterSize(uint num) { clustering = num; }

    // Output hexdump to a text stream
    void intoTextStream(QTextStream &strm) {
        outstrm = &strm;
        hexDump();
    }

    // Output hexdump to a QString
    QString toString();

protected:
    void init();
    void hexDump();
    void sideviewDump(int at);

private:
    uint wrap;
    uint clustering;
    uint dataSize;
    int dataWidth;
    const char *data;
    const char *prefix;
    bool dirty;

    char sideviewLayout[QWSHEXDUMP_MAX + 1];
    char sideview[15];

    QTextStream *outstrm;
};

void QWSHexDump::init()
{
    prefix = "> ";             // Standard line prefix
    clustering = 2;            // Word-size clustering by default
    if (wrap > QWSHEXDUMP_MAX) // No wider than QWSHexDump_MAX bytes
        wrap = QWSHEXDUMP_MAX;
}

void QWSHexDump::hexDump()
{
    *outstrm << "(" << dataSize << " bytes):\n" << prefix;
    sprintf(sideviewLayout, " [%%-%us]", wrap);
    dataWidth = (2 * wrap) + (wrap / clustering);

    dirty = false;
    uint wrapIndex = 0;
    for (uint i = 0; i < dataSize; i++) {
        uint c = static_cast<uchar>(data[i]);
        sideview[wrapIndex = i%wrap] = isprint(c) ? c : '.';

        if (wrapIndex && (wrapIndex % clustering == 0))
            *outstrm << " ";

        outstrm->setFieldWidth(2);
        outstrm->setPadChar('0');
        outstrm->setf(0, QTextStream::showbase);
        *outstrm << hex << c;
        dirty = true;

        if (wrapIndex == wrap-1) {
            sideviewDump(wrapIndex);
            wrapIndex = 0;
            if (i+1 < dataSize)
                *outstrm << endl << prefix;
        }

    }
    sideviewDump(wrapIndex);
}

void QWSHexDump::sideviewDump(int at)
{
    if (dirty) {
        dirty = false;
        ++at;
        sideview[at] = '\0';
        int currentWidth = (2 * at) + (at / clustering) - (at%clustering?0:1);
        int missing = qMax(dataWidth - currentWidth, 0);
        while (missing--)
            *outstrm << " ";

        *outstrm << " [";
        outstrm->setPadChar(' ');
        outstrm->setFieldWidth(wrap);
        outstrm->setf(QTextStream::left, QTextStream::adjustfield);
        *outstrm << sideview;
        *outstrm << "]";
    }
}

// Output hexdump to a QString
QString QWSHexDump::toString() {
    QString result;
    QTextStream strm(&result, QFile::WriteOnly);
    outstrm = &strm;
    hexDump();
    return result;
}

#ifndef QT_NO_DEBUG
QDebug &operator<<(QDebug &dbg, QWSHexDump *hd) {
    if (!hd)
        return dbg << "QWSHexDump(0x0)";
    QString result = hd->toString();
    dbg.nospace() << result;
    return dbg.space();
}

// GCC & Intel wont handle references here
QDebug operator<<(QDebug dbg, QWSHexDump hd) {
    return dbg << &hd;
}
#endif
// QWSHexDump -[ end ]-----------------------------------------------


QDebug &operator<<(QDebug &dbg, QWSCommand::Type tp)
{
    const char *typeStr;
    switch(tp) {
        case QWSCommand::Create:
            typeStr = "Create";
            break;
        case QWSCommand::Destroy:
            typeStr = "Destroy";
            break;
        case QWSCommand::Region:
            typeStr = "Region";
            break;
        case QWSCommand::RegionMove:
            typeStr = "RegionMove";
            break;
        case QWSCommand::RegionDestroy:
            typeStr = "RegionDestroy";
            break;
        case QWSCommand::SetProperty:
            typeStr = "SetProperty";
            break;
        case QWSCommand::AddProperty:
            typeStr = "AddProperty";
            break;
        case QWSCommand::RemoveProperty:
            typeStr = "RemoveProperty";
            break;
        case QWSCommand::GetProperty:
            typeStr = "GetProperty";
            break;
        case QWSCommand::SetSelectionOwner:
            typeStr = "SetSelectionOwner";
            break;
        case QWSCommand::ConvertSelection:
            typeStr = "ConvertSelection";
            break;
        case QWSCommand::RequestFocus:
            typeStr = "RequestFocus";
            break;
        case QWSCommand::ChangeAltitude:
            typeStr = "ChangeAltitude";
            break;
        case QWSCommand::SetOpacity:
            typeStr = "SetOpacity";
            break;
        case QWSCommand::DefineCursor:
            typeStr = "DefineCursor";
            break;
        case QWSCommand::SelectCursor:
            typeStr = "SelectCursor";
            break;
        case QWSCommand::PositionCursor:
            typeStr = "PositionCursor";
            break;
        case QWSCommand::GrabMouse:
            typeStr = "GrabMouse";
            break;
        case QWSCommand::PlaySound:
            typeStr = "PlaySound";
            break;
        case QWSCommand::QCopRegisterChannel:
            typeStr = "QCopRegisterChannel";
            break;
        case QWSCommand::QCopSend:
            typeStr = "QCopSend";
            break;
        case QWSCommand::RegionName:
            typeStr = "RegionName";
            break;
        case QWSCommand::Identify:
            typeStr = "Identify";
            break;
        case QWSCommand::GrabKeyboard:
            typeStr = "GrabKeyboard";
            break;
        case QWSCommand::RepaintRegion:
            typeStr = "RepaintRegion";
            break;
        case QWSCommand::IMMouse:
            typeStr = "IMMouse";
            break;
        case QWSCommand::IMUpdate:
            typeStr = "IMUpdate";
            break;
        case QWSCommand::IMResponse:
            typeStr = "IMResponse";
            break;
        case QWSCommand::Unknown:
        default:
            typeStr = "Unknown";
            break;
    }
    dbg << typeStr;
    return dbg.space();
};

#define N_EVENTS 18
const char * eventNames[N_EVENTS] =  {
        "NoEvent",
        "Connected",
        "Mouse", "Focus", "Key",
        "RegionModified",
        "Creation",
        "PropertyNotify",
        "PropertyReply",
        "SelectionClear",
        "SelectionRequest",
        "SelectionNotify",
        "MaxWindowRect",
        "QCopMessage",
        "WindowOperation",
        "IMEvent",
        "IMQuery",
        "IMInit"
    };

class QWSServer;
extern QWSServer *qwsServer;
#endif


/*********************************************************************
 *
 * Functions to read/write commands on/from a socket
 *
 *********************************************************************/

#ifndef QT_NO_QWS_MULTIPROCESS
void qws_write_command(QWSSocket *socket, int type, char *simpleData, int simpleLen,
                       char *rawData, int rawLen)
{
#ifdef QWSCOMMAND_DEBUG
    if (simpleLen) qDebug() << "simpleData " << QWSHexDump(simpleData, simpleLen);
    if (rawLen > 0) qDebug() << "rawData " << QWSHexDump(rawData, rawLen);
#endif
    qws_write_uint(socket, type);
    qws_write_uint(socket, rawLen == -1 ? 0 : rawLen);

    // Add total lenght of command here, allowing for later command expansion...
    // qws_write_uint(socket, rawLen == -1 ? 0 : rawLen);

    if (simpleData && simpleLen)
        socket->write(simpleData, simpleLen);

    if (rawLen && rawData)
        socket->write(rawData, rawLen);
}

bool qws_read_command(QWSSocket *socket, char *&simpleData, int &simpleLen,
                      char *&rawData, int &rawLen, int &bytesRead)
{
    if (rawLen == -1) {
        if (socket->bytesAvailable() < sizeof(rawLen))
            return false;
        rawLen = qws_read_uint(socket);
#ifdef QWSCOMMAND_DEBUG
        qDebug() << "qws_read_command rawLen " << rawLen;
#endif
    }

    if (!bytesRead) {
        if (simpleLen) {
            if (socket->bytesAvailable() < uint(simpleLen))
                return false;
            bytesRead = socket->read(simpleData, simpleLen);
#ifdef QWSCOMMAND_DEBUG
         if (simpleLen)
             qDebug() << "simpleData " << QWSHexDump(simpleData, bytesRead);
#endif
        } else {
            bytesRead = 1; // hack!
        }
// #ifdef QWSCOMMAND_DEBUG
//         qDebug() << "simpleLen " << simpleLen << ", bytesRead " << bytesRead;
//#endif
    }

    if (bytesRead) {
        if (!rawLen)
            return true;
        if (socket->bytesAvailable() < uint(rawLen))
            return false;
        rawData = new char[rawLen];
        bytesRead += socket->read(rawData, rawLen);
#ifdef QWSCOMMAND_DEBUG
        qDebug() << "rawData " << QWSHexDump(rawData, rawLen);
        //qDebug() << "==== bytesRead " << bytesRead;
#endif
        return true;
    }
    return false;
}
#endif

/*********************************************************************
 *
 * QWSCommand base class - only use derived classes from that
 *
 *********************************************************************/
QWSProtocolItem::~QWSProtocolItem() {
    if (deleteRaw)
        delete []rawDataPtr;
}

#ifndef QT_NO_QWS_MULTIPROCESS
void QWSProtocolItem::write(QWSSocket *s) {
#ifdef QWSCOMMAND_DEBUG
    if (!qwsServer)
        qDebug() << "QWSProtocolItem::write sending type " << static_cast<QWSCommand::Type>(type);
    else
        qDebug() << "QWSProtocolItem::write sending event " << (type < N_EVENTS ? eventNames[type] : "unknown");
#endif
    qws_write_command(s, type, simpleDataPtr, simpleLen, rawDataPtr, rawLen);
}

bool QWSProtocolItem::read(QWSSocket *s) {
#ifdef QWSCOMMAND_DEBUG
    if (qwsServer)
        qDebug() << "QWSProtocolItem::read reading type " << static_cast<QWSCommand::Type>(type);
    else
        //qDebug() << "QWSProtocolItem::read reading event " << (type < N_EVENTS ? eventNames[type] : "unknown");
        qDebug("QWSProtocolItem::read reading event %s", type < N_EVENTS ? eventNames[type] : "unknown");
#endif
    bool b = qws_read_command(s, simpleDataPtr, simpleLen, rawDataPtr, rawLen, bytesRead);
    if (b) {
        setData(rawDataPtr, rawLen, false);
        deleteRaw = true;
    }
    return b;
}
#endif // QT_NO_QWS_MULTIPROCESS

void QWSProtocolItem::copyFrom(const QWSProtocolItem *item) {
    if (this == item)
        return;
    simpleLen = item->simpleLen;
    memcpy(simpleDataPtr, item->simpleDataPtr, simpleLen);
    setData(item->rawDataPtr, item->rawLen);
}

void QWSProtocolItem::setData(const char *data, int len, bool allocateMem) {
    if (!data && !len) {
        rawDataPtr = 0;
        rawLen = 0;
        return;
    }
    if (len < 0)
        return;
    if (deleteRaw)
        delete [] rawDataPtr;
    if (allocateMem) {
        rawDataPtr = new char[len];
        if (data)
            memcpy(rawDataPtr, data, len);
        deleteRaw = true;
    } else {
        rawDataPtr = const_cast<char *>(data);
        deleteRaw = false;
    }
    rawLen = len;
}

QWSCommand *QWSCommand::factory(int type)
{
    QWSCommand *command = 0;
    switch (type) {
    case QWSCommand::Create:
        command = new QWSCreateCommand;
        break;
    case QWSCommand::Region:
        command = new QWSRegionCommand;
        break;
    case QWSCommand::RegionMove:
        command = new QWSRegionMoveCommand;
        break;
    case QWSCommand::RegionDestroy:
        command = new QWSRegionDestroyCommand;
        break;
    case QWSCommand::AddProperty:
        command = new QWSAddPropertyCommand;
        break;
    case QWSCommand::SetProperty:
        command = new QWSSetPropertyCommand;
        break;
    case QWSCommand::RemoveProperty:
        command = new QWSRemovePropertyCommand;
        break;
    case QWSCommand::GetProperty:
        command = new QWSGetPropertyCommand;
        break;
    case QWSCommand::SetSelectionOwner:
        command = new QWSSetSelectionOwnerCommand;
        break;
    case QWSCommand::RequestFocus:
        command = new QWSRequestFocusCommand;
        break;
    case QWSCommand::ChangeAltitude:
        command = new QWSChangeAltitudeCommand;
        break;
    case QWSCommand::SetOpacity:
        command = new QWSSetOpacityCommand;
        break;
    case QWSCommand::DefineCursor:
        command = new QWSDefineCursorCommand;
        break;
    case QWSCommand::SelectCursor:
        command = new QWSSelectCursorCommand;
        break;
    case QWSCommand::GrabMouse:
        command = new QWSGrabMouseCommand;
        break;
    case QWSCommand::GrabKeyboard:
        command = new QWSGrabKeyboardCommand;
        break;
#ifndef QT_NO_SOUND
    case QWSCommand::PlaySound:
        command = new QWSPlaySoundCommand;
        break;
#endif
#ifndef QT_NO_COP
    case QWSCommand::QCopRegisterChannel:
        command = new QWSQCopRegisterChannelCommand;
        break;
    case QWSCommand::QCopSend:
        command = new QWSQCopSendCommand;
        break;
#endif
    case QWSCommand::RegionName:
        command = new QWSRegionNameCommand;
        break;
    case QWSCommand::Identify:
        command = new QWSIdentifyCommand;
        break;
    case QWSCommand::RepaintRegion:
        command = new QWSRepaintRegionCommand;
        break;
#ifndef QT_NO_QWS_IM
    case QWSCommand::IMUpdate:
        command = new QWSIMUpdateCommand;
        break;

    case QWSCommand::IMMouse:
        command = new QWSIMMouseCommand;
        break;

    case QWSCommand::IMResponse:
        command = new QWSIMResponseCommand;
        break;
#endif
    case QWSCommand::PositionCursor:
        command = new QWSPositionCursorCommand;
        break;
    default:
        qWarning("QWSCommand::factory : Type error - got %08x!", type);
    }
    return command;
}
