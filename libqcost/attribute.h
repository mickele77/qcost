/*
   QCost is a cost estimating software.
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
#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "qcost_export.h"

class Bill;
class BillItem;
class MathParser;
class PriceFieldModel;

class QTextStream;
class QXmlStreamAttributes;
class QXmlStreamWriter;
class QString;

class AttributePrivate;

class EXPORT_QCOST_LIB_OPT Attribute {

public:
    friend class AttributesModel;

    explicit Attribute(MathParser * prs , PriceFieldModel *pfm);
    ~Attribute();

    Attribute & operator= (const Attribute & cp );

    QString name();
    void setName( const QString & n );

    unsigned int id();

    void writeXml( QXmlStreamWriter * writer );
    void loadFromXml(const QXmlStreamAttributes &attrs);

private:
    AttributePrivate * m_d;

    void nextId();
};

#endif // ATTRIBUTE_H
