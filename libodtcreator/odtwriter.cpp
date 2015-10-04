/*
   This file is part of QCost, a cost estimating software, and was
   derived from a file contained in the QtGui module of the Qt Toolkit
   (version 5.1.1).
   Original file name and position: qt/qtbase/src/gui/text/qtextodfwriter.cpp

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

#include "odtwriter.h"

#include <QImageWriter>
#include <QTextListFormat>
#include <QTextList>
#include <QBuffer>
#include <QUrl>

#include <QTextTable>
#include <QTextDocument>
#include <QStack>
#include <QXmlStreamWriter>

#include "qtextformatuserdefined.h"
#include "zipwriter.h"

#include <QDebug>

class OdtWriterPrivate{
public:
    OdtWriterPrivate( const QTextDocument &doc, QIODevice *dev ):
        pageWidth(210.0),
        pageHeight(297.0),
        marginLeft(10.0),
        marginRight(10.0),
        marginTop(10.0),
        marginBottom(10.0),
        defaultPointSize(8),
        document(&doc),
        device(dev),
        strategy(0),
        codec(0),
        createArchive(true){
    }
    double pageWidth;
    double pageHeight;
    double marginLeft;
    double marginRight;
    double marginTop;
    double marginBottom;
    Qt::Orientation pageOrientation;
    int defaultPointSize;

    const QTextDocument * document;
    QIODevice * device;

    QOutputStrategy * strategy;
    QTextCodec * codec;
    bool createArchive;

    QStack<QTextList *> listStack;
};

/// Convert pixels to postscript point units
static QString pixelToPoint(qreal pixels)
{
    // we hardcode 96 DPI, we do the same in the ODF importer to have a perfect roundtrip.
    return QString::number(pixels * 72 / 96) + QString::fromLatin1("pt");
}

// strategies
class QOutputStrategy {
public:
    QOutputStrategy() : contentStream(0), stylesStream(0), counter(1) { }
    virtual ~QOutputStrategy() {}
    virtual void addFile(const QString &fileName, const QString &mimeType, const QByteArray &bytes) = 0;

    QString createUniqueImageName()
    {
        return QString::fromLatin1("Pictures/Picture%1").arg(counter++);
    }

    QIODevice *contentStream;
    QIODevice *stylesStream;
    int counter;
};

class QXmlStreamStrategy : public QOutputStrategy {
public:
    QXmlStreamStrategy(QIODevice *device)
    {
        contentStream = device;
    }

    virtual ~QXmlStreamStrategy()
    {
        if (contentStream)
            contentStream->close();
    }
    virtual void addFile(const QString &, const QString &, const QByteArray &)
    {
        // we ignore this...
    }
};

class ZipStreamStrategy : public QOutputStrategy {
public:
    ZipStreamStrategy(QIODevice *device)
        : zip(device),
          manifestWriter(&manifest)
    {
        QByteArray mime("application/vnd.oasis.opendocument.text");
        zip.setCompressionPolicy(ZipWriter::NeverCompress);
        zip.addFile(QString::fromLatin1("mimetype"), mime); // for mime-magick
        zip.setCompressionPolicy(ZipWriter::AutoCompress);
        contentStream = &content;
        content.open(QIODevice::WriteOnly);
        styles.open(QIODevice::WriteOnly);
        stylesStream = &styles;
        manifest.open(QIODevice::WriteOnly);

        manifestNS = QString::fromLatin1("urn:oasis:names:tc:opendocument:xmlns:manifest:1.0");
        // prettyfy
        manifestWriter.setAutoFormatting(true);
        manifestWriter.setAutoFormattingIndent(1);

        manifestWriter.writeNamespace(manifestNS, QString::fromLatin1("manifest"));
        manifestWriter.writeStartDocument();
        manifestWriter.writeStartElement(manifestNS, QString::fromLatin1("manifest"));
        manifestWriter.writeAttribute(manifestNS, QString::fromLatin1("version"), QString::fromLatin1("1.2"));
        addFile(QString::fromLatin1("/"), QString::fromLatin1("application/vnd.oasis.opendocument.text"));
        addFile(QString::fromLatin1("content.xml"), QString::fromLatin1("text/xml"));
        addFile(QString::fromLatin1("styles.xml"), QString::fromLatin1("text/xml"));
    }

    ~ZipStreamStrategy()
    {
        manifestWriter.writeEndDocument();
        manifest.close();
        zip.addFile(QString::fromLatin1("META-INF/manifest.xml"), &manifest);
        content.close();
        zip.addFile(QString::fromLatin1("content.xml"), &content);
        styles.close();
        zip.addFile(QString::fromLatin1("styles.xml"), &styles);
        zip.close();
    }

    virtual void addFile(const QString &fileName, const QString &mimeType, const QByteArray &bytes)
    {
        zip.addFile(fileName, bytes);
        addFile(fileName, mimeType);
    }

private:
    void addFile(const QString &fileName, const QString &mimeType)
    {
        manifestWriter.writeEmptyElement(manifestNS, QString::fromLatin1("file-entry"));
        manifestWriter.writeAttribute(manifestNS, QString::fromLatin1("media-type"), mimeType);
        manifestWriter.writeAttribute(manifestNS, QString::fromLatin1("full-path"), fileName);
    }

    QBuffer content;
    QBuffer styles;
    QBuffer manifest;
    ZipWriter zip;
    QXmlStreamWriter manifestWriter;
    QString manifestNS;
};

static QString bulletChar(QTextListFormat::Style style)
{
    switch(style) {
    case QTextListFormat::ListDisc:
        return QChar(0x25cf); // bullet character
    case QTextListFormat::ListCircle:
        return QChar(0x25cb); // white circle
    case QTextListFormat::ListSquare:
        return QChar(0x25a1); // white square
    case QTextListFormat::ListDecimal:
        return QString::fromLatin1("1");
    case QTextListFormat::ListLowerAlpha:
        return QString::fromLatin1("a");
    case QTextListFormat::ListUpperAlpha:
        return QString::fromLatin1("A");
    case QTextListFormat::ListLowerRoman:
        return QString::fromLatin1("i");
    case QTextListFormat::ListUpperRoman:
        return QString::fromLatin1("I");
    default:
    case QTextListFormat::ListStyleUndefined:
        return QString();
    }
}

void OdtWriter::writeFrame(QXmlStreamWriter &writer, const QTextFrame *frame) {
    Q_ASSERT(frame);
    const QTextTable *table = qobject_cast<const QTextTable*> (frame);

    if (table) { // Start a table.
        writer.writeStartElement(tableNS, QString::fromLatin1("table"));
        writer.writeAttribute(tableNS, QString::fromLatin1("name"), table->objectName() );
        writer.writeAttribute(tableNS, QString::fromLatin1("style-name"), QString::fromLatin1("T%1").arg(table->formatIndex() ));
        for( int i=0; i < table->format().columns(); ++i ){
            writer.writeEmptyElement(tableNS, QString::fromLatin1("table-column"));
            writer.writeAttribute(tableNS, QString::fromLatin1("style-name"), QString::fromLatin1("T%1.%2").arg( QString::number(table->formatIndex()), QString::number(i)) );
            // writer.writeAttribute(tableNS, QString::fromLatin1("number-columns-repeated"), QString::number(table->columns()));
        }
        if( table->format().headerRowCount() > 0 ){
            writer.writeStartElement(tableNS, QString::fromLatin1("table-header-rows") );
        }
    } else if (frame->document() && frame->document()->rootFrame() != frame) { // start a section
        writer.writeStartElement(textNS, QString::fromLatin1("section"));
    }

    QTextFrame::iterator iterator = frame->begin();
    QTextFrame *child = 0;

    int tableRow = -1;
    while (! iterator.atEnd()) {
        if (iterator.currentFrame() && child != iterator.currentFrame())
            writeFrame(writer, iterator.currentFrame());
        else { // no frame, its a block
            QTextBlock block = iterator.currentBlock();
            if (table) {
                QTextTableCell cell = table->cellAt(block.position());
                if (tableRow < cell.row()) {
                    if (tableRow >= 0){
                        writer.writeEndElement(); // close table row
                        if( table->format().headerRowCount() > 0 ){
                            if( tableRow == (table->format().headerRowCount()-1) ){
                                writer.writeEndElement(); // close table-header-rows
                            }
                        }
                    }
                    tableRow = cell.row();
                    writer.writeStartElement(tableNS, QString::fromLatin1("table-row"));
                }
                writer.writeStartElement(tableNS, QString::fromLatin1("table-cell"));
                if (cell.columnSpan() > 1)
                    writer.writeAttribute(tableNS, QString::fromLatin1("number-columns-spanned"), QString::number(cell.columnSpan()));
                if (cell.rowSpan() > 1)
                    writer.writeAttribute(tableNS, QString::fromLatin1("number-rows-spanned"), QString::number(cell.rowSpan()));
                if (cell.format().isTableCellFormat()) {
                    writer.writeAttribute(tableNS, QString::fromLatin1("style-name"), QString::fromLatin1("TC%1").arg(cell.tableCellFormatIndex()));
                }
            }
            writeBlock(writer, block);

            if (table){
                // per evitare che se ci sono new line nella cella, i paragrafi vengano scritti su più celle
                QTextTableCell cell = table->cellAt(block.position());
                QTextFrame::iterator nextIterator = iterator;
                ++nextIterator;
                QTextBlock nextBlock = nextIterator.currentBlock();
                QTextTableCell nextCell = table->cellAt(nextBlock.position());
                while( cell == nextCell ){
                    writeBlock(writer, nextBlock);
                    iterator = nextIterator;
                    ++nextIterator;
                    nextBlock = nextIterator.currentBlock();
                    nextCell = table->cellAt(nextBlock.position());
                }

                // scriviamo la fine della cella
                writer.writeEndElement(); // table-cell
            }
        }
        child = iterator.currentFrame();
        ++iterator;
    }
    if (tableRow >= 0){
        writer.writeEndElement(); // close table-row
    }

    if (table || (frame->document() && frame->document()->rootFrame() != frame))
        writer.writeEndElement();  // close table or section element
}

void OdtWriter::writeBlock(QXmlStreamWriter &writer, const QTextBlock &block)
{
    if (block.textList()) { // its a list-item
        const int listLevel = block.textList()->format().indent();
        if (m_d->listStack.isEmpty() || m_d->listStack.top() != block.textList()) {
            // not the same list we were in.
            while (m_d->listStack.count() >= listLevel && !m_d->listStack.isEmpty() && m_d->listStack.top() != block.textList() ) { // we need to close tags
                m_d->listStack.pop();
                writer.writeEndElement(); // list
                if (m_d->listStack.count())
                    writer.writeEndElement(); // list-item
            }
            while (m_d->listStack.count() < listLevel) {
                if (m_d->listStack.count())
                    writer.writeStartElement(textNS, QString::fromLatin1("list-item"));
                writer.writeStartElement(textNS, QString::fromLatin1("list"));
                if (m_d->listStack.count() == listLevel - 1) {
                    m_d->listStack.push(block.textList());
                    writer.writeAttribute(textNS, QString::fromLatin1("style-name"), QString::fromLatin1("L%1")
                                          .arg(block.textList()->formatIndex()));
                }
                else {
                    m_d->listStack.push(0);
                }
            }
        }
        writer.writeStartElement(textNS, QString::fromLatin1("list-item"));
    }
    else {
        while (! m_d->listStack.isEmpty()) {
            m_d->listStack.pop();
            writer.writeEndElement(); // list
            if (m_d->listStack.count())
                writer.writeEndElement(); // list-item
        }
    }

    if (block.length() == 1) { // only a linefeed
        writer.writeEmptyElement(textNS, QString::fromLatin1("p"));
        writer.writeAttribute(textNS, QString::fromLatin1("style-name"), QString::fromLatin1("P%1")
                              .arg(block.blockFormatIndex()));
        if (block.textList())
            writer.writeEndElement(); // numbered-paragraph
        return;
    }
    writer.writeStartElement(textNS, QString::fromLatin1("p"));
    writer.writeAttribute(textNS, QString::fromLatin1("style-name"), QString::fromLatin1("P%1")
                          .arg(block.blockFormatIndex()));
    for (QTextBlock::Iterator frag= block.begin(); !frag.atEnd(); frag++) {
        writer.writeCharacters(QString()); // Trick to make sure that the span gets no linefeed in front of it.
        writer.writeStartElement(textNS, QString::fromLatin1("span"));

        QString fragmentText = frag.fragment().text();
        if (fragmentText.length() == 1 && fragmentText[0] == 0xFFFC) { // its an inline character.
            writeInlineCharacter(writer, frag.fragment());
            writer.writeEndElement(); // span
            continue;
        }

        writer.writeAttribute(textNS, QString::fromLatin1("style-name"), QString::fromLatin1("C%1")
                              .arg(frag.fragment().charFormatIndex()));
        bool escapeNextSpace = true;
        int precedingSpaces = 0;
        int exportedIndex = 0;
        for (int i=0; i <= fragmentText.count(); ++i) {
            QChar character = fragmentText[i];
            bool isSpace = character.unicode() == ' ';

            // find more than one space. -> <text:s text:c="2" />
            if (!isSpace && escapeNextSpace && precedingSpaces > 1) {
                const bool startParag = exportedIndex == 0 && i == precedingSpaces;
                if (!startParag)
                    writer.writeCharacters(fragmentText.mid(exportedIndex, i - precedingSpaces + 1 - exportedIndex));
                writer.writeEmptyElement(textNS, QString::fromLatin1("s"));
                const int count = precedingSpaces - (startParag?0:1);
                if (count > 1)
                    writer.writeAttribute(textNS, QString::fromLatin1("c"), QString::number(count));
                precedingSpaces = 0;
                exportedIndex = i;
            }

            if (i < fragmentText.count()) {
                if (character.unicode() == 0x2028) { // soft-return
                    //if (exportedIndex < i)
                    writer.writeCharacters(fragmentText.mid(exportedIndex, i - exportedIndex));
                    writer.writeEmptyElement(textNS, QString::fromLatin1("line-break"));
                    exportedIndex = i+1;
                    continue;
                } else if (character.unicode() == '\t') { // Tab
                    //if (exportedIndex < i)
                    writer.writeCharacters(fragmentText.mid(exportedIndex, i - exportedIndex));
                    writer.writeEmptyElement(textNS, QString::fromLatin1("tab"));
                    exportedIndex = i+1;
                    precedingSpaces = 0;
                } else if (isSpace) {
                    ++precedingSpaces;
                    escapeNextSpace = true;
                } else if (!isSpace) {
                    precedingSpaces = 0;
                }
            }
        }

        writer.writeCharacters(fragmentText.mid(exportedIndex));
        writer.writeEndElement(); // span
    }
    writer.writeCharacters(QString()); // Trick to make sure that the span gets no linefeed behind it.
    writer.writeEndElement(); // p
    if (block.textList())
        writer.writeEndElement(); // list-item
}

void OdtWriter::writeInlineCharacter(QXmlStreamWriter &writer, const QTextFragment &fragment) const
{
    writer.writeStartElement(drawNS, QString::fromLatin1("frame"));
    if (m_d->strategy == 0) {
        // don't do anything.
    }
    else if (fragment.charFormat().isImageFormat()) {
        QTextImageFormat imageFormat = fragment.charFormat().toImageFormat();
        writer.writeAttribute(drawNS, QString::fromLatin1("name"), imageFormat.name());

        // vvv  Copy pasted mostly from Qt =================
        QImage image;
        QString name = imageFormat.name();
        if (name.startsWith(QLatin1String(":/"))) // auto-detect resources
            name.prepend(QLatin1String("qrc"));
        QUrl url = QUrl(name);
        const QVariant data = m_d->document->resource(QTextDocument::ImageResource, url);
        if (data.type() == QVariant::Image) {
            image = qvariant_cast<QImage>(data);
        } else if (data.type() == QVariant::ByteArray) {
            image.loadFromData(data.toByteArray());
        }

        if (image.isNull()) {
            QString context;
            if (image.isNull()) { // try direct loading
                name = imageFormat.name(); // remove qrc:/ prefix again
                image.load(name);
            }
        }

        // ^^^ Copy pasted mostly from Qt =================
        if (! image.isNull()) {
            QBuffer imageBytes;
            QImageWriter imageWriter(&imageBytes, "png");
            imageWriter.write(image);
            QString filename = m_d->strategy->createUniqueImageName();
            m_d->strategy->addFile(filename, QString::fromLatin1("image/png"), imageBytes.data());

            // get the width/height from the format.
            qreal width = (imageFormat.hasProperty(QTextFormat::ImageWidth)) ? imageFormat.width() : image.width();
            writer.writeAttribute(svgNS, QString::fromLatin1("width"), pixelToPoint(width));
            qreal height = (imageFormat.hasProperty(QTextFormat::ImageHeight)) ? imageFormat.height() : image.height();
            writer.writeAttribute(svgNS, QString::fromLatin1("height"), pixelToPoint(height));

            writer.writeStartElement(drawNS, QString::fromLatin1("image"));
            writer.writeAttribute(xlinkNS, QString::fromLatin1("href"), filename);
            writer.writeEndElement(); // image
        }
    }

    writer.writeEndElement(); // frame
}

void OdtWriter::setPageSizeMM(double width, double height) {
    m_d->pageHeight = height;
    m_d->pageWidth = width;
}

void OdtWriter::setMarginsMM(double left, double right, double top, double bottom) {
    m_d->marginLeft = left;
    m_d->marginRight = right;
    m_d->marginTop = top;
    m_d->marginBottom = bottom;
}

void OdtWriter::setPageOrientation(Qt::Orientation orientation ) {
    m_d->pageOrientation = orientation;
}

void OdtWriter::setDefaultPointSize(int v) {
    m_d->defaultPointSize = v;
}

QString OdtWriter::tableCellBorderData(int bStyle, int bWidth, int bColor, const QTextTableCellFormat &format) const {
    QString style;

    int borderStyle = -1;
    if (format.hasProperty(bStyle) ) {
        borderStyle = format.property(bStyle).toInt();
    } else if (format.hasProperty( QTextFormatUserDefined::TableCellAllBordersStyle ) ) {
        borderStyle = format.property(QTextFormatUserDefined::TableCellAllBordersStyle).toInt();
    }

    switch( borderStyle ){
    case QTextFrameFormat::BorderStyle_Solid:
        style = "solid";
        break;
    case QTextFrameFormat::BorderStyle_Double:
        style = "double";
        break;
    case QTextFrameFormat::BorderStyle_Dashed:
        style = "dashed";
        break;
    case QTextFrameFormat::BorderStyle_Dotted:
        style = "dotted";
        break;
    case QTextFrameFormat::BorderStyle_None:
        return "none";
        break;
    default:
        return QString();
        break;
    }

    QString width="1.0pt";
    if (format.hasProperty(bWidth) ) {
        double borderWidth = format.property(bWidth).toDouble();
        width = QString::number(borderWidth) + "pt";
    } else if (format.hasProperty(QTextFormatUserDefined::TableCellAllBordersWidth) ) {
        double borderWidth = format.property(QTextFormatUserDefined::TableCellAllBordersWidth).toDouble();
        width = QString::number(borderWidth) + "pt";
    }

    QString color="#000000";
    if (format.hasProperty(bColor) ) {
        QColor col = format.property(bColor).value<QColor>();
        color = col.name();
    } else if (format.hasProperty( QTextFormatUserDefined::TableCellAllBordersColor ) ) {
        QColor col = format.property( QTextFormatUserDefined::TableCellAllBordersColor ).value<QColor>();
        color = col.name();
    }

    return width + " " + style + " " + color;
}

void OdtWriter::writeFormats(QXmlStreamWriter &writer) const
{
    writer.writeStartElement(officeNS, QString::fromLatin1("automatic-styles"));

    QVector<QTextFormat> allStyles = m_d->document->allFormats();

    for( int i=0; i < allStyles.size(); ++i ){
        switch ( allStyles.at(i).type()) {
        case QTextFormat::CharFormat: {
            if( allStyles.at(i).objectType() == QTextFormat::TableCellObject ){
                writeTableCellFormat(writer, allStyles.at(i).toTableCellFormat(), i);
            } else {
                writeCharacterFormat(writer, allStyles.at(i).toCharFormat(), i);
            }
            break;}
        case QTextFormat::BlockFormat:
            writeBlockFormat(writer, allStyles.at(i).toBlockFormat(), i);
            break;
        case QTextFormat::ListFormat:
            writeListFormat(writer, allStyles.at(i).toListFormat(), i);
            break;
        case QTextFormat::FrameFormat:{
            if( allStyles.at(i).objectType() == QTextFormat::TableObject ){
                writeTableFormat(writer, allStyles.at(i).toTableFormat(), i);
            } else {
                writeFrameFormat(writer, allStyles.at(i).toFrameFormat(), i);
            }
            break;}
        case QTextFormat::TableFormat:
            writeTableFormat(writer, allStyles.at(i).toTableFormat(), i);
            break;
        }
    }

    writer.writeEndElement(); // automatic-styles
}

void OdtWriter::writeBlockFormat(QXmlStreamWriter &writer, QTextBlockFormat format, int formatIndex) const
{
    writer.writeStartElement(styleNS, QString::fromLatin1("style"));
    writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("P%1").arg(formatIndex));
    writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("paragraph"));
    writer.writeStartElement(styleNS, QString::fromLatin1("paragraph-properties"));

    if (format.hasProperty(QTextFormat::BlockAlignment)) {
        const Qt::Alignment alignment = format.alignment() & Qt::AlignHorizontal_Mask;
        QString value;
        if (alignment == Qt::AlignLeading)
            value = QString::fromLatin1("start");
        else if (alignment == Qt::AlignTrailing)
            value = QString::fromLatin1("end");
        else if (alignment == (Qt::AlignLeft | Qt::AlignAbsolute))
            value = QString::fromLatin1("left");
        else if (alignment == (Qt::AlignRight | Qt::AlignAbsolute))
            value = QString::fromLatin1("right");
        else if (alignment == Qt::AlignHCenter)
            value = QString::fromLatin1("center");
        else if (alignment == Qt::AlignJustify)
            value = QString::fromLatin1("justify");
        else
            qWarning() << "QOdtWriter: unsupported paragraph alignment; " << format.alignment();
        if (! value.isNull())
            writer.writeAttribute(foNS, QString::fromLatin1("text-align"), value);
    }

    if (format.hasProperty(QTextFormat::BlockTopMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-top"), pixelToPoint(qMax(qreal(0.), format.topMargin())) );
    if (format.hasProperty(QTextFormat::BlockBottomMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-bottom"), pixelToPoint(qMax(qreal(0.), format.bottomMargin())) );
    if (format.hasProperty(QTextFormat::BlockLeftMargin) || format.hasProperty(QTextFormat::BlockIndent))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-left"), pixelToPoint(qMax(qreal(0.),
                                                                                          format.leftMargin() + format.indent())));
    if (format.hasProperty(QTextFormat::BlockRightMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-right"), pixelToPoint(qMax(qreal(0.), format.rightMargin())) );
    if (format.hasProperty(QTextFormat::TextIndent))
        writer.writeAttribute(foNS, QString::fromLatin1("text-indent"), pixelToPoint(format.textIndent()));
    if (format.hasProperty(QTextFormat::PageBreakPolicy)) {
        if (format.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore)
            writer.writeAttribute(foNS, QString::fromLatin1("break-before"), QString::fromLatin1("page"));
        if (format.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysAfter)
            writer.writeAttribute(foNS, QString::fromLatin1("break-after"), QString::fromLatin1("page"));
    }
    if (format.hasProperty(QTextFormat::BackgroundBrush)) {
        QBrush brush = format.background();
        writer.writeAttribute(foNS, QString::fromLatin1("background-color"), brush.color().name());
    }
    if (format.hasProperty(QTextFormat::BlockNonBreakableLines))
        writer.writeAttribute(foNS, QString::fromLatin1("keep-together"),
                              format.nonBreakableLines() ? QString::fromLatin1("true") : QString::fromLatin1("false"));
    if (format.hasProperty(QTextFormat::TabPositions)) {
        QList<QTextOption::Tab> tabs = format.tabPositions();
        writer.writeStartElement(styleNS, QString::fromLatin1("tab-stops"));
        QList<QTextOption::Tab>::Iterator iterator = tabs.begin();
        while(iterator != tabs.end()) {
            writer.writeEmptyElement(styleNS, QString::fromLatin1("tab-stop"));
            writer.writeAttribute(styleNS, QString::fromLatin1("position"), pixelToPoint(iterator->position) );
            QString type;
            switch(iterator->type) {
            case QTextOption::DelimiterTab: type = QString::fromLatin1("char"); break;
            case QTextOption::LeftTab: type = QString::fromLatin1("left"); break;
            case QTextOption::RightTab: type = QString::fromLatin1("right"); break;
            case QTextOption::CenterTab: type = QString::fromLatin1("center"); break;
            }
            writer.writeAttribute(styleNS, QString::fromLatin1("type"), type);
            if (iterator->delimiter != 0)
                writer.writeAttribute(styleNS, QString::fromLatin1("char"), iterator->delimiter);
            ++iterator;
        }

        writer.writeEndElement(); // tab-stops
    }

    writer.writeEndElement(); // paragraph-properties
    writer.writeEndElement(); // style
}

void OdtWriter::writeCharacterFormat(QXmlStreamWriter &writer, QTextCharFormat format, int formatIndex) const {
    writer.writeStartElement(styleNS, QString::fromLatin1("style"));
    writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("C%1").arg(formatIndex));
    writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("text"));
    writer.writeEmptyElement(styleNS, QString::fromLatin1("text-properties"));
    if (format.fontItalic())
        writer.writeAttribute(foNS, QString::fromLatin1("font-style"), QString::fromLatin1("italic"));
    if (format.hasProperty(QTextFormat::FontWeight) && format.fontWeight() != QFont::Normal) {
        QString value;
        if (format.fontWeight() == QFont::Bold)
            value = QString::fromLatin1("bold");
        else
            value = QString::number(format.fontWeight() * 10);
        writer.writeAttribute(foNS, QString::fromLatin1("font-weight"), value);
    }
    if (format.hasProperty(QTextFormat::FontFamily))
        writer.writeAttribute(foNS, QString::fromLatin1("font-family"), format.fontFamily());
    else
        writer.writeAttribute(foNS, QString::fromLatin1("font-family"), QString::fromLatin1("Sans")); // Qt default
    if (format.hasProperty(QTextFormat::FontPointSize))
        writer.writeAttribute(foNS, QString::fromLatin1("font-size"), QString::fromLatin1("%1pt").arg(format.fontPointSize()));
    if (format.hasProperty(QTextFormat::FontCapitalization)) {
        switch(format.fontCapitalization()) {
        case QFont::MixedCase:
            writer.writeAttribute(foNS, QString::fromLatin1("text-transform"), QString::fromLatin1("none")); break;
        case QFont::AllUppercase:
            writer.writeAttribute(foNS, QString::fromLatin1("text-transform"), QString::fromLatin1("uppercase")); break;
        case QFont::AllLowercase:
            writer.writeAttribute(foNS, QString::fromLatin1("text-transform"), QString::fromLatin1("lowercase")); break;
        case QFont::Capitalize:
            writer.writeAttribute(foNS, QString::fromLatin1("text-transform"), QString::fromLatin1("capitalize")); break;
        case QFont::SmallCaps:
            writer.writeAttribute(foNS, QString::fromLatin1("font-variant"), QString::fromLatin1("small-caps")); break;
        }
    }
    if (format.hasProperty(QTextFormat::FontLetterSpacing))
        writer.writeAttribute(foNS, QString::fromLatin1("letter-spacing"), pixelToPoint(format.fontLetterSpacing()));
    if (format.hasProperty(QTextFormat::FontWordSpacing) && format.fontWordSpacing() != 0)
        writer.writeAttribute(foNS, QString::fromLatin1("word-spacing"), pixelToPoint(format.fontWordSpacing()));
    if (format.hasProperty(QTextFormat::FontUnderline))
        writer.writeAttribute(styleNS, QString::fromLatin1("text-underline-type"),
                              format.fontUnderline() ? QString::fromLatin1("single") : QString::fromLatin1("none"));
    if (format.hasProperty(QTextFormat::FontOverline)) {
        // TODO bool   fontOverline () const
    }
    if (format.hasProperty(QTextFormat::FontStrikeOut))
        writer.writeAttribute(styleNS,QString::fromLatin1( "text-line-through-type"),
                              format.fontStrikeOut() ? QString::fromLatin1("single") : QString::fromLatin1("none"));
    if (format.hasProperty(QTextFormat::TextUnderlineColor))
        writer.writeAttribute(styleNS, QString::fromLatin1("text-underline-color"), format.underlineColor().name());
    if (format.hasProperty(QTextFormat::FontFixedPitch)) {
        // TODO bool   fontFixedPitch () const
    }
    if (format.hasProperty(QTextFormat::TextUnderlineStyle)) {
        QString value;
        switch (format.underlineStyle()) {
        case QTextCharFormat::NoUnderline: value = QString::fromLatin1("none"); break;
        case QTextCharFormat::SingleUnderline: value = QString::fromLatin1("solid"); break;
        case QTextCharFormat::DashUnderline: value = QString::fromLatin1("dash"); break;
        case QTextCharFormat::DotLine: value = QString::fromLatin1("dotted"); break;
        case QTextCharFormat::DashDotLine: value = QString::fromLatin1("dash-dot"); break;
        case QTextCharFormat::DashDotDotLine: value = QString::fromLatin1("dot-dot-dash"); break;
        case QTextCharFormat::WaveUnderline: value = QString::fromLatin1("wave"); break;
        case QTextCharFormat::SpellCheckUnderline: value = QString::fromLatin1("none"); break;
        }
        writer.writeAttribute(styleNS, QString::fromLatin1("text-underline-style"), value);
    }
    if (format.hasProperty(QTextFormat::TextVerticalAlignment)) {
        QString value;
        switch (format.verticalAlignment()) {
        case QTextCharFormat::AlignMiddle:
        case QTextCharFormat::AlignNormal: value = QString::fromLatin1("0%"); break;
        case QTextCharFormat::AlignSuperScript: value = QString::fromLatin1("super"); break;
        case QTextCharFormat::AlignSubScript: value = QString::fromLatin1("sub"); break;
        case QTextCharFormat::AlignTop: value = QString::fromLatin1("100%"); break;
        case QTextCharFormat::AlignBottom : value = QString::fromLatin1("-100%"); break;
        case QTextCharFormat::AlignBaseline: break;
        }
        writer.writeAttribute(styleNS, QString::fromLatin1("text-position"), value);
    }
    if (format.hasProperty(QTextFormat::TextOutline))
        writer.writeAttribute(styleNS, QString::fromLatin1("text-outline"), QString::fromLatin1("true"));
    if (format.hasProperty(QTextFormat::TextToolTip)) {
        // TODO QString   toolTip () const
    }
    if (format.hasProperty(QTextFormat::IsAnchor)) {
        // TODO bool   isAnchor () const
    }
    if (format.hasProperty(QTextFormat::AnchorHref)) {
        // TODO QString   anchorHref () const
    }
    if (format.hasProperty(QTextFormat::AnchorName)) {
        // TODO QString   anchorName () const
    }
    if (format.hasProperty(QTextFormat::ForegroundBrush)) {
        QBrush brush = format.foreground();
        writer.writeAttribute(foNS, QString::fromLatin1("color"), brush.color().name());
    }
    if (format.hasProperty(QTextFormat::BackgroundBrush)) {
        QBrush brush = format.background();
        writer.writeAttribute(foNS, QString::fromLatin1("background-color"), brush.color().name());
    }

    writer.writeEndElement(); // style
}

void OdtWriter::writeListFormat(QXmlStreamWriter &writer, QTextListFormat format, int formatIndex) const
{
    writer.writeStartElement(textNS, QString::fromLatin1("list-style"));
    writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("L%1").arg(formatIndex));

    QTextListFormat::Style style = format.style();
    if (style == QTextListFormat::ListDecimal || style == QTextListFormat::ListLowerAlpha
            || style == QTextListFormat::ListUpperAlpha
            || style == QTextListFormat::ListLowerRoman
            || style == QTextListFormat::ListUpperRoman) {
        writer.writeStartElement(textNS, QString::fromLatin1("list-level-style-number"));
        writer.writeAttribute(styleNS, QString::fromLatin1("num-format"), bulletChar(style));

        if (format.hasProperty(QTextFormat::ListNumberSuffix))
            writer.writeAttribute(styleNS, QString::fromLatin1("num-suffix"), format.numberSuffix());
        else
            writer.writeAttribute(styleNS, QString::fromLatin1("num-suffix"), QString::fromLatin1("."));

        if (format.hasProperty(QTextFormat::ListNumberPrefix))
            writer.writeAttribute(styleNS, QString::fromLatin1("num-prefix"), format.numberPrefix());

    } else {
        writer.writeStartElement(textNS, QString::fromLatin1("list-level-style-bullet"));
        writer.writeAttribute(textNS, QString::fromLatin1("bullet-char"), bulletChar(style));
    }

    writer.writeAttribute(textNS, QString::fromLatin1("level"), QString::number(format.indent()));
    writer.writeEmptyElement(styleNS, QString::fromLatin1("list-level-properties"));
    writer.writeAttribute(foNS, QString::fromLatin1("text-align"), QString::fromLatin1("start"));
    QString spacing = QString::fromLatin1("%1mm").arg(format.indent() * 8);
    writer.writeAttribute(textNS, QString::fromLatin1("space-before"), spacing);
    //writer.writeAttribute(textNS, QString::fromLatin1("min-label-width"), spacing);

    writer.writeEndElement(); // list-level-style-*
    writer.writeEndElement(); // list-style
}

void OdtWriter::writeFrameFormat(QXmlStreamWriter &writer, QTextFrameFormat format, int formatIndex) const
{
    writer.writeStartElement(styleNS, QString::fromLatin1("style"));
    writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("s%1").arg(formatIndex));
    writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("section"));
    writer.writeEmptyElement(styleNS, QString::fromLatin1("section-properties"));
    if (format.hasProperty(QTextFormat::FrameTopMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-top"), pixelToPoint(qMax(qreal(0.), format.topMargin())) );
    if (format.hasProperty(QTextFormat::FrameBottomMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-bottom"), pixelToPoint(qMax(qreal(0.), format.bottomMargin())) );
    if (format.hasProperty(QTextFormat::FrameLeftMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-left"), pixelToPoint(qMax(qreal(0.), format.leftMargin())) );
    if (format.hasProperty(QTextFormat::FrameRightMargin))
        writer.writeAttribute(foNS, QString::fromLatin1("margin-right"), pixelToPoint(qMax(qreal(0.), format.rightMargin())) );

    writer.writeEndElement(); // style

    // TODO consider putting the following properties in a qt-namespace.
    // Position   position () const
    // qreal   border () const
    // QBrush   borderBrush () const
    // BorderStyle   borderStyle () const
    // qreal   padding () const
    // QTextLength   width () const
    // QTextLength   height () const
    // PageBreakFlags   pageBreakPolicy () const
}

void OdtWriter::writeTableCellFormat(QXmlStreamWriter &writer, QTextTableCellFormat format, int formatIndex) const
{
    writer.writeStartElement(styleNS, QString::fromLatin1("style"));
    writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("TC%1").arg(formatIndex));
    writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("table-cell"));
    writer.writeEmptyElement(styleNS, QString::fromLatin1("table-cell-properties"));

    qreal padding = format.topPadding();
    if (padding > 0 && padding == format.bottomPadding()
            && padding == format.leftPadding() && padding == format.rightPadding()) {
        writer.writeAttribute(foNS, QString::fromLatin1("padding"), pixelToPoint(padding));
    }
    else {
        if (padding > 0)
            writer.writeAttribute(foNS, QString::fromLatin1("padding-top"), pixelToPoint(padding));
        if (format.bottomPadding() > 0)
            writer.writeAttribute(foNS, QString::fromLatin1("padding-bottom"), pixelToPoint(format.bottomPadding()));
        if (format.leftPadding() > 0)
            writer.writeAttribute(foNS, QString::fromLatin1("padding-left"), pixelToPoint(format.leftPadding()));
        if (format.rightPadding() > 0)
            writer.writeAttribute(foNS, QString::fromLatin1("padding-right"), pixelToPoint(format.rightPadding()));
    }

    if (format.hasProperty(QTextFormat::TextVerticalAlignment)) {
        QString pos;
        switch (format.verticalAlignment()) {
        case QTextCharFormat::AlignMiddle:
            pos = QString::fromLatin1("middle"); break;
        case QTextCharFormat::AlignTop:
            pos = QString::fromLatin1("top"); break;
        case QTextCharFormat::AlignBottom:
            pos = QString::fromLatin1("bottom"); break;
        default:
            pos = QString::fromLatin1("automatic"); break;
        }
        writer.writeAttribute(styleNS, QString::fromLatin1("vertical-align"), pos);
    }

    if (format.hasProperty(QTextFormat::BackgroundBrush)) {
        writer.writeAttribute(foNS, QString::fromLatin1("background-color"), format.background().color().name() );
    }

    QList<int> styleCodes;
    styleCodes << QTextFormatUserDefined::TableCellBorderLeftStyle << QTextFormatUserDefined::TableCellBorderRightStyle << QTextFormatUserDefined::TableCellBorderTopStyle << QTextFormatUserDefined::TableCellBorderBottomStyle;
    QList<int> widthCodes;
    widthCodes << QTextFormatUserDefined::TableCellBorderLeftWidth << QTextFormatUserDefined::TableCellBorderRightWidth << QTextFormatUserDefined::TableCellBorderTopWidth << QTextFormatUserDefined::TableCellBorderBottomWidth;
    QList<int> colCodes;
    colCodes << QTextFormatUserDefined::TableCellBorderLeftColor << QTextFormatUserDefined::TableCellBorderRightColor << QTextFormatUserDefined::TableCellBorderTopColor << QTextFormatUserDefined::TableCellBorderBottomColor;
    QList<QString> attrStrings;
    attrStrings << QString::fromLatin1("border-left") << QString::fromLatin1("border-right") << QString::fromLatin1("border-top") << QString::fromLatin1("border-bottom");

    for( int i=0; i < attrStrings.size(); ++i ){
        QString borderData = tableCellBorderData( styleCodes.at(i), widthCodes.at(i), colCodes.at(i), format );
        if( !borderData.isEmpty() ){
            writer.writeAttribute(foNS, attrStrings.at(i), borderData );
        }
    }
    if (format.hasProperty(QTextFormat::FontPointSize))
        writer.writeAttribute(foNS, QString::fromLatin1("font-size"), QString::fromLatin1("%1pt").arg(format.fontPointSize()));

    // TODO ODF just search for style-table-cell-properties-attlist)
    // QTextFormat::BackgroundImageUrl
    // format.background
    // QTextFormat::FrameBorder

    writer.writeEndElement(); // style
}

void OdtWriter::writeTableFormat(QXmlStreamWriter &writer, QTextTableFormat format, int formatIndex) const {
    writer.writeStartElement(styleNS, QString::fromLatin1("style"));
    writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("T%1").arg(formatIndex));
    writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("table"));
    writer.writeEmptyElement(styleNS, QString::fromLatin1("table-properties"));
    if (format.hasProperty(QTextFormat::BackgroundBrush)){
        // TODO writer.writeAttribute(foNS, QString::fromLatin1("margin-top"), pixelToPoint(qMax(qreal(0.), format.topMargin())) );
    }
    if (format.hasProperty(QTextFormat::BlockAlignment)) {
        const Qt::Alignment alignment = format.alignment() & Qt::AlignHorizontal_Mask;
        QString value;
        if (alignment == Qt::AlignLeft)
            value = QString::fromLatin1("left");
        else if (alignment == Qt::AlignRight )
            value = QString::fromLatin1("right");
        else if (alignment == Qt::AlignHCenter)
            value = QString::fromLatin1("center");
        else {
            qWarning() << "QOdtWriter: unsupported paragraph alignment; " << format.alignment();
            value = QString::fromLatin1("left");
        }
        if (! value.isNull())
            writer.writeAttribute(tableNS, QString::fromLatin1("align"), value);
    }
    if (format.hasProperty(QTextFormat::FrameWidth)) {
        if( format.width().type() == QTextLength::FixedLength ){
            writer.writeAttribute(styleNS, QString::fromLatin1("width"), QString("%1mm").arg(format.width().value( 1000 ) ) );
        } else if( format.width().type() == QTextLength::PercentageLength ){
            writer.writeAttribute(styleNS, QString::fromLatin1("rel-width"), QString("%1%").arg(format.width().value(100)) );
            // writer.writeAttribute(styleNS, QString::fromLatin1("rel-width"), QString("%1\%").arg(format.width().value(100)) );
        }
    }
    writer.writeEndElement();

    for( int i=0; i < format.columns(); ++i ){
        writer.writeStartElement(styleNS, QString::fromLatin1("style"));
        writer.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("T%1.%2").arg( QString::number(formatIndex), QString::number(i)) );
        writer.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("table-column"));
        writer.writeEmptyElement(styleNS, QString::fromLatin1("table-column-properties"));

        if (format.hasProperty(QTextFormat::TableColumnWidthConstraints) ){
            if( i < format.columnWidthConstraints().size() ){
                QTextLength txtLength = format.columnWidthConstraints().at(i);
                if( txtLength.type() == QTextLength::FixedLength ){
                    writer.writeAttribute(styleNS, QString::fromLatin1("column-width"), QString("%1mm").arg(txtLength.value( 100 ) ) );
                } else if( txtLength.type() == QTextLength::PercentageLength ){
                    writer.writeAttribute(styleNS, QString::fromLatin1("rel-column-width"), QString("%1*").arg(txtLength.value(100)) );
                }
            }
        }
        writer.writeEndElement();
    }
}

OdtWriter::OdtWriter(const QTextDocument &document, QIODevice *device)
    : officeNS (QLatin1String("urn:oasis:names:tc:opendocument:xmlns:office:1.0")),
      textNS (QLatin1String("urn:oasis:names:tc:opendocument:xmlns:text:1.0")),
      styleNS (QLatin1String("urn:oasis:names:tc:opendocument:xmlns:style:1.0")),
      foNS (QLatin1String("urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0")),
      tableNS (QLatin1String("urn:oasis:names:tc:opendocument:xmlns:table:1.0")),
      drawNS (QLatin1String("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0")),
      xlinkNS (QLatin1String("http://www.w3.org/1999/xlink")),
      svgNS (QLatin1String("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0")),
      m_d( new OdtWriterPrivate(document, device) ){
}

bool OdtWriter::writeAll() {
    if (m_d->createArchive)
        m_d->strategy = new ZipStreamStrategy(m_d->device);
    else
        m_d->strategy = new QXmlStreamStrategy(m_d->device);

    if (!m_d->device->isWritable() && ! m_d->device->open(QIODevice::WriteOnly)) {
        qWarning() << "OdtWriter::writeAll: the device can not be opened for writing";
        return false;
    }

    QXmlStreamWriter contentWriter(m_d->strategy->contentStream);
#ifndef QT_NO_TEXTCODEC
    if (m_d->codec)
        contentWriter.setCodec(m_d->codec);
#endif
    // prettyfy
    contentWriter.setAutoFormatting(true);
    contentWriter.setAutoFormattingIndent(2);

    contentWriter.writeNamespace(officeNS, QString::fromLatin1("office"));
    contentWriter.writeNamespace(textNS, QString::fromLatin1("text"));
    contentWriter.writeNamespace(styleNS, QString::fromLatin1("style"));
    contentWriter.writeNamespace(foNS, QString::fromLatin1("fo"));
    contentWriter.writeNamespace(tableNS, QString::fromLatin1("table"));
    contentWriter.writeNamespace(drawNS, QString::fromLatin1("draw"));
    contentWriter.writeNamespace(xlinkNS, QString::fromLatin1("xlink"));
    contentWriter.writeNamespace(svgNS, QString::fromLatin1("svg"));
    contentWriter.writeStartDocument();
    contentWriter.writeStartElement(officeNS, QString::fromLatin1("document-content"));
    contentWriter.writeAttribute(officeNS, QString::fromLatin1("version"), QString::fromLatin1("1.2"));

    writeFormats(contentWriter);

    contentWriter.writeStartElement(officeNS, QString::fromLatin1("body"));
    contentWriter.writeStartElement(officeNS, QString::fromLatin1("text"));

    writeFrame(contentWriter, m_d->document->rootFrame());

    contentWriter.writeEndElement(); // text
    contentWriter.writeEndElement(); // body
    contentWriter.writeEndElement(); // document-content
    contentWriter.writeEndDocument();

    // styles

    QXmlStreamWriter  stylesWriter( m_d->strategy->stylesStream );
#ifndef QT_NO_TEXTCODEC
    if (m_d->codec)
        stylesWriter.setCodec(m_d->codec);
#endif
    // prettyfy
    stylesWriter.setAutoFormatting(true);
    stylesWriter.setAutoFormattingIndent(2);

    stylesWriter.writeNamespace(officeNS, QString::fromLatin1("office"));
    stylesWriter.writeNamespace(textNS, QString::fromLatin1("text"));
    stylesWriter.writeNamespace(styleNS, QString::fromLatin1("style"));
    stylesWriter.writeNamespace(foNS, QString::fromLatin1("fo"));
    stylesWriter.writeNamespace(tableNS, QString::fromLatin1("table"));
    stylesWriter.writeNamespace(drawNS, QString::fromLatin1("draw"));
    stylesWriter.writeNamespace(xlinkNS, QString::fromLatin1("xlink"));
    stylesWriter.writeNamespace(svgNS, QString::fromLatin1("svg"));
    stylesWriter.writeStartDocument();
    stylesWriter.writeStartElement(officeNS, QString::fromLatin1("document-styles"));
    stylesWriter.writeAttribute(officeNS, QString::fromLatin1("version"), QString::fromLatin1("1.2"));{
        stylesWriter.writeStartElement(officeNS, QString::fromLatin1("font-face-decls") ); {
            stylesWriter.writeEmptyElement( styleNS, QString::fromLatin1("font-face") );
            stylesWriter.writeAttribute( styleNS, QString::fromLatin1("name"), QString::fromLatin1("Sans"));
            stylesWriter.writeAttribute( svgNS, QString::fromLatin1("font-family"), QString::fromLatin1("Sans"));}
        stylesWriter.writeEndElement(); // font-face-decls

        stylesWriter.writeStartElement(officeNS, QString::fromLatin1("styles"));{
            stylesWriter.writeStartElement(styleNS, QString::fromLatin1("default-style"));{
                stylesWriter.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("paragraph"));
                stylesWriter.writeEmptyElement(styleNS, QString::fromLatin1("paragraph-properties"));
                stylesWriter.writeAttribute(styleNS, QString::fromLatin1("text-autospace"), QString::fromLatin1("ideograph-alpha"));
                stylesWriter.writeAttribute(styleNS, QString::fromLatin1("punctuation-wrap"), QString::fromLatin1("hanging"));
                stylesWriter.writeAttribute(styleNS, QString::fromLatin1("line-break"), QString::fromLatin1("strict"));
                stylesWriter.writeAttribute(styleNS, QString::fromLatin1("writing-mode"), QString::fromLatin1("page"));
                stylesWriter.writeEmptyElement(styleNS, QString::fromLatin1("text-properties"));
                stylesWriter.writeAttribute(foNS, QString::fromLatin1("color"), QString::fromLatin1("#000000"));
                stylesWriter.writeAttribute(styleNS, QString::fromLatin1("font-name"), QString::fromLatin1("Sans"));}
            stylesWriter.writeAttribute(foNS, QString::fromLatin1("font-size"), QString::fromLatin1("%1pt").arg(m_d->defaultPointSize));
            stylesWriter.writeEndElement(); // default-style
            stylesWriter.writeEmptyElement(styleNS, QString::fromLatin1("style"));
            stylesWriter.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("Standard"));
            stylesWriter.writeAttribute(styleNS, QString::fromLatin1("family"), QString::fromLatin1("paragraph"));
            stylesWriter.writeAttribute(styleNS, QString::fromLatin1("class"), QString::fromLatin1("text"));}
        stylesWriter.writeEndElement(); // styles

        stylesWriter.writeStartElement(officeNS, QString::fromLatin1("automatic-styles"));{
            stylesWriter.writeStartElement(styleNS, QString::fromLatin1("page-layout"));
            stylesWriter.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("MainPageLayout") );{
                stylesWriter.writeStartElement(styleNS, QString::fromLatin1("page-layout-properties"));
                if( m_d->pageOrientation == Qt::Horizontal ){
                    stylesWriter.writeAttribute(styleNS, QString::fromLatin1("print-orientation"), QString::fromLatin1("landscape"));
                } else {
                    stylesWriter.writeAttribute(styleNS, QString::fromLatin1("print-orientation"), QString::fromLatin1("portrait"));
                }
                stylesWriter.writeAttribute(foNS, QString::fromLatin1("page-width"), QString::fromLatin1("%1mm").arg(m_d->pageWidth));
                stylesWriter.writeAttribute(foNS, QString::fromLatin1("page-height"), QString::fromLatin1("%1mm").arg(m_d->pageHeight));
                stylesWriter.writeAttribute(foNS, QString::fromLatin1("margin-top"), QString::fromLatin1("%1mm").arg(m_d->marginTop));
                stylesWriter.writeAttribute(foNS, QString::fromLatin1("margin-bottom"), QString::fromLatin1("%1mm").arg(m_d->marginBottom));
                stylesWriter.writeAttribute(foNS, QString::fromLatin1("margin-left"), QString::fromLatin1("%1mm").arg(m_d->marginLeft));
                stylesWriter.writeAttribute(foNS, QString::fromLatin1("margin-right"), QString::fromLatin1("%1mm").arg(m_d->marginRight));
                stylesWriter.writeEndElement(); }// page-layout-properties
            stylesWriter.writeEndElement(); }// page-layout
        stylesWriter.writeEndElement(); // automatic-styles

        stylesWriter.writeStartElement(officeNS, QString::fromLatin1("master-styles"));{
            stylesWriter.writeEmptyElement(styleNS, QString::fromLatin1("master-page"));
            stylesWriter.writeAttribute(styleNS, QString::fromLatin1("name"), QString::fromLatin1("Standard"));
            stylesWriter.writeAttribute(styleNS, QString::fromLatin1("page-layout-name"), QString::fromLatin1("MainPageLayout") );}
        stylesWriter.writeEndElement(); }// master-styles
    stylesWriter.writeEndElement(); // document-styles

    stylesWriter.writeEndDocument();


    delete m_d->strategy;
    m_d->strategy = 0;


    return true;
}

void OdtWriter::setCodec(QTextCodec *codec) { m_d->codec = codec; }

void OdtWriter::setCreateArchive(bool on) { m_d->createArchive = on; }

bool OdtWriter::createArchive() const { return m_d->createArchive; }
