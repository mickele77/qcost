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
#ifndef PRICEITEMDATASETMODEL_H
#define PRICEITEMDATASETMODEL_H

#include "qcost_export.h"

class Bill;
class PriceItem;
class ProjectPriceListParentItem;
class MathParser;
class PriceFieldModel;
class PriceItemDataSetPrivate;
class QXmlStreamWriter;
class QXmlStreamAttributes;
class QString;

#include <QAbstractTableModel>

class PriceItemDataSetModelPrivate;

class EXPORT_QCOST_LIB_OPT PriceItemDataSetModel: public QAbstractTableModel {
    Q_OBJECT

public:
    friend class PriceItem;

    PriceItemDataSetModel(MathParser* prs, PriceFieldModel * pfm, PriceItem * pItem = 0);

    ~PriceItemDataSetModel();

    PriceItemDataSetModel & operator=(const PriceItemDataSetModel & cp );

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int associatedAPRow();

    bool insertPriceDataSet( int column, int count = 1);
    bool appendPriceDataSet( int count = 1 );
    bool appendPriceDataSet( const QXmlStreamAttributes &attrs );
    bool removePriceDataSet( int column, int count = 1);

    double value( int priceField, int priceDataSet ) const;
    QString valueStr( int priceField, int priceDataSet ) const;
    bool setValue(int priceField, int priceDataSet, double newValInput );
    bool setValue( int priceField, int priceDataSet, const QString & newVal );

    bool associateAP( int priceDataSet );
    void setAssociateAP(int priceDataSet, bool v = true );
    Bill * associatedAP( int priceDataSet );

    double overheads( int priceDataSet ) const;
    QString overheadsStr( int priceDataSet ) const;
    void setOverheads( int priceDataSet, double newVal );
    void setOverheads( int priceDataSet, const QString & newVal );
    bool inheritOverheadsFromRoot( int priceDataSet );
    void setInheritOverheadsFromRoot( int priceDataSet, bool newVal );

    double profits( int priceDataSet ) const;
    QString profitsStr( int priceDataSet ) const;
    void setProfits( int priceDataSet, double newVal );
    void setProfits( int priceDataSet, const QString & newVal );
    bool inheritProfitsFromRoot( int priceDataSet );
    void setInheritProfitsFromRoot( int priceDataSet, bool newVal );

    void writeXml10(QXmlStreamWriter *writer) const;
    void writeXml20( QXmlStreamWriter * writer ) const;
    void loadXmlPriceDataSet(int priceDataSet, const QXmlStreamAttributes &attrs);
    void readFromXmlTmp( ProjectPriceListParentItem *priceLists );

    int priceDataSetCount() const ;
    int lastValueRow() const;
    int associatedAPRow() const;

signals:
    void modelChanged();

    // segnale emesso quando cambia il valore del prezzo
    void valueChanged( int priceField, int priceDataSet, double newVal );

    void priceDataSetCountChanged( int newPriceColCount );

    void beginInsertPriceDataSets(int first, int last);
    void endInsertPriceDataSets(int first, int last);
    void beginRemovePriceDataSets(int first, int last);
    void endRemovePriceDataSets(int first, int last);

    void beginInsertPriceField( int first, int last );
    void endInsertPriceField( int first, int last );
    void beginRemovePriceField( int first, int last );
    void endRemovePriceField( int first, int last );

    void overheadsChanged( int priceDataSet, const QString & newVal );
    void inheritOverheadsFromRootChanged( int priceDataSet, bool newVal );
    void profitsChanged( int priceDataSet, const QString & newVal );
    void inheritProfitsFromRootChanged( int priceDataSet, bool newVal );

private:
    PriceItemDataSetModelPrivate * m_d;

    bool insertPriceDataSetPrivate(int columnInput, int count = 1);
    bool removePriceDataSetPrivate(int column, int count = 1);

    double overheadsFromRoot(int priceDataSet, PriceItem *pItem) const;
    void setOverheadsToRoot(int priceDataSet, double newVal, PriceItem *pItem);
    void setOverheadsFromRoot(int priceDataSet, double newVal);

    double profitsFromRoot(int priceDataSet, PriceItem *pItem) const;
    void setProfitsToRoot(int priceDataSet, double newVal, PriceItem *pItem);
    void setProfitsFromRoot(int priceDataSet, double newVal);

private slots:
    void setValueFromAP(int priceField, double v);

    void insertPriceField(int row);
    void removePriceField(int row);

    void updateValueFormula(int priceField);
};

#endif // PRICEITEMDATASETMODEL_H
