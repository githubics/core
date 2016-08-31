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
#ifndef QT_NO_DEBUG_OUTPUT
//#define QT_NO_DEBUG_OUTPUT
#endif
#include <QDebug>
#include "qicsmediaplayer.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>

QIcsMediaPlayer::QIcsMediaPlayer(QObject *parent)
    : MediaPlayerInterface(parent)
    , m_playerView(new QQuickView())
    , m_audioVideoPlayerController(new MediaPlayerController(this))
{
    Q_INIT_RESOURCE(qml);
    m_playerView->engine()->rootContext()->setContextProperty("mediaPlayerController", m_audioVideoPlayerController);
    m_playerView->setSource(QUrl(QStringLiteral("qrc:/mediaplayer/main.qml")));
    m_playerView->setFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);

    connect(m_audioVideoPlayerController,&MediaPlayerController::playbackStateChanged,this,&QIcsMediaPlayer::sendPlayState);
    m_audioVideoPlayerController->setPlaybackState(MediaPlayerController::StoppedState);
    connect(m_audioVideoPlayerController,&MediaPlayerController::trackIndexChanged,this,&MediaPlayerInterface::currentTrackIndexChanged);
}

void QIcsMediaPlayer::setMediaPlaylist(const QStringList playList)
{
    m_audioVideoPlayerController->setVideoTracks(playList);
}

void QIcsMediaPlayer::setCurrentTrack(int index) const
{

    m_audioVideoPlayerController->setTrackByIndex(index);

    if (!m_playerView->isVisible()){
        m_playerView->show();
    } else {
        m_playerView->raise();
    }
    m_audioVideoPlayerController->playTrack();
}

const QSet<QString> QIcsMediaPlayer::supportedFileSuffixes() const
{
    QSet<QString> suffixes;
    suffixes << "mp4"<<"mov"<<"avi"<<"mkv"<<"m4v"<<"webm";
    return suffixes;
}

void QIcsMediaPlayer::sendPlayState(MediaPlayerController::PlaybackState state) const
{
    PlayState playState=translatePlayState(state);
    emit playStateChanged(playState);
}

int QIcsMediaPlayer::currentTrackIndex() const
{
    return m_audioVideoPlayerController->trackIndex();
}

void QIcsMediaPlayer::play() const
{
    // TODO get from the controller if we are in video or audio mode
    if (!m_playerView->isVisible()){
        m_playerView->show();
    } else {
        m_playerView->raise();
    }
    m_audioVideoPlayerController->playTrack();
}

void QIcsMediaPlayer::pause() const
{
    m_audioVideoPlayerController->pauseTrack();
//    translatePlayState();
}

void QIcsMediaPlayer::stop() const
{
    m_audioVideoPlayerController->stopTrack();
    if (m_playerView->isVisible()) m_playerView->hide();
//    translatePlayState();
}

void QIcsMediaPlayer::next() const
{
    m_audioVideoPlayerController->setNextTrack();
    m_audioVideoPlayerController->playTrack();
//    translatePlayState();

}

void QIcsMediaPlayer::previous() const
{
    m_audioVideoPlayerController->setPreviousTrack();
    m_audioVideoPlayerController->playTrack();

//    translatePlayState();
}

// TODO: Implement adding and removing of playlist components
// without disturbing playback if at all possible
/*void QIcsMediaPlayer::addPlaylistMedia(QUrl mediaItemUrl)
{
//    m_mediaPlaylist->addMedia(mediaItemUrl);
}*/

void QIcsMediaPlayer::setCurrentVolume(quint8 currentVolume)
{
    if(currentVolume == m_currentVolume) return;

    m_currentVolume = currentVolume;
//    emit currentVolumeChanged(m_currentVolume);

}
void QIcsMediaPlayer::incrementCurrentVolume()
{
    setCurrentVolume(m_currentVolume + 5);
//    m_mediaPlayer->setVolume(m_currentVolume);
}

void QIcsMediaPlayer::decrementCurrentVolume()
{
    setCurrentVolume(m_currentVolume - 5);
//    m_mediaPlayer->setVolume(m_currentVolume);
}


void QIcsMediaPlayer::setVideoRectangle(const QRect videoRect) const
{
    m_playerView->setGeometry(videoRect);
}

QIcsMediaPlayer::PlayState QIcsMediaPlayer::translatePlayState(MediaPlayerController::PlaybackState state) const
{
    PlayState playState;

    switch (state) {
    case MediaPlayerController::StoppedState:
        playState=mmTypes::Stopped;
        break;
    case MediaPlayerController::PausedState:
        playState=mmTypes::Paused;
        break;
    case MediaPlayerController::PlayingState:
        playState=mmTypes::Playing;
        break;
    }
    return playState;
}
