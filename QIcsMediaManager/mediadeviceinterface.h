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
#ifndef MEDIADEVICEINTERFACE
#define MEDIADEVICEINTERFACE

#include <QObject>
#include <QVariantMap>

// forward declare Qt classes
QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

/** TODO: Document and somehow ensure that the JSon format returned from the
 *  DeviceInterface implementations contains the expected format
 **/
class MediaDeviceInterface : public QObject
{
    Q_OBJECT

public:
    /** The structure of the playlist that we are working on.
     *  The correspnding object is not owned by the device interface
     *  but by the MediaSource. A pointer to the playlist is handed
     *  to the Device which then updates the content.
     **/
    struct Playlist {
        QString deviceUrl;
        QList<QVariantMap> audioFiles;
        QList<QVariantMap> videoFiles;
    };

    explicit MediaDeviceInterface(QObject * parent = 0)
        : QObject(parent)
        {}
    virtual ~MediaDeviceInterface() {}

    /** Asynchronous call to update the media playlist.
     *  When a list is available the signal mediaPlaylistUpdated()
     *  is emitted
     */
    virtual void updateMediaPlaylist(Playlist * playlist) = 0;

signals:
    /** The signal is emitted when the FileSystemDevice has an updated
     *  MediaPlaylist.
     **/
    void mediaPlaylistUpdated();
};

#endif // MEDIADEVICEINTERFACE
