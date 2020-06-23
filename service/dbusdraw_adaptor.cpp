/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author: Zhang Hao<zhanghao@uniontech.com>
*
* Maintainer: Zhang Hao <zhanghao@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "dbusdraw_adaptor.h"

#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QWidget>
#include <QDebug>

dbusdraw_adaptor::dbusdraw_adaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

dbusdraw_adaptor::~dbusdraw_adaptor()
{
    // destructor
}

void dbusdraw_adaptor::openFiles(QList<QVariant> filePaths)
{
    QStringList paths;
    for (QVariant path : filePaths) {
        paths << path.toString().toLocal8Bit();
    }
    QMetaObject::invokeMethod(parent(), "openFiles",
                              Q_ARG(QStringList, paths));
}

void dbusdraw_adaptor::openImages(QList<QVariant> images)
{
    for (QVariant img : images) {
        QByteArray data = img.toByteArray();
        QString tmp_data = QString::fromLatin1(data.data(), data.size());
        //data = QByteArray::fromBase64(tmp_data.toLatin1());
        //data = qUncompress(data);
        QByteArray srcData = QByteArray::fromBase64(tmp_data.toLatin1());
        data = qUncompress(srcData);
        QImage image;
        image.loadFromData(data);
        QMetaObject::invokeMethod(parent(), "openImage",
                                  Q_ARG(QImage, image), Q_ARG(const QByteArray &, srcData));
    }
}

bool dbusdraw_adaptor::openFile(QString filePath)
{
    if (QFile::exists(filePath)) {
        QStringList paths;
        paths.append(filePath);
        QMetaObject::invokeMethod(parent(), "openFiles",
                                  Q_ARG(QStringList, paths));
        return true;
    }
    return false;
}

