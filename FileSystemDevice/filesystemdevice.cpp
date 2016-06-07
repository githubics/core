/*
* Copyright (C) 2016  Integrated Computer Solutions, Inc
* 54 B Middlesex Turnpike, Bedford, MA 01730
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#if ! defined QT_NO_DEBUG_OUTPUT
#define QT_NO_DEBUG_OUTPUT
#endif
#include <QDebug>

#include "filesystemdevice.h"
#include "indexingworker.h"

#include <QThread>

#if ! defined QT_NO_DEBUG_OUTPUT
#include <QTime>
QTime stopWatch;
#endif

FileSystemDevice::FileSystemDevice(QObject *parent)
    : MediaDeviceInterface(parent)
    , thread(0)
    , worker(0)
    , busy(false)
{
    thread = new QThread;
    worker = new IndexingWorker;
    connect(thread,&QThread::finished,worker,&QObject::deleteLater);
    connect(this,&FileSystemDevice::startIndexing,worker,&IndexingWorker::startIndexing);
    connect(worker,&IndexingWorker::indexingFinished,this,&FileSystemDevice::indexingFinished);
    worker->moveToThread(thread);
    thread->start();
}

FileSystemDevice::~FileSystemDevice()
{
    if (thread) {
        thread->quit();
        thread->deleteLater();
    }
}

void FileSystemDevice::updateMediaPlaylist(const QUrl & url)
{
    qDebug() << Q_FUNC_INFO << url << "busy=" << busy;
    if (!busy) { // only one indexing job at a time
        busy=true;
#if ! defined QT_NO_DEBUG_OUTPUT
        stopWatch.start();
#endif
        emit startIndexing(url);
    }
}

void FileSystemDevice::indexingFinished()
{
    busy=false;
    qDebug() << Q_FUNC_INFO << "busy=" << busy;
    QJsonObject mediaObject;
    // FIXME: the url is not known here currently -
    // we likely want this to become the identifier for the device though
    mediaObject.insert("DeviceUrl","DeviceUrl"/*url.url()*/);
    mediaObject.insert("AudioFile",worker->audioFiles());
    mediaObject.insert("VideoFile",worker->videoFiles());
#if ! defined QT_NO_DEBUG_OUTPUT
    qDebug() << Q_FUNC_INFO << "indexed"
             << worker->audioFiles().count() << "audiofiles"
             << worker->videoFiles().count() << "videofiles"
             << "finished in" << stopWatch.elapsed()/1000.f;
#endif
    emit mediaPlaylistUpdated(mediaObject);
}

const QJsonObject FileSystemDevice::getMediaPlaylist(const QUrl & url) const
{
    // NOTE: this makes a call across threads. It will block both threads.
    worker->startIndexing(url);

    // TODO: Define this object somewhere and just fill it here with
    // data.
    QJsonObject mediaObject;
    mediaObject.insert("DeviceUrl",url.url()); // this is likely obsolete
    mediaObject.insert("AudioFile",worker->audioFiles());
    mediaObject.insert("VideoFile",worker->videoFiles());
    return mediaObject;
}


#if 0
bool FileSystemDevice::saveJsonObjectToFile( QString fileName, QJsonObject mediaObject )
{
    QFile saveFile( fileName );

    if (!saveFile.open(QIODevice::WriteOnly)) {
        // TODO: Handle error if device write proptected
        qDebug() << Q_FUNC_INFO << "Couldn't save to file";
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonDocument saveDoc( mediaObject );
    int res = saveFile.write( saveDoc.toJson() );
    qDebug() << Q_FUNC_INFO << " Written: " << res << " bytes";

    return true;
}

QJsonObject FileSystemDevice::getMediaSourceListFromFile( QString fileName )
{
    QJsonObject res;
    QFile loadFile( fileName );

     if (!loadFile.open(QIODevice::ReadOnly)) {
         // TODO: Handle error properly
         qWarning("Couldn't open save file.");
         return res;
     }

     QByteArray saveData = loadFile.readAll();

     QJsonDocument loadDoc( QJsonDocument::fromJson(saveData) );

     res = loadDoc.object();

     return res;
}

#endif
