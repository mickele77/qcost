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
#include "unitmeasuremodel.h"

#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlDatabase>

class UnitMeasureModelPrivate{
public:
    UnitMeasureModelPrivate( QSqlDatabase * d):
        db(d){
    }
    ~UnitMeasureModelPrivate(){
    }
    int querySize( QSqlQuery * query ){
        if( db->driver()->hasFeature( QSqlDriver::QuerySize )){
            return query->size();
        } else {
            int ret = 0;
            if( query->isSelect() ){
                query->seek(0);
                if( query->isValid() ){
                    ret++;
                }
                while( query->next() ){
                    ret++;
                }
                query->seek(0);
            }
            return ret;
        }
    }

    QSqlDatabase * db;
    QList<QString> pendingTransactions;
    static int nextId;
};

int UnitMeasureModelPrivate::nextId = 1;

UnitMeasureModel::UnitMeasureModel(QSqlDatabase * db, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new UnitMeasureModelPrivate(db)) {
    QString queryStr = "INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (0, 1.0, '---')";
    m_d->db->exec( queryStr );
}

QModelIndex UnitMeasureModel::index(int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(parent)
    QString queryStr = QString("SELECT unitMeasureId, unitMeasureOrderNum FROM unitMeasureTable ORDER BY unitMeasureOrderNum");
    QSqlQuery query = m_d->db->exec( queryStr );
    if (query.seek(row))
        return createIndex(row, column, query.value("unitMeasureId").toUInt());
    return QModelIndex();
}

int UnitMeasureModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    QString queryStr = QString( "SELECT unitMeasureId FROM unitMeasureTable");
    QSqlQuery query = m_d->db->exec( queryStr );
    return m_d->querySize( &query );
}

int UnitMeasureModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 1;
}

Qt::ItemFlags UnitMeasureModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return QAbstractTableModel::flags( index );
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant UnitMeasureModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() )
        return QVariant();

    if( role == Qt::DisplayRole || role == Qt::EditRole ){
        QString queryStr;
        queryStr = QString("SELECT unitMeasureTag, unitMeasureId, unitMeasureOrderNum FROM unitMeasureTable WHERE unitMeasureId=%1").arg( index.internalId() );
        QSqlQuery query = m_d->db->exec( queryStr );
        if( query.next() ){
            return query.value(0);
        } else {
            return QVariant();
        }
    }
    return QVariant();
}

bool UnitMeasureModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || role != Qt::EditRole || index.column() != 0)
        return false;

    QString queryStr = QString("UPDATE unitMeasureTable SET unitMeasureTag='%1' WHERE unitMeasureId=%2").arg( value.toString(), QString::number( index.internalId()));
    execTransaction( queryStr );
    emit dataChanged(index, index);
    return true;
}

QVariant UnitMeasureModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role == Qt::DisplayRole){
        if (orientation == Qt::Horizontal ){
            return QVariant( tr("Unità di misura") );
        }
        if (orientation == Qt::Vertical ){
            return QVariant( section+1 );
        }
    }
    return QVariant();
}

bool UnitMeasureModel::insertRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED( parent )
    if( row < 0 || count < 1 ){
        return false;
    }

    QString queryStr = QString("SELECT unitMeasureId, unitMeasureOrderNum FROM unitMeasureTable ORDER BY unitMeasureOrderNum");
    QSqlQuery queryVerify = m_d->db->exec( queryStr );
    if( queryVerify.seek(row) ){

        QString queryStr = QString("SELECT unitMeasureTag, unitMeasureId, unitMeasureOrderNum FROM unitMeasureTable WHERE unitMeasureId=%1").arg( queryVerify.value(0).toString() );
        QSqlQuery query = m_d->db->exec( queryStr );

        double orderNum = 1.0;
        if( row == 0 ){
            if( query.seek(1)){
                orderNum = query.value("unitMeasureOrderNum").toDouble() - 1.0;
            }
        } else {
            if( query.seek(row-1)){
                orderNum = query.value("unitMeasureOrderNum").toDouble();
                if( query.seek(row)){
                    orderNum = (orderNum + query.value("unitMeasureOrderNum").toDouble()) * 0.50;
                } else {
                    orderNum += 1.0;
                }
            }
        }

        queryStr = QString("INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (%1, %2, %3)").arg(
                    QString::number(nextId()), QString::number( orderNum ), QString("---") );
        execTransaction( queryStr );
        return true;
    }

    return false;
}

