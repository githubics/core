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
#ifndef INDEXINGWORKER_H
#define INDEXINGWORKER_H

#include <QObject>
#include <QJsonArray>
#include <QStringList>
#include <QUrl>
#include <QHash>
#include <QDateTime>


class IndexingWorker : public QObject
{
    Q_OBJECT
public:
    explicit IndexingWorker(QObject *parent = 0)
        : QObject(parent) {
        // TODO: These need to be synced with the capabilities of the corresponding
        // players somehow.
        // Maybe define a set of AudioFileMediaTypes in the MediaManager and use them here.
        audioFilters << "*.mp3" << "*.ogg" << "*.wav";
        videoFilters << "*.mp4" << "*.mkv" << "*.m4v" << "*.avi" << "*.wmv" << "*.mov";
    }
    ~IndexingWorker(){}

    QJsonArray audioFiles() const {return m_audioFiles;}
    QJsonArray videoFiles() const {return m_videoFiles;}

public slots:
    void startIndexing(const QUrl url);

signals:
    void indexingFinished() const;
    void indexingAborted() const;

private:
    void indexDirectory(const QString dirPath);
    void buildDirectoryTree(const QString & dirPath);
    void printDirectoryTree();
    void indexDirectoryTree();
    void mediaInfo(const QStringList fileList);

private:
    QJsonArray m_audioFiles;
    QJsonArray m_videoFiles;
    QStringList audioFilters;
    QStringList videoFilters;

    QHash<QString, QDateTime> m_dirTree;

};


#endif // INDEXINGWORKER_H
