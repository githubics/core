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
#include "mediadeviceinterface.h"

#include <QJsonArray>
#include <QJsonDocument>


MediaSource::MediaSource(MediaDeviceInterface * device, const QUrl & deviceUrl, QObject * parent)
    : QObject(parent)
    , m_device(device)
    , m_deviceUrl(deviceUrl)
{
    m_device->setParent(this);
    connect(m_device,&MediaDeviceInterface::mediaPlaylistUpdated,this,&MediaSource::setMediaSourcePlaylist);
}

void MediaSource::updateMediaSourcePlaylist()
{
    m_device->updateMediaPlaylist(m_deviceUrl);
}

QJsonObject MediaSource::mediaSourcePlaylist() const
{
    return m_mediaSourcePlaylist;
}

void MediaSource::setMediaSourcePlaylist(const QJsonObject & mediaSourcePlaylist)
{
    qDebug() << Q_FUNC_INFO;
    if (m_mediaSourcePlaylist == mediaSourcePlaylist)
        return;

    qDebug() << Q_FUNC_INFO << "setting new mediaSourcePlaylist";
    m_mediaSourcePlaylist = mediaSourcePlaylist;
    emit mediaSourcePlaylistChanged(this);
}

bool MediaSource::hasMediaType(const QString & mediaTypeStr) const
{
    if (m_mediaSourcePlaylist.contains(mediaTypeStr) && !m_mediaSourcePlaylist[mediaTypeStr].toArray().isEmpty())
        return true;
    return false;
}

const QJsonArray MediaSource::mediaArray(const QString &mediaTypeStr) const
{
    if (m_mediaSourcePlaylist.contains(mediaTypeStr))
        return m_mediaSourcePlaylist[mediaTypeStr].toArray();
    else
        return QJsonArray();
}

const QString MediaSource::deviceUrlString() const
{
    return m_deviceUrl.toString();
}

const QUrl MediaSource::deviceUrl() const
{
    return m_deviceUrl;
}

void MediaSource::setDeviceUrl(QUrl deviceUrl)
{
    if (m_deviceUrl == deviceUrl)
        return;

    m_deviceUrl = deviceUrl;
    emit deviceUrlChanged(deviceUrl);
}
