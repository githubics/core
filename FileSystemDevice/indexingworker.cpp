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
    indexDirectory(url.toLocalFile());
    buildDirectoryTree(url.toLocalFile());
    emit indexingFinished();
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
    qDebug() << Q_FUNC_INFO << "Opened" << nfiles << "files";
    if (nfiles!=fileList.count()) {
        qWarning() << Q_FUNC_INFO << "some files could not be opened, have" << nfiles << "out of" << fileList.count();
    }

    qDebug() << Q_FUNC_INFO << generalInform;
    MI.Option(QStringLiteral("Inform").toStdWString(), generalInform.toStdWString());
    QString informOptionExample=QString::fromStdWString(MI.Inform());
    qDebug() << Q_FUNC_INFO << qPrintable("\r\n\r\nGeneral Inform\r\n") << qPrintable(informOptionExample);

    QStringList informResult=informOptionExample.split('\n',QString::SkipEmptyParts);
    QVariantMap resMap;
    foreach (QString res, informResult) {
        qDebug() << Q_FUNC_INFO << res;
        QStringList resList=res.split("|");
        qDebug() << resList.count() << generalParams.count();
        Q_ASSERT((resList.count()-1)==generalParams.count());
        for (int i=0;i<resList.count()-1;++i) {
            qDebug() << generalParams[i] << ":" << resList[i];
            resMap[generalParams[i]] = resList[i];
        }
        QJsonObject resObject=QJsonObject::fromVariantMap(resMap);
        QString mimeType=resMap["InternetMediaType"].toString();
        if (mimeType.startsWith("audio")) m_audioFiles.append(resObject);
        else if (mimeType.startsWith("video")) m_videoFiles.append(resObject);
        else {
            qWarning() << Q_FUNC_INFO << "mimetype for file" << resMap["CompleteName"]<< "not one of audio or video but" << resMap["InternetMediaType"];
        }
        qDebug() << Q_FUNC_INFO << "resObject" << resObject;
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


void IndexingWorker::buildDirectoryTree( const QString dirPath )
{
    QDir dir(dirPath);
    qDebug() << Q_FUNC_INFO << dir << dirPath;

    // FIXME: this should be configurable and is different on Linux and Mac
    if (dirPath.count("/")>31) return;

    //
    QFileInfo* pDirInfo = new QFileInfo( dirPath );
    QDateTime dirDateTime = pDirInfo->lastModified();

    m_directoryDate.insert( dirPath, dirDateTime);

    dir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
    QStringList dirList=dir.entryList();

    foreach ( QString curDir, dirList )
    {
        QString dirFullPath = dirPath + QDir::separator() + curDir;

        pDirInfo = new QFileInfo( dirFullPath );
        dirDateTime = pDirInfo->lastModified();

        m_directoryDate.insert( dirFullPath, dirDateTime);

        buildDirectoryTree( dirFullPath );
    }
}

/*
void IndexingWorker::indexDirectoryWithTime(const QString dirPath)
{

    // FIXME: this should be configurable and is different on Linux and Mac
    if ( dirPath.count("/") > m_directory_scan_depth )
        return;

    QDir dir(dirPath);
    qDebug() << Q_FUNC_INFO << dir << dirPath;

    dir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
    QStringList dirList=dir.entryList();
    bool old_map_file_exists = QFile( m_map_file_name ).exists();


    foreach ( QString cur_dir, dirList )
    {
        QString full_path_cur_dir = dirPath + QString("/") + cur_dir;

        // Map contains directory new path and access date
        QMap< QString, QString> new_map_path_date;

        QFileInfo* p_dir_info = new QFileInfo( dirPath + QString("/") + cur_dir );
        QDateTime date_time_dir = p_dir_info->lastModified();
        new_map_path_date.insert( dirPath + QString("/") + cur_dir, date_time_dir.toString() );


        // Map contains directory old path and access date
        QMap< QString, QString> old_map_path_date;

        if( old_map_file_exists )
        {
            old_map_path_date = readMap( m_map_file_name );

            // Compare dates/times in old and new map
            QMapIterator<QString, QString> i( new_map_path_date );
            while ( i.hasNext() )
            {
                i.next();
                bool key_exist = old_map_path_date.contains( i.key() );

                if( key_exist )
                {
                    QDateTime new_dt = QDateTime::fromString( i.value() );
                    QDateTime old_dt = QDateTime::fromString( old_map_path_date.value( i.key() ) );

                    if( old_dt < new_dt )
                        m_dir_changed = true;
                }
                else
                {
                    old_map_path_date.insert( i.key(), i.value() );
                    m_dir_changed = true;
                }
            }
        }
        else
        {
            m_dir_changed = true;
        }

        indexDirectoryWithTime( full_path_cur_dir );

        if( m_dir_changed )
        {
            // Modifie existing file map
            if( old_map_file_exists )
            {
                QMap<QString, QString> map_from_file = readMap( m_map_file_name );

                QMapIterator<QString, QString> i( new_map_path_date );
                while (i.hasNext())
                {
                    i.next();
                    map_from_file.remove( i.key() );
                    map_from_file.insert( i.key(), i.value() );
                }

                writeMap( map_from_file, m_map_file_name );
            }
            else // or write map file for very first time
            {
                writeMap( new_map_path_date, m_map_file_name );
            }
        }

 **/
