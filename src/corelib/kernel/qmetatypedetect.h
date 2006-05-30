/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMETATYPEDETECT_H
#define QMETATYPEDETECT_H

#include <QtCore/qmetatype.h>

#ifdef QT_NO_DATASTREAM_DETECTION

namespace QtInternal
{
    template<typename T> inline QMetaType::SaveOperator getSaveOperator(T * = 0) { return 0; }
    template<typename T> inline QMetaType::LoadOperator getLoadOperator(T * = 0) { return 0; }
}

#else

#ifndef QT_NO_DATASTREAM

#include <QtCore/qdatastream.h>

namespace QtInternal
{
    struct EmptyStruct {};
    struct LowPreferenceStruct { LowPreferenceStruct(...); };

    EmptyStruct operator<<(QDataStream &, const LowPreferenceStruct &);
    EmptyStruct operator>>(QDataStream &, const LowPreferenceStruct &);

    template<typename T>
    struct DataStreamChecker
    {
        static EmptyStruct hasStreamHelper(const EmptyStruct &);
        static QDataStream hasStreamHelper(const QDataStream &);
        static QDataStream &dsDummy();
        static T &dummy();

#ifdef I_HAVE_A_BROKEN_COMPILER
        static const bool HasDataStream =
            sizeof(hasStreamHelper(dsDummy() << dummy())) == sizeof(QDataStream)
            && sizeof(hasStreamHelper(dsDummy() >> dummy())) == sizeof(QDataStream);
#else
        enum {
            HasOutDataStream = sizeof(hasStreamHelper(dsDummy() >> dummy())) == sizeof(QDataStream),
            HasInDataStream = sizeof(hasStreamHelper(dsDummy() << dummy())) == sizeof(QDataStream),
            HasDataStream = HasOutDataStream & HasInDataStream
        };
#endif
    };

    template<bool>
    struct DataStreamOpHelper
    {
        template <typename T>
        struct Getter {
            static QMetaType::SaveOperator saveOp() { return 0; }
            static QMetaType::LoadOperator loadOp() { return 0; }
        };
    };

    template<>
    struct DataStreamOpHelper<true>
    {
        template <typename T>
        struct Getter {
            static QMetaType::SaveOperator saveOp()
            {
                typedef void(*SavePtr)(QDataStream &, const T *);
                SavePtr op = ::qMetaTypeSaveHelper<T>;
                return reinterpret_cast<QMetaType::SaveOperator>(op);
            }
            static QMetaType::SaveOperator loadOp()
            {
                typedef void(*LoadPtr)(QDataStream &, T *);
                LoadPtr op = ::qMetaTypeLoadHelper<T>;
                return reinterpret_cast<QMetaType::LoadOperator>(op);
            }
        };

    };

    template<typename T>
    inline QMetaType::SaveOperator getSaveOperator(T * = 0)
    {
        typedef typename DataStreamOpHelper<DataStreamChecker<T>::HasDataStream>::template Getter<T> GetterHelper;
        return GetterHelper::saveOp();
    }
    template<typename T>
    inline QMetaType::LoadOperator getLoadOperator(T * = 0)
    {
        typedef typename DataStreamOpHelper<DataStreamChecker<T>::HasDataStream>::template Getter<T> GetterHelper;
        return GetterHelper::loadOp();
    }
};
#endif // QT_NO_DATASTREAM

#endif // QT_NO_DATASTREAM_DETECTION

#endif
