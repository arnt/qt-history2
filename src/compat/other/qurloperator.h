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

#ifndef QURLOPERATOR_H
#define QURLOPERATOR_H

#include "qobject.h"
#include "q3url.h"
#include "qptrlist.h"
#include "qnetworkprotocol.h"
#include "qstringlist.h" // QString->QStringList conversion

#ifndef QT_NO_NETWORKPROTOCOL

class QUrlInfo;
class QUrlOperatorPrivate;

class Q_COMPAT_EXPORT QUrlOperator : public QObject, public Q3Url
{
    friend class QNetworkProtocol;

    Q_OBJECT

public:
    QUrlOperator();
    QUrlOperator(const QString &urL);
    QUrlOperator(const QUrlOperator& url);
    QUrlOperator(const QUrlOperator& url, const QString& relUrl, bool checkSlash = false);
    virtual ~QUrlOperator();

    virtual void setPath(const QString& path);
    virtual bool cdUp();

    virtual const QNetworkOperation *listChildren();
    virtual const QNetworkOperation *mkdir(const QString &dirname);
    virtual const QNetworkOperation *remove(const QString &filename);
    virtual const QNetworkOperation *rename(const QString &oldname, const QString &newname);
    virtual const QNetworkOperation *get(const QString &location = QString::null);
    virtual const QNetworkOperation *put(const QByteArray &data, const QString &location = QString::null );
    virtual QPtrList<QNetworkOperation> copy(const QString &from, const QString &to, bool move = false, bool toPath = true);
    virtual void copy(const QStringList &files, const QString &dest, bool move = false);
    virtual bool isDir(bool *ok = 0);

    virtual void setNameFilter(const QString &nameFilter);
    QString nameFilter() const;

    virtual QUrlInfo info(const QString &entry) const;

    QUrlOperator& operator=(const QUrlOperator &url);
    QUrlOperator& operator=(const QString &url);

    virtual void stop();

signals:
    void newChildren(const QList<QUrlInfo> &, QNetworkOperation *res);
    void finished(QNetworkOperation *res);
    void start(QNetworkOperation *res);
    void createdDirectory(const QUrlInfo &, QNetworkOperation *res);
    void removed(QNetworkOperation *res);
    void itemChanged(QNetworkOperation *res);
    void data(const QByteArray &, QNetworkOperation *res);
    void dataTransferProgress(int bytesDone, int bytesTotal, QNetworkOperation *res);
    void startedNextCopy(const QPtrList<QNetworkOperation> &lst);
    void connectionStateChanged(int state, const QString &data);

protected:
    void reset();
    bool parse(const QString& url);
    virtual bool checkValid();
    virtual void clearEntries();
    void getNetworkProtocol();
    void deleteNetworkProtocol();

private slots:
    const QNetworkOperation *startOperation(QNetworkOperation *op);
    void copyGotData(const QByteArray &data, QNetworkOperation *op);
    void continueCopy(QNetworkOperation *op);
    void finishedCopy();
    void addEntry(const QList<QUrlInfo> &i);
    void slotItemChanged(QNetworkOperation *op);

private:
    void deleteOperation(QNetworkOperation *op);

    QUrlOperatorPrivate *d;
};

#endif // QT_NO_NETWORKPROTOCOL

#endif // QURLOPERATOR_H
