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
#include "qeventloop.h"
#include "simpledevicemanager.h"

#include <QTimer>
#include <QFileSystemWatcher>
#include <QDir>
#include "QProcess"
#if ! defined Q_OS_MAC && ! defined Q_OS_WIN
#include <QProcessEnvironment>
#endif

#if defined Q_OS_MAC
#define USBWATCHPATH QString("/Volumes/")
#elif defined Q_OS_WIN
#define USBWATCHPATH QString("E:/")
#else
#define USBWATCHPATH QString("/media/%1/").arg(pEnv.systemEnvironment().value("USER"))
#endif

SimpleDeviceManager::SimpleDeviceManager(QObject * parent)
    : DeviceManagerInterface(parent)
    , m_usbWatcher(0)
{
#if ! defined Q_OS_MAC && ! defined Q_OS_WIN
    QProcessEnvironment pEnv;
#endif
    m_usbWatchPath = USBWATCHPATH;
    m_usbWatcher = new QFileSystemWatcher();
    m_usbWatcher->addPath(m_usbWatchPath);
    connect(m_usbWatcher, &QFileSystemWatcher::directoryChanged, [=](){ QTimer::singleShot(2000, this, &SimpleDeviceManager::detectUsbDevices); } );
#if defined SIMULATE_USB_DEVICE_PATH
    m_usbWatchPath = QString(SIMULATE_USB_DEVICE_PATH);
#endif
    m_process = new QProcess(this);
    connect(m_process,SIGNAL(readyRead()), this, SLOT(handleProcessResponse()));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(handleProcessError()));
    connect(m_process, SIGNAL(finished(int)), this, SLOT(handleProcessExited(int)));

    QTimer::singleShot(3000, this, &SimpleDeviceManager::detectUsbDevices);
}

void SimpleDeviceManager::deviceChanged()
{
    QDir d(m_usbWatchPath);
    qDebug() << Q_FUNC_INFO << m_usbWatchPath;

    if(!d.exists()) return;
    d.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
    QStringList entryList=d.entryList();

;
    QMap<QString, QString>::const_iterator i = m_deviceList.constBegin();
    while (i != m_deviceList.constEnd()) {


        QString fullPath = m_usbWatchPath+"/"+i.value(); //make into one string so we dont need to do it over and over

        if(entryList.contains( i.value()  ) ){

            if(!m_devices.contains(fullPath)){
                 qDebug() << Q_FUNC_INFO << "found new mounted usb device:" << i.value(); //should likely be logged
                m_devices.append(fullPath );
                emit deviceCreated("USBDevice",QUrl::fromLocalFile(fullPath));
            }

        } else {

            if(i.value() == ""){
               // non usb storage devices will have empty mount points as we only correlate devices tracked in /dev/disk/by-id/usb*
            } else {
                if(m_devices.contains(fullPath)){
                      qDebug() << Q_FUNC_INFO << " device removed:" << fullPath;        //should likey be logged
                        emit deviceRemoved("USBDevice",QUrl::fromLocalFile(fullPath));
                      m_devices.removeAll(fullPath);
                }
            }
        }
        ++i;
    }

    //check whether a device was removed

    /*
     * this block will be delete once additional testing is done. kept for reference.
    foreach (QString gonedir, m_devices) {
        if (entryList.isEmpty()||!entryList.contains(gonedir)) {
            emit deviceRemoved("USBDevice",QUrl::fromLocalFile(m_usbWatchPath+gonedir));
            qDebug() << Q_FUNC_INFO << "device removed:" << gonedir;
            m_devices.removeAll(gonedir);
        }
    }
    // check whether one was inserted
    foreach(QString nudir, entryList) {
        if (nudir.startsWith("Macintosh")) continue;
        if (!m_devices.contains(nudir)) {
            qDebug() << Q_FUNC_INFO << "found device:" << nudir;
            m_devices.append(nudir);
            emit deviceCreated("USBDevice",QUrl::fromLocalFile(m_usbWatchPath+nudir));
        }
    }
    */
}

void SimpleDeviceManager::detectUsbDevices()
{
    m_process->setProgram("/bin/bash");
    m_process->setArguments(QStringList() << "-c" << "for devlink in /dev/disk/by-id/usb*; do readlink -e ${devlink}; done");
    m_process->start();
}

void SimpleDeviceManager::handleProcessResponse()
{
    //reads each line from the usb devices returned by QProcess execution.
    QString usbMount = m_process->readLine();
    m_deviceList.insert(usbMount.remove("\n"), "");
}

void SimpleDeviceManager::handleProcessError()
{
    //for testing
    qDebug() << m_process->errorString();
}

void SimpleDeviceManager::handleProcessExited(int exitCode)
{
    // qDebug() << Q_FUNC_INFO << "setting up mount / device hash now ";

    /* get the mounts. While doing this in a synchronous fashion is bad
     * consider:
     * All systems will briefly throw all i/o to the device just plugged in. The impact on the system varies,
     * but this is done at the same time, so any hiccups would be masked by that. This is also a very short run.
     */
    QProcess mountCheckProcess;
    mountCheckProcess.setProgram("mount");
    mountCheckProcess.start();
    mountCheckProcess.waitForFinished();
    QString res = mountCheckProcess.readAll();

    //we iterate through the map and do some work to correlate mountpoints to the devices found previously, leaving us with a list of usb devices that are mounted as storage.

    foreach (QString usbid, m_deviceList.keys()) {

        if(res.contains(usbid)){
            QString temp = res;
           // qDebug()  << usbid << " drive is usb and is mounted " ;
            int start = temp.indexOf(usbid)+usbid.count();
            int substart =temp.indexOf(" on ", start)+3;
            int end = temp.indexOf(" type ", substart);
            QString mountPoint = temp.mid(substart, end - substart).simplified().remove(m_usbWatchPath);
            m_deviceList.insert(usbid, mountPoint);
            m_devices.append(mountPoint);
        }
    }
    deviceChanged();

}
