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
#ifndef ATTRIBUTESMODEL_H
#define ATTRIBUTESMODEL_H

#include "qcost_export.h"

class AccountingLSBill;
class AccountingLSBills;
class AccountingTAMBill;
class AccountingBill;
class Bill;
class Attribute;
class MathParser;
class PriceFieldModel;

class QTextStream;
class QXmlStreamReader;
class QXmlStreamWriter;

#include <QAbstractTableModel>

class AttributesModelPrivate;

class EXPORT_QCOST_LIB_OPT AttributesModel : public QAbstractTableModel {
    Q_OBJECT
public:
    friend class Project;
    explicit AttributesModel( Bill * myBill, MathParser * prs, PriceFieldModel *pfm, QObject *parent = 0);
    explicit AttributesModel( AccountingBill * myBill, MathParser * prs, PriceFieldModel *pfm, QObject *parent = 0);
    explicit AttributesModel( AccountingTAMBill * myBill, MathParser * prs, PriceFieldModel *pfm, QObject *parent = 0);
    explicit AttributesModel( AccountingLSBill * myBill, MathParser * prs, PriceFieldModel *pfm, QObject *parent = 0);
    explicit AttributesModel( AccountingLSBills * myBills, MathParser * prs, PriceFieldModel *pfm, QObject *parent = 0);
    ~AttributesModel();

    AttributesModel & operator= (const AttributesModel & cp );

    int size();
    Attribute * attribute( int i );
    Attribute * attributeId( unsigned int id );

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

    void writeXml10(QXmlStreamWriter *writer) const;
    void writeXml20( QXmlStreamWriter * writer ) const;
    void readXml( QXmlStreamReader * reader );

    /** nel caso il modello sia riferito alla classe Bill, inserice due etichette
      * una per gli importi soggetti a ribasso ed una per quelli non soggetti. */
    void insertStandardAttributes();

    void setBill(Bill *b);
    void setBill(AccountingBill *b);
    void setBill(AccountingTAMBill *b);
    void setBill(AccountingLSBill *b);
    void setBill(AccountingLSBills *b);

signals:
    void modelChanged();
    void aboutToBeDeleted();

private:
    AttributesModelPrivate * m_d;
};

#endif // ATTRIBUTESMODEL_H