bool UnitMeasureModel::removeRows(int row, int count, const QModelIndex &parent) {
    if( count < 1 || row < 0 ){
        return false;
    }

    QString queryStr = QString("SELECT unitMeasureId, unitMeasureOrderNum FROM unitMeasureTable ORDER BY unitMeasureOrderNum" );
    QSqlQuery query = m_d->db->exec( queryStr );

    for( int i=row+count-1; i >= row; --i){
        if( query.seek(i) ){
            queryStr = QString("SELECT unitMeasure FROM priceListTable WHERE unitMeasure=%1" ).arg( query.value("unitMeasureId").toString() );
            QSqlQuery queryVerify = m_d->db->exec( queryStr );
            if( !queryVerify.next() ){
                beginRemoveRows( parent, i, i);
                queryStr = QString("DELETE FROM unitMeasureTable WHERE unitMeasureId=%1").arg( query.value("unitMeasureId").toString() );
                execTransaction( queryStr );
                endRemoveRows();
            }
        }
    }
    return true;
}

bool UnitMeasureModel::appendRow(int *newId, const QString &newTag) {
    QString queryStr = QString("SELECT unitMeasureOrderNum FROM unitMeasureTable ORDER BY unitMeasureOrderNum");
    QSqlQuery query = m_d->db->exec( queryStr );
    if( query.last() ){
        double orderNum = query.value(0).toDouble() + 1.0;
        *newId = nextId();
        queryStr = QString("INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (%1, %2, '%3')").arg(
                    QString::number(*newId), QString::number( orderNum ), newTag );
        beginResetModel();
        execTransaction( queryStr );
        endResetModel();
        return true;
    }

    return false;
}

void UnitMeasureModel::clear() {
    beginResetModel();
    QString queryStr( "DELETE FROM unitMeasureTable" );
    m_d->db->exec( queryStr );
    queryStr = "INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (0, 1.0, '---')";
    m_d->db->exec( queryStr );
    endResetModel();
    m_d->pendingTransactions.clear();
}

int UnitMeasureModel::nextId() {
    int ret = (m_d->nextId)++;
    QString queryStr = QString("SELECT unitMeasureId FROM unitMeasureTable WHERE id=%1").arg( ret );
    QSqlQuery query = m_d->db->exec( queryStr );
    while( query.next() ){
        ret = (m_d->nextId)++;
        queryStr = QString("SELECT unitMeasureId FROM unitMeasureTable WHERE id=%1").arg( ret );
        query.exec( queryStr );
    }
    return ret;
}

QSqlQuery UnitMeasureModel::execTransaction(const QString &queryStr) {
    QSqlQuery query = m_d->db->exec( queryStr );

    QStringList changingCommands;
    changingCommands << "CREATE" << "INSERT" << "DELETE" << "UPDATE";
    QString queryStrUp = queryStr.toUpper();
    for( int i=0; i< changingCommands.size(); ++i ){
        if( queryStrUp.startsWith(changingCommands.at(i)) ){
            m_d->pendingTransactions.append( queryStr );
            emit modelChanged(true);
            break;
        }
    }

    return query;
}

bool UnitMeasureModel::execPendingTransaction() {
    if( m_d->pendingTransactions.size() > 0 ){
        QString queryStr;
        for( int i=0; i < m_d->pendingTransactions.size(); ++i ){
            queryStr = m_d->pendingTransactions.at(i);
            queryStr.replace( "unitMeasureTable", "fileDB.unitMeasureTable");
            queryStr.replace( "priceListTable", "fileDB.priceListTable");
            m_d->db->exec( queryStr );
        }
        m_d->pendingTransactions.clear();
        return true;
    }
    return false;

}
