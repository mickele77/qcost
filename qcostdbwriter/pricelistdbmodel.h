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
#ifndef PRICELISTDBMODEL_H
#define PRICELISTDBMODEL_H

class QTextStream;
class QSqlQuery;
class QLocale;
class UnitMeasureModel;

#include <QAbstractItemModel>

class PriceListDBModelPrivate;

class PriceListDBModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum PriceColType{
        nullCol,
        codeCol,
        shortDescCol,
        longDescCol,
        unitMeasureCol,
        priceTotalCol,
        priceSafetyCol,
        priceHumanCol,
        perPriceHumanCol,
        priceEquipmentCol,
        perPriceEquipmentCol,
        priceMaterialCol,
        perPriceMaterialCol,
        overheadsCol,
        profitsCol
    };
    explicit PriceListDBModel(QLocale * l, QObject *parent = 0);
    ~PriceListDBModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

    QList< QPair<QString, int> > unitMeasureList();
    UnitMeasureModel * unitMeasureModel();

    void clear();

    QString code( const QModelIndex &index );
    QString shortDescription( const QModelIndex &index );
    QString longDescription( const QModelIndex &index );
    int unitMeasure( const QModelIndex &index );
    double priceTotal( const QModelIndex &index );
    double priceSafety(const QModelIndex &index);
    double priceHuman( const QModelIndex &index );
    double priceEquipment( const QModelIndex &index );
    double priceMaterial( const QModelIndex &index );
    double overheads( const QModelIndex &index );
    double profits( const QModelIndex &index );

    void setCode( const QString & v, const QPersistentModelIndex &index );
    void setShortDescription( const QString & v, const QPersistentModelIndex &index );
    void setLongDescription( const QString & v, const QPersistentModelIndex &index );
    void setUnitMeasure( int v, const QPersistentModelIndex &index );
    void setPriceTotal( double v, const QPersistentModelIndex &index );
    void setPriceSafety( double v, const QPersistentModelIndex &index );
    void setPriceHuman( double v, const QPersistentModelIndex &index );
    void setPriceEquipment( double v, const QPersistentModelIndex &index );
    void setPriceMaterial( double v, const QPersistentModelIndex &index );
    void setOverheads( double v, const QPersistentModelIndex &index );
    void setProfits( double v, const QPersistentModelIndex &index );

    void importFromDB(const QString &fileName);
    void importFromTXT(const QString &decimalSeparator, const QString &thousandSeparator, double ovh, double prf, bool setShortDescFromLong, QList<PriceColType> *pCols, QTextStream * in );

    QList< QPair<PriceColType, QString> > inputColsUserName();

    QString currentFile();
    bool setCurrentFile(const QString &newFile, bool openExisting = false );
    bool saveCurrentFile();

    int unitMeasureColumn();

    QLocale * locale();

signals:
    void modelChanged( bool );

private:
    PriceListDBModelPrivate * m_d;

    int nextId();
    void deleteChildren( const QModelIndex & parent );
    QSqlQuery execTransaction( const QString & queryStr );
    bool loadFromDBChildren(int fileParentId, int parentId, const QString &importFileDBName);
    QStringList loadFromTXTChildren(int *progNumber, QStringList line, int parentId, QString parentCode, const QString &decimalSeparator, const QString &thousandSeparator, double ovh, double prf, bool setShortDescFromLong, QList<PriceColType> *pCols, QTextStream *input);
    QStringList readLineFromTXT(int nCol, QTextStream *input);
};

#endif // PRICELISTDBMODEL_H
