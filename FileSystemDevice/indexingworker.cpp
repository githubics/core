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
//#define QT_NO_DEBUG_OUTPUT
#endif
#include <QDebug>
#include <QDir>

#include "indexingworker.h"

#if defined USE_MEDIAINFO
#include "MediaInfo/MediaInfoList.h"
#define QSLWSTR( _ARG_ ) QStringLiteral( _ARG_ ).toStdWString()
#endif

// These delimiters signal end of line and end of parameter,
// if these by any chance show up as part of a file name we are in trouble
#define MI_EOL_DELIMITER "|^"
#define MI_EOP_DELIMITER "|+"

void IndexingWorker::startIndexing(MediaDeviceInterface::Playlist * playlist)
{
    Q_ASSERT(playlist!=0);
    playlist->audioFiles.clear();
    playlist->videoFiles.clear();
    buildDirectoryTree(playlist->deviceUrl);
    printDirectoryTree();
    indexDirectoryTree(playlist);
    emit indexingFinished();
}

void IndexingWorker::printDirectoryTree()
{
    qDebug() << Q_FUNC_INFO ;
    for (QHash<QString, QDateTime>::const_iterator i=m_dirTree.constBegin();
         i!=m_dirTree.constEnd();
         ++i) {
        qDebug() << i.key() << ": " << i.value();
    }
}

void IndexingWorker::mediaInfo(const QStringList fileList, MediaDeviceInterface::Playlist * playlist)
{
    if (fileList.isEmpty()) return;
    QStringList generalParams;
    generalParams << "CompleteName" << "FolderName" << "FileName" << "FileExtension" << "Artist" << "Cover_Data"
                  << "FileSize" << "Duration" << "FrameRate" << "BitRate" << "BitRate_Mode/String"
                  << "Format" << "InternetMediaType"
                  << "Title" << "Season" << "Movie"
                  << "Album" << "Album_More" << "Album/Performer"
                  << "Part" << "Track" << "Track/Position" << "Compilation"
                  << "Performer" << "Genre" << "Mood"
                  << "Released_Date" << "Original/Released_Date" << "Recorded_Date"
                  << "Encoded_Application" << "Encoded_Library" << "BPM"
                  << "Cover" << "Cover_Mime" << "Lyrics"
                  << "Added_Date";
    QString generalInform;
    generalInform="General;";
    foreach(QString s, generalParams) {
        generalInform += QString("\%%1\%%2").arg(s).arg(MI_EOP_DELIMITER);
    }
    generalInform+=MI_EOL_DELIMITER;
    MediaInfoLib::MediaInfoList MI;
    MI.Option(QSLWSTR("ParseSpeed"), QSLWSTR("0"));
    MI.Option(QSLWSTR("Language"), QSLWSTR("raw"));
    MI.Option(QSLWSTR("ReadByHuman"), QSLWSTR("0"));
    MI.Option(QSLWSTR("Legacy"), QSLWSTR("0"));
    int nfiles;
    foreach (QString file, fileList) {
        nfiles=MI.Open(file.toStdWString(), MediaInfoLib::FileOption_NoRecursive);
    }
//    qDebug() << Q_FUNC_INFO << "Opened" << nfiles << "files";
    if (nfiles!=fileList.count()) {
        qWarning() << Q_FUNC_INFO << "some files could not be opened, have" << nfiles << "out of" << fileList.count();
    }

//    qDebug() << Q_FUNC_INFO << generalInform;
    MI.Option(QStringLiteral("Inform").toStdWString(), generalInform.toStdWString());
    QString informOptionExample=QString::fromStdWString(MI.Inform());
//    qDebug() << Q_FUNC_INFO << qPrintable("\r\n\r\nGeneral Inform\r\n") << qPrintable(informOptionExample);

    QStringList informResult=informOptionExample.split(MI_EOL_DELIMITER,QString::SkipEmptyParts);
    QVariantMap resMap;
    foreach (QString res, informResult) {
//        qDebug() << Q_FUNC_INFO << res;
        QStringList resList=res.split(MI_EOP_DELIMITER);
//        qDebug() << resList.count() << generalParams.count();
        Q_ASSERT((resList.count()-1)==generalParams.count());
        for (int i=0;i<resList.count()-1;++i) {
//            qDebug() << generalParams[i] << ":" << resList[i];
            resMap[generalParams[i]] = resList[i];
        }
        QString mimeType=resMap["InternetMediaType"].toString();
        if (mimeType.startsWith("audio")) playlist->audioFiles.append(resMap);
        else if (mimeType.startsWith("video")) playlist->videoFiles.append(resMap);
        else {
            qWarning() << Q_FUNC_INFO << "mimetype for file" << resMap["CompleteName"]<< "not one of audio or video but" << resMap["InternetMediaType"];
        }
//        qDebug() << Q_FUNC_INFO << "resObject" << resObject;
    }
}



void IndexingWorker::buildDirectoryTree( const QString & dirPath)
{
    QDir dir(dirPath);
    qDebug() << Q_FUNC_INFO << dir << dirPath;

    // TODO: this should be configurable
    if (dirPath.count("/")>31) return;

    QFileInfo dirInfo(dirPath);
    QDateTime dirDateTime = dirInfo.lastModified();
    m_dirTree.insert(dirPath, dirDateTime);

    dir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
    QStringList dirList=dir.entryList();

    foreach ( QString curDir, dirList )
    {
        QString dirFullPath = dirPath + QDir::separator() + curDir;
        buildDirectoryTree( dirFullPath );
    }
}

void IndexingWorker::indexDirectoryTree(MediaDeviceInterface::Playlist * playlist)
{
    QHash<QString, QDateTime>::const_iterator id = m_dirTree.constBegin();

    while (id != m_dirTree.constEnd()) {
        qDebug() << id.key() << ": " << id.value();
        const QString & dirPath=id.key();
        QDir dir(dirPath);
        qDebug() << Q_FUNC_INFO << dir << dirPath;

        QStringList mediaInfoList;
        dir.setFilter(QDir::Files);
        dir.setNameFilters(audioFilters);
        QStringList audioList=dir.entryList();
        foreach (const QString & a, audioList)
            mediaInfoList.append(dirPath+"/"+a);

        dir.setNameFilters(videoFilters);
        QStringList videoList=dir.entryList();
        foreach (QString v, videoList)
            mediaInfoList.append(dirPath+"/"+v);

        mediaInfo(mediaInfoList,playlist);

        qDebug() << Q_FUNC_INFO << "appended" <<audioList.count()<<"audio files and"<<videoList.count()<<"video files in"<<dir.absolutePath();
        ++id;
    }
}
