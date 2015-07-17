/*
   This file is part of QCost, a cost estimating software, and was
   derived from a file contained in the QtGui module of the Qt Toolkit
   (version 5.1.1).
   Original file name and position: qt/qtbase/src/gui/text/qtextodfwriter_p.h

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

#ifndef ODTWRITER_H
#define ODTWRITER_H

#include <QString>

class QTextCursor;
class QTextBlock;
class QIODevice;
class QXmlStreamWriter;
class QTextBlockFormat;
class QTextCharFormat;
class QTextListFormat;
class QTextFrameFormat;
class QTextTableCellFormat;
class QTextFrame;
class QTextFragment;
class QOutputStrategy;
class QTextDocument;
class QTextTableFormat;
class OdtWriterPrivate;
class QTextCodec;

class OdtWriter {
public:
    OdtWriter(const QTextDocument &document, QIODevice *device);
    bool writeAll();

    void setCodec(QTextCodec *codec);
    void setCreateArchive(bool on);
    bool createArchive() const;

    void writeBlock(QXmlStreamWriter &writer, const QTextBlock &block);
    void writeFormats(QXmlStreamWriter &writer) const;
    void writeBlockFormat(QXmlStreamWriter &writer, QTextBlockFormat format, int formatIndex) const;
    void writeCharacterFormat(QXmlStreamWriter &writer, QTextCharFormat format, int formatIndex) const;
    void writeListFormat(QXmlStreamWriter &writer, QTextListFormat format, int formatIndex) const;
    void writeFrameFormat(QXmlStreamWriter &writer, QTextFrameFormat format, int formatIndex) const;
    void writeTableCellFormat(QXmlStreamWriter &writer, QTextTableCellFormat format, int formatIndex) const;
    void writeTableFormat(QXmlStreamWriter &writer, QTextTableFormat format, int formatIndex) const;

    void writeFrame(QXmlStreamWriter &writer, const QTextFrame *frame);
    void writeInlineCharacter(QXmlStreamWriter &writer, const QTextFragment &fragment) const;

    const QString officeNS, textNS, styleNS, foNS, tableNS, drawNS, xlinkNS, svgNS;

    void setPageSizeMM( double width, double height );
    void setMarginsMM( double left, double right, double top, double bottom );
    void setPageOrientation(Qt::Orientation orientation);

    void setDefaultPointSize( int v );
private:
    OdtWriterPrivate * m_d;

    QString tableCellBorderData( int bStyle, int bWidth, int bColor, const  QTextTableCellFormat & format ) const;
};

#endif // ODTWRITER_H
