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
#ifndef BILL_H
#define BILL_H

#include "qcost_export.h"

class ProjectPriceListParentItem;
class PriceList;
class PriceItem;
class BillItem;
class Attribute;
class AttributeModel;
class PriceFieldModel;
class MathParser;
class QXmlStreamAttributes;
class QXmlStreamWriter;
class QXmlStreamReader;
class QTextStream;
class QString;
class QTextCursor;

#include "billprinter.h"
#include "projectitem.h"
#include <QAbstractItemModel>

class BillPrivate;

class EXPORT_QCOST_LIB_OPT Bill : public QAbstractItemModel, public ProjectItem {
    Q_OBJECT
public:
    enum SetPriceListMode{
        None,
        SearchAndAdd,
        Add,
        Search,
        NULLPriceItem,
        ResetBill
    };

    Bill(const QString &n, ProjectItem *parent, PriceFieldModel *pfm, MathParser * parser = NULL );
    Bill( Bill & );

    Bill & operator= (const Bill & cp );

    virtual ~Bill();

    QString name();
    QString description();

    ProjectItem *child(int number);
    int childCount() const;

    int childNumber( ProjectItem * item );

    bool reset();
    bool canChildrenBeInserted();
    bool insertChildren(int position, int count);
    bool removeChildren(int position, int count);

    Qt::ItemFlags flags() const;
    QVariant data() const;
    bool setData( const QVariant &value);

    void setPriceList( PriceList * pl, SetPriceListMode plMode = SearchAndAdd );
    PriceList * priceList();

    int priceDataSet();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertBillItems(PriceItem * p, int position = -1, int count = 1, const QModelIndex &parent = QModelIndex() );
    bool removeBillItems(int position = -1, int rows = 1, const QModelIndex &parent = QModelIndex() );

    BillItem *billItem(const QModelIndex &index ) const;
    BillItem *billItem(int childNum, const QModelIndex &parentIndex = QModelIndex() );
    BillItem *lastBillItem( const QModelIndex &parentIndex = QModelIndex() );

    BillItem *billItemId(unsigned int itemId);
    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex index(BillItem *item, int column) const;

    bool moveRows(const QModelIndex & sourceParent, int sourceRow, int count, const QModelIndex & destinationParent, int destinationChild);

    double amount( int field ) const ;
    QString amountStr( int field ) const ;

    AttributeModel * attributeModel();
    double amountAttribute( Attribute * attr, int field );
    QString amountAttributeStr( Attribute * attr, int field );

    bool isUsingPriceItem( PriceItem * p );
    bool isUsingPriceList( PriceList * pl );

    void writeXml( QXmlStreamWriter * writer );
    void readXml(QXmlStreamReader *reader, ProjectPriceListParentItem * priceLists);
    void readXmlTmp(QXmlStreamReader *reader);
    void loadFromXml(const QXmlStreamAttributes &attrs, ProjectPriceListParentItem * priceLists);
    void loadTmpData( PriceList *priceList );

    void nextId();
    unsigned int id();

    void loadTmpData( ProjectPriceListParentItem *priceLists );
    void loadFromXmlTmp(const QXmlStreamAttributes &attrs);
    void setTmpData(ProjectPriceListParentItem *priceLists);

    QList<PriceItem *> connectedPriceItems();

    void writeODTBillOnTable( QTextCursor * cursor,
                              BillPrinter::PrintBillItemsOption prItemsOption,
                              QList<int> fieldsToPrint,
                              bool groupPrAm = false );
    void writeODTSummaryOnTable( QTextCursor * cursor,
                                 BillPrinter::PrintBillItemsOption prItemsOption,
                                 QList<int> fieldsToPrint,
                                 bool groupPrAm = false ,
                                 bool writeDetails = true );

    void writeODTAttributeBillOnTable( QTextCursor *cursor,
                                       BillPrinter::AttributePrintOption prOption,
                                       BillPrinter::PrintBillItemsOption prItemsOption,
                                       const QList<int> &fieldsToPrint,
                                       const QList<Attribute *> &attrsToPrint,
                                       bool groupPrAm = false );
    void insertStandardAttributes();

public slots:
    void setName( const QString & n);
    void setDescription( const QString & value );
    void setPriceDataSet( int );

signals:
    void aboutToBeDeleted();
    void nameChanged(  const QString & );
    void descriptionChanged(  const QString & );
    void priceListChanged( PriceList * );

    void amountChanged( int field, const QString & newVal );
    void amountChanged( int field, double newVal );

    void modelChanged();

private slots:
    void updateValue(BillItem *item, int column);
    void insertPriceField( int firstPFInserted, int lastPFInserted );
    void removePriceField( int firstPFRemoved, int lastPFRemoved );
private:
    BillPrivate * m_d;
};

#endif // BILL_H
