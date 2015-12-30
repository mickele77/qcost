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

#ifndef PRICEITEM_H
#define PRICEITEM_H

#include "qcost_export.h"

class ProjectPriceListParentItem;
class Bill;
class PriceItemDataSetModel;
class PriceItemDataSet;

class UnitMeasureModel;
class UnitMeasure;
class MathParser;
class PriceFieldModel;

class QXmlStreamAttributes;
class QXmlStreamReader;
class QXmlStreamWriter;
class QTextCursor;
class QTextTableCellFormat;
class QTextStream;
class QString;

#include "pricelistprinter.h"
#include "pricefieldmodel.h"

#include <QObject>
#include "treeitem.h"

class PriceItemPrivate;

class EXPORT_QCOST_LIB_OPT PriceItem : public QObject, public TreeItem {
    Q_OBJECT
public:
    friend class PriceItemDataSetModel;
    PriceItem( PriceItem * parentItem, PriceFieldModel * pfm, MathParser * prs = NULL  );
    ~PriceItem();

    PriceItem & operator=(const PriceItem &cp);

    PriceItem *parentItem();
    void setParentItem(PriceItem *newParent, int position = -1 );
    bool isDescending(PriceItem *ancestor);

    // spese generali
    double overheads(int priceDataSet);
    // spese generali stringa
    QString overheadsStr(int priceDataSet);
    // imposta spese generali
    void setOverheads( int priceDataSet, double newVal);
    // imposta spese generali
    void setOverheads(int priceDataSet, const QString & newVal );
    // ci dice se l'oggetto eredita le spese generali dall'oggetto radice
    bool inheritOverheadsFromRoot( int priceDataSet );
    // Imposta l'ereditarieta' delle spese generali dall'oggetto radice
    void setInheritOverheadsFromRoot(int priceDataSet, bool newVal = true );

    // utili di impresa
    double profits(int priceDataSet);
    // utili di impresa string
    QString profitsStr(int priceDataSet);
    // imposta utili di impresa
    void setProfits(int priceDataSet, double newVal);
    // imposta utili di impresa
    void setProfits(int priceDataSet, const QString & newVal );
    // ci dice se l'oggetto eredita gli utili dall'oggetto radice
    bool inheritProfitsFromRoot(int priceDataSet);
    // imposta l'ereditarieta' degli utili dall'oggetto radice
    void setInheritProfitsFromRoot(int priceDataSet, bool newVal = true );

    unsigned int id();

    PriceFieldModel *priceFieldModel();

    PriceItemDataSetModel * dataModel();

    int priceDataSetCount();

    int columnCount() const;

    TreeItem *child(int number);
    PriceItem * childItem(int number);
    PriceItem * lastChild();
    int childrenCount() const;
    bool hasChildren() const;
    bool insertChildren(int position, int count=1);
    bool appendChildren( int count = 1);
    bool removeChildren(int position, int count=1);
    int childNumber() const;
    QList<PriceItem *> allChildrenList();
    PriceItem *priceItemId( unsigned int dd );
    PriceItem *priceItemIdChildren(unsigned int dd);
    PriceItem *priceItemFullCode(const QString &c);
    PriceItem *priceItemFullCodeChildren(const QString &c);

    Qt::ItemFlags flags(int column) const;
    QVariant data(int column, int role = Qt::DisplayRole ) const;
    bool setData(int column, const QVariant &value );

    QString code() const;
    QString codeFull() const;
    bool inheritCodeFromParent();
    QString shortDescription();
    QString shortDescriptionFull();
    bool inheritShortDescFromParent();
    QString longDescription();
    QString longDescriptionFull();
    bool inheritLongDescFromParent();

    UnitMeasure * unitMeasure();
    void setUnitMeasure( UnitMeasure * ump );
    bool isUsingUnitMeasure( UnitMeasure * );

    int firstValueCol();
    double value(int priceField, int priceDataSet ) const;
    QString valueStr( int priceField, int priceDataSet ) const;
    bool setValue( int priceField, int priceDataSet, double newValue );
    bool setValue( int priceField, int priceDataSet, const QString & newValue );
    void setValue( PriceFieldModel::FieldType fType, int priceDataSet, double newValue );
    void emitValueChanged( int priceField, int priceDataSet, double newVal );

    bool associateAP(int priceDataSet );
    void setAssociateAP(int priceDataSet, bool newValue );
    Bill *associatedAP( int priceDataSet );
    QList<PriceItem *> connectedPriceItems();

    void writeXml( QXmlStreamWriter * writer );
    void readXml(QXmlStreamReader *reader, UnitMeasureModel *uml);
    void loadFromXml(const QXmlStreamAttributes &attrs, UnitMeasureModel *uml);
    void readFromXmlTmp(ProjectPriceListParentItem *priceLists);

    void writeODTOnTable(QTextCursor * cursor, PriceListPrinter::PrintPriceItemsOption printOption, const QList<int> fieldsToPrint, int priceDataSetToPrint = 0);

    static QString codeSeparator();
    static QString shortDescSeparator();
    static QString longDescSeparator();

public slots:
    void setCode( const QString & );
    void setInheritCodeFromParent( bool );
    void setShortDescription( const QString & );
    void setInheritShortDescFromParent( bool );
    void setLongDescription( const QString & );
    void setInheritLongDescFromParent( bool );

signals:
    void aboutToBeDeleted();

    void itemChanged();

    void hasChildrenChanged( PriceItem *, QList<int> );
    void hasChildrenChanged( bool );

    void dataChanged( PriceItem *, int column);
    void unitMeasureChanged( UnitMeasure * );

    void profitsChanged(int pricaDataSet, const QString & newVal );
    void inheritProfitsFromRootChanged( int pricaDataSet, bool newVal );
    void overheadsChanged(int priceDataSet, const QString & newVal );
    void inheritOverheadsFromRootChanged( int priceDataSet, bool newVal );

    void codeChanged(  const QString & );
    void codeFullChanged(  const QString &  );
    void inheritCodeFromParentChanged( bool );
    void shortDescriptionChanged( const QString & );
    void shortDescriptionFullChanged( const QString & );
    void inheritShortDescFromParentChanged( bool );
    void longDescriptionChanged( const QString & );
    void longDescriptionFullChanged( const QString & );
    void inheritLongDescFromParentChanged( bool );
    void valueChanged( int priceField, int priceDataSet, const QString & newValue);
    void associateAPChanged( int priceDataSet, bool newValue);

    void priceDataSetCountChanged( int newPriceDataSetCount );

    void beginInsertPriceDataSets(int first, int last);
    void endInsertPriceDataSets(int first, int last);
    void beginRemovePriceDataSets(int first, int last);
    void endRemovePriceDataSets(int first, int last);

private:
    PriceItemPrivate * m_d;

    void addChild(PriceItem *newChild, int position);
    void removeChild(int position);
    void setHasChildrenChanged(PriceItem * p , QList<int> indexes);
    TreeItem * parentInternal();

    void setId( unsigned int ii );

    QString nextCode();
    QString giveMeUniqueCode();

    void emitAssociateAPChanged(int col, bool newVal );

private slots:
    void emitCodeFullChanged( const QString & );
    void emitShortDescFullChanged( const QString & );
    void emitLongDescFullChanged(const QString & str);
};

#endif // PRICEITEM_H
