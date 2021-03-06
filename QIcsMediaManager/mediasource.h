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
#ifndef MEDIASOURCE_H
#define MEDIASOURCE_H

#include "mediadeviceinterface.h"
#include "mediamanagertypes.h"
#include <QObject>
#include <QJsonObject>
/*!
 * \brief MediaSource
 * This class stores and caches Playlists from MediaDevices.
 * in JSonObjects that contain the DeviceUrl,
 * a JsonArray with AudioFiles,
 * a JSonArray with VideoFiles
 *
 */
class MediaSource : public QObject
{
    Q_OBJECT

public:
    typedef MediaDeviceInterface::Playlist Playlist;

    /** Creates a MediaSource. **/
    explicit MediaSource(QObject * parent = 0 );

    /** Check whether the MediaType is present in the MediaSourcePlaylist */
    bool hasMediaType(const mmTypes::MediaType & mediaType) const;

    /** Returns a QJsonArray for the given MediaType.
     *  The returned QJsonArray will be empty if the MediaType was not present
     **/
    const QJsonArray mediaArray(const mmTypes::MediaType & mediaType) const;

    /** Returns the deviceUrl that is associated with this MediaSource as a string */
    const QString deviceUrlString() const;

//    /** Saves mediaObject to the file fileName, return true on success, false -- otherwise**/
//    bool saveJsonObjectToFile( QString fileName, QJsonObject mediaObject );

//    /** Reads QJsonObject from the file fileName**/
//    QJsonObject getMediaSourceListFromFile( QString fileName );

    /** Fast unchecked access to the Playlist pointer */
    Playlist * playlist() {return &m_playlist;}


signals:
    /** The MediaSource notifies the MediaManager that the playlist has changed
     *  with this signal. The payload is a pointer to the MediaSource instance and
     *  the MediaManager can then grab the playlists by MediaType from the MediaSource
     **/
    void mediaSourcePlaylistChanged(const MediaSource * mediaSource) const;

public slots:
    /** MediaDevice emits a signal when a playlist has been updated that is connected
     *  to this slot and that sets the new playlist. MediaSource emits the mediaSourcePlaylistChanged
     *  signal when this function exits.
     **/
    void onMediaPlaylistUpdated();

private:
    Playlist m_playlist;
};

#endif // MEDIASOURCE_H
