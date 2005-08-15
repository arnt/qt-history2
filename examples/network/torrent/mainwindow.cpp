/****************************************************************************
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "addtorrentdialog.h"
#include "mainwindow.h"
#include "ratecontroller.h"
#include "torrentclient.h"

#include <QtGui>

// TorrentView extends QTreeWidget to allow drag and drop.
class TorrentView : public QTreeWidget
{
    Q_OBJECT
public:
    TorrentView(QWidget *parent = 0);

signals:
    void fileDropped(const QString &fileName);

protected:
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
};

// TorrentViewDelegate is used to draw the progress bars.
class TorrentViewDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    inline TorrentViewDelegate(MainWindow *mainWindow) : QItemDelegate(mainWindow) {}

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        if (index.column() != 2) {
            QItemDelegate::paint(painter, option, index);
            return;
        }

        // Set up a QStyleOptionProgressBar to precisely mimic the
        // environment of a progress bar.
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = option.rect;
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible = true;

        // Set the progress and text values of the style option.
        int progress = qobject_cast<MainWindow *>(parent())->clientForRow(index.row())->progress();
        progressBarOption.progress = progress < 0 ? 0 : progress;
        progressBarOption.text = QString().sprintf("%d%%", progressBarOption.progress);

        // Draw the progress bar onto the view.
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), quitDialog(0), saveChanges(false)
{
    // Initialize some static strings
    QStringList headers;
    headers << tr("Torrent") << tr("Seen / Known / Seeds") << tr("Progress")
            << tr("Download rate") << tr("Upload rate") << tr("Status");

    // Main torrent list
    torrentView = new TorrentView(this);
    torrentView->setItemDelegate(new TorrentViewDelegate(this));
    torrentView->setHeaderLabels(headers);
    torrentView->setSelectionBehavior(QAbstractItemView::SelectRows);
    torrentView->setAlternatingRowColors(true);
    torrentView->setRootIsDecorated(false);
    setCentralWidget(torrentView);

    // Set header resize modes and initial section sizes
    QFontMetrics fm = fontMetrics();
    QHeaderView *header = torrentView->header();
    header->setResizeMode(0, QHeaderView::Stretch);
    for (int i = headers.size() - 1; i >= 0; --i)
        header->resizeSection(i, fm.width("O" + headers.at(i) + "O"));

    // File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(QIcon(":/icons/bottom.png"), tr("Add &new torrent"), this, SLOT(addTorrent()));
    pauseActionMenu = fileMenu->addAction(QIcon(":/icons/player_pause.png"), tr("&Pause torrent"));
    removeActionMenu = fileMenu->addAction(QIcon(":/icons/player_stop.png"), tr("&Remove torrent"));
    fileMenu->addSeparator();
    fileMenu->addAction(QIcon(":/icons/exit.png"), tr("E&xit"), this, SLOT(close()));

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, SLOT(about()));
    helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));

    // Top toolbar
    QToolBar *topBar = new QToolBar(tr("Tools"));
    addToolBar(Qt::TopToolBarArea, topBar);
    topBar->addAction(QIcon(tr(":/icons/bottom.png")),tr("Add torrent"), this, SLOT(addTorrent()));
    removeActionTool = topBar->addAction(QIcon(tr(":/icons/player_stop.png")), tr("Remove torrent"));
    pauseActionTool = topBar->addAction(QIcon(tr(":/icons/player_pause.png")), tr("Pause torrent"));
    topBar->addSeparator();
    downActionTool = topBar->addAction(QIcon(tr(":/icons/1downarrow.png")), tr("Move down"));
    upActionTool = topBar->addAction(QIcon(tr(":/icons/1uparrow.png")), tr("Move up"));

    // Bottom toolbar
    QToolBar *bottomBar = new QToolBar(tr("Rate control"));
    addToolBar(Qt::BottomToolBarArea, bottomBar);
    downloadLimitSlider = new QSlider(Qt::Horizontal);
    downloadLimitSlider->setRange(0, 1000);
    bottomBar->addWidget(new QLabel(tr("Max download:")));
    bottomBar->addWidget(downloadLimitSlider);
    bottomBar->addWidget((downloadLimitLabel = new QLabel(tr("0 KB/s"))));
    downloadLimitLabel->setFixedSize(QSize(fm.width(tr("99999 KB/s")), fm.lineSpacing()));
    bottomBar->addSeparator();
    uploadLimitSlider = new QSlider(Qt::Horizontal);
    uploadLimitSlider->setRange(0, 1000);
    bottomBar->addWidget(new QLabel(tr("Max upload:")));
    bottomBar->addWidget(uploadLimitSlider);
    bottomBar->addWidget((uploadLimitLabel = new QLabel(tr("0 KB/s"))));
    uploadLimitLabel->setFixedSize(QSize(fm.width(tr("99999 KB/s")), fm.lineSpacing()));

    // Set up connections
    connect(torrentView, SIGNAL(itemSelectionChanged()),
            this, SLOT(setActionsEnabled()));
    connect(torrentView, SIGNAL(fileDropped(const QString &)),
            this, SLOT(acceptFileDrop(const QString &)));
    connect(uploadLimitSlider, SIGNAL(valueChanged(int)),
            this, SLOT(setUploadLimit(int)));
    connect(downloadLimitSlider, SIGNAL(valueChanged(int)),
            this, SLOT(setDownloadLimit(int)));
    connect(removeActionMenu, SIGNAL(triggered(bool)),
            this, SLOT(removeTorrent()));
    connect(removeActionTool, SIGNAL(triggered(bool)),
            this, SLOT(removeTorrent()));
    connect(pauseActionMenu, SIGNAL(triggered(bool)),
            this, SLOT(pauseTorrent()));
    connect(pauseActionTool, SIGNAL(triggered(bool)),
            this, SLOT(pauseTorrent()));
    connect(upActionTool, SIGNAL(triggered(bool)),
            this, SLOT(moveTorrentUp()));
    connect(downActionTool, SIGNAL(triggered(bool)),
            this, SLOT(moveTorrentDown()));

    // Load settings and start
    setWindowTitle(tr("Torrent Client"));
    setActionsEnabled();
    QMetaObject::invokeMethod(this, "loadSettings", Qt::QueuedConnection);
}

QSize MainWindow::sizeHint() const
{
    QSize desktopSize = QApplication::desktop()->size();
    return QSize(desktopSize.width() / 2, desktopSize.height() / 4);
}

const TorrentClient *MainWindow::clientForRow(int row) const
{
    // Return the client at the given row.
    return jobs.at(row).client;
}

int MainWindow::rowOfClient(TorrentClient *client) const
{
    // Return the row that displays this client's status, or -1 if the
    // client is not known.
    int row = 0;
    foreach (Job job, jobs) {
        if (job.client == client)
            return row;
        ++row;
    }
    return -1;
}

void MainWindow::loadSettings()
{
    // Load base settings (last working directory, upload/download limits).
    QSettings settings(QLatin1String("Trolltech"), QLatin1String("QTorrent"));
    lastDirectory = settings.value(QLatin1String("LastDirectory")).toString();
    if (lastDirectory.isEmpty())
        lastDirectory = QDir::currentPath();
    int up = settings.value(QLatin1String("UploadLimit")).toInt();
    int down = settings.value(QLatin1String("DownloadLimit")).toInt();
    uploadLimitSlider->setValue(up ? up : 170);
    downloadLimitSlider->setValue(down ? down : 550);

    // Resume all previous downloads.
    int size = settings.beginReadArray(QLatin1String("Torrents"));
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QByteArray resumeState = settings.value("resumeState").toByteArray();
        QString fileName = settings.value(QLatin1String("sourceFileName")).toString();
        QString dest = settings.value("destinationFolder").toString();

        if (addTorrent(fileName, dest, resumeState)) {
            TorrentClient *client = jobs.last().client;
            client->setDownloadedBytes(settings.value("downloadedBytes").toLongLong());
            client->setUploadedBytes(settings.value("uploadedBytes").toLongLong());
        }
    }
}

bool MainWindow::addTorrent()
{
    // Show the file dialog, let the user select what torrent to start downloading.
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a torrent file"),
                                                    lastDirectory,
                                                    tr("Torrents (*.torrent);;"
                                                       " All files (*.*)"));
    if (fileName.isEmpty())
        return false;
    lastDirectory = QFileInfo(fileName).absolutePath();

    // Show the "Add Torrent" dialog.
    AddTorrentDialog *addTorrentDialog = new AddTorrentDialog(this);
    addTorrentDialog->setTorrent(fileName);
    addTorrentDialog->deleteLater();
    if (!addTorrentDialog->exec())
        return false;

    // Add the torrent to our list of downloads
    addTorrent(fileName, addTorrentDialog->destinationFolder());
    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
    return true;
}

void MainWindow::removeTorrent()
{
    // Find the row of the current item, and find the torrent client
    // for that row.
    int row = torrentView->indexOfTopLevelItem(torrentView->currentItem());
    TorrentClient *client = jobs.at(row).client;

    // Stop the client.
    client->disconnect();
    connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
    client->stop();

    // Remove the row from the view.
    jobs.removeAt(row);
    delete torrentView->takeTopLevelItem(row);
    setActionsEnabled();

    saveSettings();
}

void MainWindow::torrentStopped()
{
    // Schedule the client for deletion.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    client->deleteLater();

    // If the quit dialog is shown, update its progress.
    if (quitDialog) {
        quitDialog->setValue(++jobsStopped);
        if (jobsStopped == jobsToStop)
            quitDialog->close();
    }
}

void MainWindow::torrentError(TorrentClient::Error)
{
    // Delete the client.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    jobs.removeAt(row);
    delete torrentView->takeTopLevelItem(row);
    client->deleteLater();

    // Display the warning.
    QMessageBox::warning(this, tr("Error"),
                         tr("An error occurred while downloading: %1").arg(client->errorString()),
                         tr("&OK"));
}

bool MainWindow::addTorrent(const QString &fileName, const QString &destinationFolder,
                            const QByteArray &resumeState)
{
    // Check if the torrent is already being downloaded.
    foreach (Job job, jobs) {
        if (job.torrentFileName == fileName) {
            QMessageBox::warning(this, tr("Already downloading"),
                                 tr("The torrent file you have selected is "
                                    "already being downloaded."));
            return false;
        }
    }

    // Create a new torrent client and attempt to parse the torrent data.
    TorrentClient *client = new TorrentClient(this);
    if (!client->setTorrent(fileName)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("The torrent file you have selected can not be opened."),
                             tr("&OK"));
        delete client;
        return false;
    }
    client->setDestinationFolder(destinationFolder);
    client->setDumpedState(resumeState);

    // Setup the client connections.
    connect(client, SIGNAL(stateChanged(TorrentClient::State)), this, SLOT(updateState(TorrentClient::State)));
    connect(client, SIGNAL(peerInfoUpdated()), this, SLOT(updatePeerInfo()));
    connect(client, SIGNAL(progressUpdated(int)), this, SLOT(updateProgress(int)));
    connect(client, SIGNAL(downloadRateUpdated(int)), this, SLOT(updateDownloadRate(int)));
    connect(client, SIGNAL(uploadRateUpdated(int)), this, SLOT(updateUploadRate(int)));
    connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
    connect(client, SIGNAL(error(TorrentClient::Error)), this, SLOT(torrentError(TorrentClient::Error)));

    // Add the client to the list of downloading jobs.
    Job job;
    job.client = client;
    job.torrentFileName = fileName;
    job.destinationDirectory = destinationFolder;
    jobs << job;

    // Create and add a row in the torrent view for this download.
    QTreeWidgetItem *item = new QTreeWidgetItem(torrentView);

    QString baseFileName = QFileInfo(fileName).fileName();
    if (baseFileName.toLower().endsWith(".torrent"))
        baseFileName.remove(baseFileName.size() - 8);

    item->setText(0, baseFileName);
    item->setText(1, tr("0 / 0 / ?"));
    item->setText(2, QLatin1String("0"));
    item->setText(3, QLatin1String("0.0 KB/s"));
    item->setText(4, QLatin1String("0.0 KB/s"));
    item->setText(5, tr("Idle"));
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(1, Qt::AlignHCenter);

    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
    client->start();
    return true;
}

void MainWindow::saveSettings()
{
    if (!saveChanges)
      return;
    saveChanges = false;

    // Prepare and reset the settings
    QSettings settings(QLatin1String("Trolltech"), QLatin1String("QTorrent"));
    settings.clear();

    settings.setValue(QLatin1String("LastDirectory"), lastDirectory);
    settings.setValue(QLatin1String("UploadLimit"), uploadLimitSlider->value());
    settings.setValue(QLatin1String("DownloadLimit"), downloadLimitSlider->value());

    // Store data on all known torrents
    settings.beginWriteArray(QLatin1String("Torrents"));
    for (int i = 0; i < jobs.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue(QLatin1String("sourceFileName"), jobs.at(i).torrentFileName);
        settings.setValue(QLatin1String("destinationFolder"), jobs.at(i).destinationDirectory);
        settings.setValue(QLatin1String("uploadedBytes"), jobs.at(i).client->uploadedBytes());
        settings.setValue(QLatin1String("downloadedBytes"), jobs.at(i).client->downloadedBytes());
        settings.setValue(QLatin1String("resumeState"), jobs.at(i).client->dumpedState());
    }
    settings.endArray();
    settings.sync();
}

void MainWindow::updateState(TorrentClient::State)
{
    // Update the state string whenever the client's state changes.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    QTreeWidgetItem *item = torrentView->topLevelItem(row);
    if (item)
        item->setText(5, client->stateString());
    setActionsEnabled();
}

void MainWindow::updatePeerInfo()
{
    // Update the number of connected, visited, seed and leecher peers.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);

    QTreeWidgetItem *item = torrentView->topLevelItem(row);
    item->setText(1, tr("%1 / %2 / %3").arg(client->connectedPeerCount())
                  .arg(client->visitedPeerCount()).arg(client->seedCount()));
}

void MainWindow::updateProgress(int percent)
{
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);

    // Update the progressbar.
    QTreeWidgetItem *item = torrentView->topLevelItem(row);
    if (item)
        item->setText(2, QString::number(percent));
}

void MainWindow::setActionsEnabled()
{
    // Find the view item and client for the current row, and update
    // the states of the actions.
    QTreeWidgetItem *item = torrentView->currentItem();
    TorrentClient *client = item ? jobs.at(torrentView->indexOfTopLevelItem(item)).client : 0;
    bool pauseEnabled = client && ((client->state() == TorrentClient::Paused)
                                       ||  (client->state() > TorrentClient::Preparing));

    removeActionTool->setEnabled(item != 0);
    removeActionMenu->setEnabled(item != 0);
    pauseActionTool->setEnabled(item != 0 && pauseEnabled);
    pauseActionMenu->setEnabled(item != 0 && pauseEnabled);

    if (client && client->state() == TorrentClient::Paused) {
        pauseActionMenu->setIcon(QIcon(":/icons/player_play.png"));
        pauseActionMenu->setText(tr("Resume torrent"));
        pauseActionTool->setIcon(QIcon(":/icons/player_play.png"));
        pauseActionTool->setText(tr("Resume torrent"));

    } else {
        pauseActionMenu->setIcon(QIcon(":/icons/player_pause.png"));
        pauseActionMenu->setText(tr("Pause torrent"));
        pauseActionTool->setIcon(QIcon(":/icons/player_pause.png"));
        pauseActionTool->setText(tr("Pause torrent"));
    }

    int row = torrentView->indexOfTopLevelItem(item);
    upActionTool->setEnabled(item && row != 0);
    downActionTool->setEnabled(item && row != jobs.size() - 1);
}

void MainWindow::updateDownloadRate(int bytesPerSecond)
{
    // Update the download rate.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    QString num;
    num.sprintf("%.1f KB/s", bytesPerSecond / 1024.0);
    torrentView->topLevelItem(row)->setText(3, num);

    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
}

void MainWindow::updateUploadRate(int bytesPerSecond)
{
    // Update the upload rate.
    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);
    QString num;
    num.sprintf("%.1f KB/s", bytesPerSecond / 1024.0);
    torrentView->topLevelItem(row)->setText(4, num);

    if (!saveChanges) {
        saveChanges = true;
        QTimer::singleShot(5000, this, SLOT(saveSettings()));
    }
}

void MainWindow::pauseTorrent()
{
    // Pause or unpause the current torrent.
    int row = torrentView->indexOfTopLevelItem(torrentView->currentItem());
    TorrentClient *client = jobs.at(row).client;
    client->setPaused(client->state() != TorrentClient::Paused);
    setActionsEnabled();
}

void MainWindow::moveTorrentUp()
{
    QTreeWidgetItem *item = torrentView->currentItem();
    int row = torrentView->indexOfTopLevelItem(item);
    if (row == 0)
        return;

    Job tmp = jobs.at(row - 1);
    jobs[row - 1] = jobs[row];
    jobs[row] = tmp;

    QTreeWidgetItem *itemAbove = torrentView->takeTopLevelItem(row - 1);
    torrentView->insertTopLevelItem(row, itemAbove);
    setActionsEnabled();
}

void MainWindow::moveTorrentDown()
{
    QTreeWidgetItem *item = torrentView->currentItem();
    int row = torrentView->indexOfTopLevelItem(item);
    if (row == jobs.size() - 1)
        return;

    Job tmp = jobs.at(row + 1);
    jobs[row + 1] = jobs[row];
    jobs[row] = tmp;

    QTreeWidgetItem *itemAbove = torrentView->takeTopLevelItem(row + 1);
    torrentView->insertTopLevelItem(row, itemAbove);
    setActionsEnabled();
}

static int rateFromValue(int value)
{
    int rate = 0;
    if (value >= 0 && value < 250) {
        rate = 1 + int(value * 0.124);
    } else if (value < 500) {
        rate = 32 + int((value - 250) * 0.384);
    } else if (value < 750) {
        rate = 128 + int((value - 500) * 1.536);
    } else {
        rate = 512 + int((value - 750) * 6.1445);
    }
    return rate;
}

void MainWindow::setUploadLimit(int value)
{
    int rate = rateFromValue(value);
    uploadLimitLabel->setText(tr("%1 KB/s").arg(QString().sprintf("%4d", rate)));
    RateController::instance()->setUploadLimit(rate * 1024);
}

void MainWindow::setDownloadLimit(int value)
{
    int rate = rateFromValue(value);
    downloadLimitLabel->setText(tr("%1 KB/s").arg(QString().sprintf("%4d", rate)));
    RateController::instance()->setDownloadLimit(rate * 1024);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Torrent"),
                       tr("The <b>Torrent</b> example demonstrates how to "
                          "write complete a complete peer-to-peer file sharing "
                          "application using Qt's network and thread classes."));
}

void MainWindow::acceptFileDrop(const QString &fileName)
{
    // Create and show the "Add Torrent" dialog.
    AddTorrentDialog *addTorrentDialog = new AddTorrentDialog;
    lastDirectory = QFileInfo(fileName).absolutePath();
    addTorrentDialog->setTorrent(fileName);
    addTorrentDialog->deleteLater();
    if (!addTorrentDialog->exec())
        return;

    // Add the torrent to our list of downloads.
    addTorrent(fileName, addTorrentDialog->destinationFolder());
    saveSettings();
}

void MainWindow::closeEvent(QCloseEvent *)
{
    if (jobs.isEmpty())
        return;

    // Save upload / download numbers.
    saveSettings();
    saveChanges = false;

    quitDialog = new QProgressDialog(tr("Disconnecting from trackers"), tr("Abort"), 0, jobsToStop, this);

    // Stop all clients, remove the rows from the view and wait for
    // them to signal that they have stopped.
    jobsToStop = 0;
    jobsStopped = 0;
    foreach (Job job, jobs) {
        ++jobsToStop;
        TorrentClient *client = job.client;
        client->disconnect();
        connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
        client->stop();
        delete torrentView->takeTopLevelItem(0);
    }

    if (jobsToStop > jobsStopped)
        quitDialog->exec();
    quitDialog->deleteLater();
    quitDialog = 0;
}

TorrentView::TorrentView(QWidget *parent)
    : QTreeWidget(parent)
{
    setAcceptDrops(true);
}

void TorrentView::dragMoveEvent(QDragMoveEvent *event)
{
    // Accept file actions with a '.torrent' extension.
    QUrl url(event->mimeData()->text());
    if (url.isValid() && url.scheme().toLower() == QLatin1String("file")
        && url.path().toLower().endsWith(".torrent"))
        event->acceptProposedAction();
}

void TorrentView::dropEvent(QDropEvent *event)
{
    // Accept drops if the file has a '.torrent' extension and it
    // exists.
    QString fileName = QUrl(event->mimeData()->text()).path();
    if (QFile::exists(fileName) && fileName.toLower().endsWith(".torrent"))
        emit fileDropped(fileName);
}

#include "mainwindow.moc"
