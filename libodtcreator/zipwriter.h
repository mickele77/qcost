/*
   This file is part of QCost, a cost estimating software, and was
   derived from a file contained in the QtGui module of the Qt Toolkit
   (version 5.1.1).
   Original file name and position: qt/qtbase/src/gui/text/qziwriter_p.h

   Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies)
   Contact: http://www.qt-project.org/legal
   Copyright (C) 2013-2014 Mocciola Michele

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

#ifndef ZIPWRITER_H
#define ZIPWRITER_H

#include "odtcreator_export.h"

#include <QString>
#include <QFile>

class ZipWriterPrivate;

class EXPORT_ODTCREATOR_LIB_OPT ZipWriter
{
public:
    explicit ZipWriter(const QString &fileName, QIODevice::OpenMode mode = (QIODevice::WriteOnly | QIODevice::Truncate) );

    explicit ZipWriter(QIODevice *device);
    ~ZipWriter();

    QIODevice* device() const;

    bool isWritable() const;
    bool exists() const;

    enum Status {
        NoError,
        FileWriteError,
        FileOpenError,
        FilePermissionsError,
        FileError
    };

    Status status() const;

    enum CompressionPolicy {
        AlwaysCompress,
        NeverCompress,
        AutoCompress
    };

    void setCompressionPolicy(CompressionPolicy policy);
    CompressionPolicy compressionPolicy() const;

    void setCreationPermissions(QFile::Permissions permissions);
    QFile::Permissions creationPermissions() const;

    void addFile(const QString &fileName, const QByteArray &data);

    void addFile(const QString &fileName, QIODevice *device);

    void addDirectory(const QString &dirName);

    void addSymLink(const QString &fileName, const QString &destination);

    void close();
private:
    ZipWriterPrivate *d;
    Q_DISABLE_COPY(ZipWriter)
};

#endif // ZIPWRITER_H
