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
#ifndef PRICEFIELDMODEL_H
#define PRICEFIELDMODEL_H

#include "library_common.h"

class QXmlStreamWriter;
class QXmlStreamReader;
class QXmlStreamAttributes;

class PriceFieldModelPrivate;
class MathParser;

#include <QAbstractTableModel>

class EXPORT_LIB_OPT PriceFieldModel: public QAbstractTableModel {
    Q_OBJECT
public:
    enum FieldType{
        None,
        PriceTotal,
        PriceHuman,
        PriceHumanNet,
        PriceNoHumanNet,
        PriceEquipment,
        PriceMaterial
    };

    static QList< QPair<FieldType, QString> > standardFieldTypeNames();
    static int fieldTypeCol();

    PriceFieldModel( MathParser * prs, QObject * parent = 0 );
    virtual ~PriceFieldModel();

    PriceFieldModel &operator =(const PriceFieldModel &cp);

    QList<QString> fieldNames();

    void createHumanNetPriceFields();

    int fieldCount();
    QString priceName(int pf );
    bool setPriceName(int pf, const QString & newName );
    QString amountName(int pf );
    bool setAmountName(int pf, const QString & newName );
    QString unitMeasure( int pf );
    bool setUnitMeasure(int pf, const QString & newVal );
    int precision( int pf );
    bool setPrecision(int pf, int newVal );
    bool applyFormula( int pf );
    bool setApplyFormula(int pf, bool newVal );
    QString formula( int pf );
    bool setFormula(int pf, const QString & newVal );
    FieldType fieldType( int pf );
    bool setFieldType(int pf, FieldType newVal, bool resetField=true );

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertRows(int row = -1, int count = 1, const QModelIndex &parent = QModelIndex() );
    bool appendRow();
    bool removeRows(int row = -1, int count = 1, const QModelIndex &parent = QModelIndex() );
    bool clear();
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow);

    void writeXml( QXmlStreamWriter * writer );
    void readXml(QXmlStreamReader *reader);

    double calcFormula( bool * ok, int field, QList<double> fieldValues, double overheads, double profits );

signals:
    void modelChanged();

    void beginInsertPriceField( int firstPFInserted,  int lastPFInserted );
    void endInsertPriceField( int firstPFInserted,  int lastPFInserted );
    void beginRemovePriceField( int firstPFRemoved, int lastPFRemoved );
    void endRemovePriceField( int firstPFRemoved, int lastPFRemoved );

    void priceNameChanged( int priceField, const QString & newName );
    void amountNameChanged( int priceField, const QString & newName );
    void unitMeasureChanged( int pf, const QString & newUnitMeasure );
    void precisionChanged( int pf, int newPrec );
    void applyFormulaChanged( int pf, bool newApplyFormula );
    void formulaChanged( int pf, const QString & newFormula );
    void formulaChanged( int pf );
    void fieldTypeChanged( int pf, FieldType newFieldType );

private:
    PriceFieldModelPrivate * m_d;
    void loadFromXml(int pf, const QXmlStreamAttributes &attrs);
};


#endif // PRICEFIELDMODEL_H
