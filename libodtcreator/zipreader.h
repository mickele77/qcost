/*
   This file is part of QCost, a cost estimating software, and was
   derived from a file contained in the QtGui module of the Qt Toolkit
   (version 5.1.1).
   Original file name and position: qt/qtbase/src/gui/text/qzipreader_p.h

   Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies)
   Contact: http://www.qt-project.org/legal
   Copyright (C) 2013-2016 Mocciola Michele

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef ZIPREADER_H
#define ZIPREADER_H

#include "odtcreator_export.h"

#include <QDateTime>
#include <QFile>
#include <QString>

class ZipReaderPrivate;

class EXPORT_ODTCREATOR_LIB_OPT ZipReader
{
public:
    explicit ZipReader(const QString &fileName, QIODevice::OpenMode mode = QIODevice::ReadOnly );

    explicit ZipReader(QIODevice *device);
    ~ZipReader();

    QIODevice* device() const;

    bool isReadable() const;
    bool exists() const;

    struct FileInfo
    {
        FileInfo();
        FileInfo(const FileInfo &other);
        ~FileInfo();
        FileInfo &operator=(const FileInfo &other);
        bool isValid() const;
        QString filePath;
        uint isDir : 1;
        uint isFile : 1;
        uint isSymLink : 1;
        QFile::Permissions permissions;
        uint crc;
        qint64 size;
        QDateTime lastModified;
        void *d;
    };

    QList<FileInfo> fileInfoList() const;
    int count() const;

    FileInfo entryInfoAt(int index) const;
    QByteArray fileData(const QString &fileName) const;
    bool extractAll(const QString &destinationDir) const;

    enum Status {
        NoError,
        FileReadError,
        FileOpenError,
        FilePermissionsError,
        FileError
    };

    Status status() const;

    void close();

private:
    ZipReaderPrivate *d;

    Q_DISABLE_COPY(ZipReader)
};

#endif // ZIPREADER_H
