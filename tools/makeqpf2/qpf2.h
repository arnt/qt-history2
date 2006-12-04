#ifndef QPF2_H
#define QPF2_H

#include <private/qfontengine_qpf_p.h>

class QFontEngine;

class QPF
{
public:
    static int debugVerbosity;

    static QByteArray generate(QFontEngine *fontEngine, bool includeCMap = true);
    void addHeader(QFontEngine *fontEngine);
    void addCMap(QFontEngine *fontEngine);
    void addGlyphs(QFontEngine *fontEngine);
    void addBlock(QFontEngineQPF::BlockTag tag, const QByteArray &data);

    void addTaggedString(QFontEngineQPF::HeaderTag tag, const QByteArray &string);
    void addTaggedQFixed(QFontEngineQPF::HeaderTag tag, QFixed value);
    void addTaggedUInt8(QFontEngineQPF::HeaderTag tag, quint8 value);
    void addTaggedInt8(QFontEngineQPF::HeaderTag tag, qint8 value);
    void addTaggedUInt16(QFontEngineQPF::HeaderTag tag, quint16 value);
    void addTaggedUInt32(QFontEngineQPF::HeaderTag tag, quint32 value);

    static void dump(const QByteArray &qpf);
    static const uchar *dumpHeader(const uchar *data);
    static const uchar *dumpHeaderTag(const uchar *data);
    static const uchar *dumpBlock(const uchar *data);

    void addUInt16(quint16 value) { qToBigEndian(value, addBytes(sizeof(value))); }
    void addUInt32(quint32 value) { qToBigEndian(value, addBytes(sizeof(value))); }
    void addUInt8(quint8 value) { *addBytes(sizeof(value)) = value; }
    void addInt8(qint8 value) { *addBytes(sizeof(value)) = quint8(value); }
    void addByteArray(const QByteArray &string) {
        uchar *data = addBytes(string.length());
        qMemCopy(data, string.constData(), string.length());
    }

    void align4() { while (qpf.size() & 3) { addUInt8('\0'); } }

    uchar *addBytes(int size) {
        const int oldSize = qpf.size();
        qpf.resize(qpf.size() + size);
        return reinterpret_cast<uchar *>(qpf.data() + oldSize);
    }

    QByteArray qpf;
};

#endif // QPF2_H
