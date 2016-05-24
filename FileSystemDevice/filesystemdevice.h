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
#ifndef FILESYSTEMDEVICE_H
#define FILESYSTEMDEVICE_H

#include "../QIcsMediaManager/mediadeviceinterface.h"
#include <QUrl>
class QThread;
class IndexingWorker;

class FileSystemDevice : public MediaDeviceInterface
{
    Q_OBJECT

public:
    FileSystemDevice(QObject *parent = 0);
    ~FileSystemDevice();

    /** Asynchronous call to update the media playlist.
     *  When a list is available the signal mediaPlaylistUpdated()
     *  is emitted
     */
    void updateMediaPlaylist(const QUrl & url) Q_DECL_OVERRIDE;

    /** Synchronous call returns an updated source list */
    const QJsonObject getMediaPlaylist(const QUrl & url) const Q_DECL_OVERRIDE;

public slots:
    virtual void indexingFinished();

signals:
    /** This signal is emitted from the FileSystemDevice to cause the IndexingWorker
     *  to index the url. An indexer can only run once but because the indexing itself
     *  blocks the threads execution loop there is no issues with calling this multiple
     *  times except that the indexing will run over and over again which is likely not
     *  intended.
     **/
    void startIndexing(const QUrl url) const;
    /** The signal is emitted when the FileSystemDevice has an updated
     *  MediaPlaylist. The argument is a reference to the new playlist
     **/
//    void mediaPlaylistUpdated(const QJsonObject playList);

private:
    QThread * thread;
    IndexingWorker * worker;
    bool busy;

};

#endif // FILESYSTEMDEVICE_H
