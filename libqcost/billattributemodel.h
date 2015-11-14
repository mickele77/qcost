/*
   QCost is a cost estimating software.
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
#ifndef BILLATTRIBUTEMODEL_H
#define BILLATTRIBUTEMODEL_H

#include "qcost_export.h"

class Bill;
class BillAttribute;
class MathParser;
class PriceFieldModel;

class QTextStream;
class QXmlStreamReader;
class QXmlStreamWriter;

#include <QAbstractTableModel>

class BillAttributeModelPrivate;

class EXPORT_QCOST_LIB_OPT BillAttributeModel : public QAbstractTableModel {
    Q_OBJECT
public:
    friend class Project;
    explicit BillAttributeModel( Bill * myBill, MathParser * prs, PriceFieldModel *pfm, QObject *parent = 0);
    ~BillAttributeModel();

    int size();
    BillAttribute * attribute( int i );
    BillAttribute * attributeId( unsigned int id );

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex())  const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & attribute, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool insertRows(int row, int count=1);
    bool append();

    bool removeRows(int row, int count=1);
    bool clear();

    void writeXml( QXmlStreamWriter * writer );
    void readXml( QXmlStreamReader * reader );

    void insertStandardAttributes();
signals:
    void modelChanged();
    void aboutToBeDeleted();

private:
    BillAttributeModelPrivate * m_d;
};

#endif // BILLATTRIBUTEMODEL_H
