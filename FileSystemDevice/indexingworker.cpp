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
#include <QJsonObject>

#include "indexingworker.h"

#if defined USE_MEDIAINFO
#include "MediaInfo/MediaInfoList.h"
#define QSLWSTR( _ARG_ ) QStringLiteral( _ARG_ ).toStdWString()
#endif

void IndexingWorker::startIndexing(const QUrl url)
{
    m_audioFiles = QJsonArray();
    m_videoFiles = QJsonArray();
    buildDirectoryTree(url.toLocalFile());
    printDirectoryTree();
    indexDirectoryTree();
    emit indexingFinished();
}

void IndexingWorker::printDirectoryTree()
{
    qDebug() << Q_FUNC_INFO ;
    QHash<QString, QDateTime>::const_iterator i = m_dirTree.constBegin();
    while (i != m_dirTree.constEnd()) {
        qDebug() << i.key() << ": " << i.value();
        ++i;
    }
}

void IndexingWorker::mediaInfo(const QStringList fileList)
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
        generalInform += QString("\%%1\%|").arg(s);
    }
    generalInform+="\\n";
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

    QStringList informResult=informOptionExample.split('\n',QString::SkipEmptyParts);
    QVariantMap resMap;
    foreach (QString res, informResult) {
//        qDebug() << Q_FUNC_INFO << res;
        QStringList resList=res.split("|");
//        qDebug() << resList.count() << generalParams.count();
        Q_ASSERT((resList.count()-1)==generalParams.count());
        for (int i=0;i<resList.count()-1;++i) {
//            qDebug() << generalParams[i] << ":" << resList[i];
            resMap[generalParams[i]] = resList[i];
        }
        QJsonObject resObject=QJsonObject::fromVariantMap(resMap);
        QString mimeType=resMap["InternetMediaType"].toString();
        if (mimeType.startsWith("audio")) m_audioFiles.append(resObject);
        else if (mimeType.startsWith("video")) m_videoFiles.append(resObject);
        else {
            qWarning() << Q_FUNC_INFO << "mimetype for file" << resMap["CompleteName"]<< "not one of audio or video but" << resMap["InternetMediaType"];
        }
//        qDebug() << Q_FUNC_INFO << "resObject" << resObject;
    }
}


void IndexingWorker::indexDirectory(const QString dirPath)
{
    QDir dir(dirPath);
    qDebug() << Q_FUNC_INFO << dir << dirPath;

    // FIXME: this should be configurable and is different on Linux and Mac
    if (dirPath.count("/")>31) return;
    QStringList mediaInfoList;

    dir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
    QStringList dirList=dir.entryList();

    dir.setFilter(QDir::Files);
    dir.setNameFilters(audioFilters);
    QStringList audioList=dir.entryList();
    foreach (QString a, audioList)
        mediaInfoList.append(dirPath+"/"+a);

    dir.setNameFilters(videoFilters);
    QStringList videoList=dir.entryList();
    foreach (QString v, videoList)
        mediaInfoList.append(dirPath+"/"+v);

    mediaInfo(mediaInfoList);

    qDebug() << Q_FUNC_INFO << "appended" <<audioList.count()<<"audio files and"<<videoList.count()<<"video files in"<<dir.absolutePath();
    qDebug() << Q_FUNC_INFO << "moving on to" <<dirList;

    foreach (QString dirName, dirList) {
        // FIXME: take this out and make directory filters configurable
//        if (!dirName.startsWith("rolands"))
            indexDirectory(dirPath+"/"+dirName);

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

void IndexingWorker::indexDirectoryTree()
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

        mediaInfo(mediaInfoList);

        qDebug() << Q_FUNC_INFO << "appended" <<audioList.count()<<"audio files and"<<videoList.count()<<"video files in"<<dir.absolutePath();
        ++id;
    }
}
