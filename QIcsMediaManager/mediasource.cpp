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

#include "mediasource.h"

#include <QJsonArray>
#include <QUrl>

MediaSource::MediaSource(QObject * parent)
    : QObject(parent)
{
}

void MediaSource::setMediaSourcePlaylist()
{
    qDebug() << Q_FUNC_INFO;
    emit mediaSourcePlaylistChanged(this);
}

bool MediaSource::hasMediaType(const mmTypes::MediaType & mediaType) const
{
    if ((mediaType==mmTypes::AudioFile) && (!m_playlist.audioFiles.isEmpty())) return true;
    if ((mediaType==mmTypes::VideoFile) && (!m_playlist.videoFiles.isEmpty())) return true;
    return false;
}

const QJsonArray MediaSource::mediaArray(const mmTypes::MediaType &mediaType) const
{
    // TODO: We are still creating a copy of the data here as we are creating
    // JSonObjects from VariantMaps. Because the data is stored in QStrings
    // this still does not matter too much.
    // Once we refine the implementation of the Playlist this can probably be
    // further simplified.
    QJsonArray mediaPlaylist;
    const QList<QVariantMap> * mediaFiles=0;
    if (mediaType==mmTypes::AudioFile) mediaFiles=&m_playlist.audioFiles;
    else if (mediaType==mmTypes::VideoFile) mediaFiles=&m_playlist.videoFiles;
    // TODO: Add code for additional MediaTypes here
    if (mediaFiles) {
        foreach (const QVariantMap & file, *mediaFiles) {
            mediaPlaylist.append(QJsonObject::fromVariantMap(file));
        }
    }
    return mediaPlaylist;
}

const QString MediaSource::deviceUrlString() const
{
    return m_playlist.deviceUrl;
}
