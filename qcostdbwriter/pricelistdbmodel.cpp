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
#include "pricelistdbmodel.h"

#include "unitmeasuremodel.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlError>
#include <QTextStream>

#include <QDebug>

class TableCol{
public:
    TableCol( PriceListDBModel::PriceColType pct, const QString & n, const QString & un ):
        priceColType(pct),
        sqlName(n),
        userName(un){
    }
    bool operator==(const TableCol &other) const {
        return priceColType == other.priceColType;
    }
    PriceListDBModel::PriceColType priceColType;
    QString sqlName;
    QString userName;
};

class PriceListDBModelPrivate{
public:
    PriceListDBModelPrivate( QLocale * l ):
        locale(l){
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(":memory:");
        db.open();

        createUMTableQuery = "unitMeasureId INTEGER PRIMARY KEY, unitMeasureOrderNum FLOAT, unitMeasureTag TEXT";
        db.exec( QString("CREATE TABLE unitMeasureTable (%1)").arg(createUMTableQuery) );
        createPLTableQuery = "id INTEGER PRIMARY KEY, parentId INTEGER, orderNum FLOAT, code TEXT, shortDesc TEXT, longDesc TEXT, unitMeasure INT DEFAULT 0, priceTotal FLOAT DEFAULT 0.0, priceHuman FLOAT DEFAULT 0.0, priceEquipment FLOAT DEFAULT 0.0, priceMaterial FLOAT DEFAULT 0.0, overheads FLOAT DEFAULT 0.13, profits FLOAT DEFAULT 0.10";
        db.exec( QString("CREATE TABLE priceListTable (%1)" ).arg(createPLTableQuery) );
        db.exec( "CREATE INDEX index_id on priceListTable (id)" );
        db.exec( "CREATE INDEX index_parentId on priceListTable (parentId)" );
        db.exec( "CREATE INDEX index_orderNum on priceListTable (orderNum)" );

        unitMeasureModel = new UnitMeasureModel( &db );

        visibleColsList << TableCol( PriceListDBModel::codeCol, "code", "Codice")
                        << TableCol( PriceListDBModel::shortDescCol, "shortDesc", "Denominazione")
                        << TableCol( PriceListDBModel::unitMeasureCol, "unitMeasure", "UdM")
                        << TableCol( PriceListDBModel::priceTotalCol, "priceTotal", "Costo Unitario")
                        << TableCol( PriceListDBModel::priceHumanCol, "priceHuman", "Costo Manodopera")
                        << TableCol( PriceListDBModel::priceHumanCol, "priceEquipment", "Costo Mezzi d'opera")
                        << TableCol( PriceListDBModel::priceHumanCol, "priceMaterial", "Costo Materiali")
                        << TableCol( PriceListDBModel::overheadsCol, "overheads", "Spese Generali")
                        << TableCol( PriceListDBModel::profitsCol, "profits", "Utili");

        colsList << TableCol( PriceListDBModel::nullCol, "", "---")
                 << TableCol( PriceListDBModel::codeCol, "code", "Codice")
                 << TableCol( PriceListDBModel::shortDescCol, "shortDesc", "Denominazione")
                 << TableCol( PriceListDBModel::longDescCol, "longDesc", "Descrizione")
                 << TableCol( PriceListDBModel::unitMeasureCol, "unitMeasure", "UdM")
                 << TableCol( PriceListDBModel::priceTotalCol, "priceTotal", "Costo Unitario")
                 << TableCol( PriceListDBModel::priceHumanCol, "priceHuman", "C.U. Manodopera")
                 << TableCol( PriceListDBModel::priceEquipmentCol, "priceEquipment", "C.U. Mezzi d'opera")
                 << TableCol( PriceListDBModel::priceMaterialCol, "priceMaterial", "C.U. Materiali")
                 << TableCol( PriceListDBModel::overheadsCol, "overheads", "Spese Generali")
                 << TableCol( PriceListDBModel::profitsCol, "profits", "Utili");

        inputColsList << TableCol( PriceListDBModel::nullCol, "", "---")
                      << TableCol( PriceListDBModel::codeCol, "code", "Codice")
                      << TableCol( PriceListDBModel::shortDescCol, "shortDesc", "Denominazione")
                      << TableCol( PriceListDBModel::longDescCol, "longDesc", "Descrizione")
                      << TableCol( PriceListDBModel::unitMeasureCol, "unitMeasure", "UdM")
                      << TableCol( PriceListDBModel::priceTotalCol, "priceTotal", "Costo Unitario")
                      << TableCol( PriceListDBModel::priceHumanCol, "priceHuman", "C.U. Manodopera")
                      << TableCol( PriceListDBModel::perPriceHumanCol, "perPriceHuman", "% Costo Manodopera")
                      << TableCol( PriceListDBModel::priceEquipmentCol, "priceEquipment", "C.U. Mezzi d'opera")
                      << TableCol( PriceListDBModel::perPriceEquipmentCol, "perPriceEquipment", "% Costo Mezzi d'opera")
                      << TableCol( PriceListDBModel::priceMaterialCol, "priceMaterial", "C.U. Materiali")
                      << TableCol( PriceListDBModel::perPriceMaterialCol, "perPriceMaterial", "% Costo Materiali")
                      << TableCol( PriceListDBModel::overheadsCol, "overheads", "Spese Generali")
                      << TableCol( PriceListDBModel::profitsCol, "profits", "Utili");

        QSqlQuery query(db);
        query.exec("INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (0, 1.0, '---')");
        query.exec("INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (1, 2.0, 'cad')");
        query.exec("INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (2, 3.0, 'h')");
        query.exec("INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (3, 4.0, 'kg')");
        query.exec("INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (4, 5.0, 'q')");
        query.exec("INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (5, 6.0, 't')");
        query.exec("INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (6, 7.0, 'm')");
        query.exec("INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (7, 8.0, 'm²')");
        query.exec("INSERT INTO unitMeasureTable (unitMeasureId, unitMeasureOrderNum, unitMeasureTag) VALUES (8, 9.0, 'm³')");

        /* DEBUG */
        /*query.exec("INSERT INTO priceListTable (id, parentId, orderNum, code, shortDesc, longDesc, unitMeasure, priceTotal, priceHuman) VALUES (1, 0, 2.0, '01', 'breve', 'lunga', 0, 1002.1, 741.6)");
        query.exec("INSERT INTO priceListTable (id, parentId, orderNum, code, shortDesc, longDesc, unitMeasure, priceTotal, priceHuman) VALUES (2, 1, 1.0, '01.A01', 'breve', 'lunga', 0, 3.4, 1.7)");
        query.exec("INSERT INTO priceListTable (id, parentId, orderNum, code, shortDesc, longDesc, unitMeasure, priceTotal, priceHuman) VALUES (3, 1, 2.0, '01.A02', 'breve', 'lunga', 0, 5.3, 3.2)");
        query.exec("INSERT INTO priceListTable (id, parentId, orderNum, code, shortDesc, longDesc, unitMeasure, priceTotal, priceHuman) VALUES (4, 3, 1.0, '01.A01.A00', 'breve', 'lunga', 3, 6.7, 4.5)");
        query.exec("INSERT INTO priceListTable (id, parentId, orderNum, code, shortDesc, longDesc, unitMeasure, priceTotal, priceHuman) VALUES (5, 3, 2.0, '01.A01.B00', 'breve', 'lunga', 5, 9.9, 6.5)");
        query.exec("INSERT INTO priceListTable (id, parentId, orderNum, code, shortDesc, longDesc, unitMeasure, priceTotal, priceHuman) VALUES (6, 0, 1.0, '02', 'breve', 'lunga', 0, 10.5, 8.1)");
        query.exec("INSERT INTO priceListTable (id, parentId, orderNum, code, shortDesc, longDesc, unitMeasure, priceTotal, priceHuman) VALUES (7, 6, 1.0, '02.A01', 'breve', 'lunga', 0, 4.7, 3.1)");
        query.exec("INSERT INTO priceListTable (id, parentId, orderNum, code, shortDesc, longDesc, unitMeasure, priceTotal, priceHuman) VALUES (8, 6, 2.0, '02.A02', 'breve', 'lunga', 0, 3.8, 2.1)");
        query.exec("INSERT INTO priceListTable (id, parentId, orderNum, code, shortDesc, longDesc, unitMeasure, priceTotal, priceHuman) VALUES (9, 8, 1.0, '02.A01.A00', 'breve', 'lunga', 7, 2.1, 0.7)");
        query.exec("INSERT INTO priceListTable (id, parentId, orderNum, code, shortDesc, longDesc, unitMeasure, priceTotal, priceHuman) VALUES (10, 8, 2.0, '02.A01.B00', 'breve', 'lunga', 8, 1.4, 0.6)");*/
        /* /DEBUG */
    }
    ~PriceListDBModelPrivate(){
        QString dbName = db.databaseName();
        db.close();
        QSqlDatabase::removeDatabase( dbName );
    }
    int querySize( QSqlQuery * query ){
        if( db.driver()->hasFeature( QSqlDriver::QuerySize )){
            return query->size();
        } else {
            int ret = 0;
            if( query->isSelect() ){
                if( query->isValid() ){
                    ret++;
                }
                while( query->next() ){
                    ret++;
                }
            }
            return ret;
        }
    }
    QString sqlName( PriceListDBModel::PriceColType pct ){
        for( int i=0; i < colsList.size(); ++i){
            if( colsList.at(i).priceColType == pct ){
                return colsList.at(i).sqlName;
            }
        }
        return QString();
    }
    int indexColVisible( PriceListDBModel::PriceColType pct ){
        for( int i=0; i < visibleColsList.size(); ++i){
            if( visibleColsList.at(i).priceColType == pct ){
                return i;
            }
        }
        return -1;
    }

    QLocale * locale;
    QSqlDatabase db;
    QString currentFile;
    QList<TableCol> visibleColsList;
    QList<TableCol> colsList;
    QList<TableCol> inputColsList;
    QList<QString> pendingTransactions;
    UnitMeasureModel *unitMeasureModel;
    QString createUMTableQuery;
    QString createPLTableQuery;
    static int nextId;
};

int PriceListDBModelPrivate::nextId = 1;

PriceListDBModel::PriceListDBModel(QLocale *l, QObject *parent) :
    QAbstractItemModel(parent),
    m_d( new PriceListDBModelPrivate( l ) ) {
}

PriceListDBModel::~PriceListDBModel(){
    delete m_d->unitMeasureModel;
    delete m_d;
}

QModelIndex PriceListDBModel::index(int row, int column, const QModelIndex &parent) const {
    qint32 parentId;
    if (parent.isValid())
        parentId = parent.internalId();
    else
        parentId = 0;

    QString queryStr = QString("SELECT id, orderNum FROM priceListTable WHERE parentId=%1 ORDER BY orderNum").arg( parentId );
    QSqlQuery query = m_d->db.exec( queryStr );
    if (query.seek(row))
        return createIndex(row, column, query.value("id").toInt());
    return QModelIndex();
}

QModelIndex PriceListDBModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    QString queryStr = QString("SELECT parentId FROM priceListTable WHERE id=%1").arg( child.internalId() );
    QSqlQuery query = m_d->db.exec( queryStr );
    if( query.next() ){
        qint32 parent = query.value(0).toInt();
        if (parent == 0)
            return QModelIndex();

        queryStr = QString("SELECT parentId FROM priceListTable WHERE id=%1").arg( parent );
        query = m_d->db.exec( queryStr);
        if( query.next() ){
            qint32 parentOfParent = query.value(0).toInt();

            int row = 0;
            queryStr = QString("SELECT id FROM priceListTable WHERE parentId=%1").arg( parentOfParent );
            query = m_d->db.exec( queryStr );
            for (int i=0; query.next(); i++) {
                if (query.value(0).toInt() == parent) {
                    row = i;
                    break;
                }
            }

            return createIndex(row, 0, parent);
        }
    }
    return QModelIndex();
}

int PriceListDBModel::rowCount(const QModelIndex &parent) const {
    qint32 parentId;
    if (parent.isValid())
        parentId = parent.internalId();
    else
        parentId = 0;

    QString queryStr = QString( "SELECT id FROM priceListTable WHERE parentId=%1").arg(parentId);
    QSqlQuery query( queryStr, m_d->db );
    return m_d->querySize( &query );
}

int PriceListDBModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_d->visibleColsList.size();
}

Qt::ItemFlags PriceListDBModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if( (m_d->visibleColsList.at(index.column()).priceColType == codeCol) ||
            (m_d->visibleColsList.at(index.column()).priceColType == shortDescCol) ||
            (m_d->visibleColsList.at(index.column()).priceColType == longDescCol) ){
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    } else {
        QString childQueryStr = QString("SELECT id, parentId FROM priceListTable WHERE parentId=%1").arg( index.internalId() );
        QSqlQuery childQuery( childQueryStr, m_d->db );
        if(childQuery.next()){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        } else {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable |  Qt::ItemIsEditable;
        }
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PriceListDBModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() )
        return QVariant();

    if( role == Qt::TextAlignmentRole ){
        if( (m_d->visibleColsList.at(index.column()).priceColType == codeCol) ||
                (m_d->visibleColsList.at(index.column()).priceColType == shortDescCol) ||
                (m_d->visibleColsList.at(index.column()).priceColType == longDescCol) ){
            return QVariant(Qt::AlignLeft | Qt::AlignTop);
        } else if( m_d->visibleColsList.at(index.column()).priceColType == unitMeasureCol ) {
            return QVariant(Qt::AlignCenter | Qt::AlignTop);
        } else {
            return QVariant(Qt::AlignRight | Qt::AlignTop);
        }
    }

    if( role == Qt::DisplayRole || role == Qt::EditRole ){
        QString childQueryStr = QString("SELECT id, parentId FROM priceListTable WHERE parentId=%1").arg( index.internalId() );
        QSqlQuery childQuery( childQueryStr, m_d->db );
        if(childQuery.next()){
            if( (m_d->visibleColsList.at(index.column()).priceColType == codeCol) ||
                    (m_d->visibleColsList.at(index.column()).priceColType == shortDescCol) ||
                    (m_d->visibleColsList.at(index.column()).priceColType == longDescCol) ){
                QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg(
                            m_d->visibleColsList.at(index.column()).sqlName,
                            QString::number(index.internalId()) );
                QSqlQuery query( queryStr, m_d->db );
                if( query.next() ){
                    return QVariant( query.value(0).toString() );
                }
            }
        } else {
            QString queryStr;
            if( m_d->visibleColsList.at(index.column()).priceColType == unitMeasureCol ){
                queryStr = QString("SELECT unitMeasureTag, unitMeasureId, id, unitMeasure FROM priceListTable JOIN unitMeasureTable ON unitMeasureId=unitMeasure WHERE id=%1").arg( index.internalId() );
            } else {
                queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg(
                            m_d->visibleColsList.at(index.column()).sqlName,
                            QString::number(index.internalId()) );
            }
            QSqlQuery query( queryStr, m_d->db );
            if( query.next() ){
                QString ret;
                if( (m_d->visibleColsList.at(index.column()).priceColType == priceTotalCol) ||
                        (m_d->visibleColsList.at(index.column()).priceColType == priceHumanCol) ||
                        (m_d->visibleColsList.at(index.column()).priceColType == priceEquipmentCol) ||
                        (m_d->visibleColsList.at(index.column()).priceColType == priceMaterialCol) ){
                    double v = query.value(0).toDouble();
                    ret = m_d->locale->toString( v, 'f', 2);
                } else {
                    ret = query.value(0).toString();
                }
                return QVariant(ret);
            }
        }
    }

    return QVariant();
}

bool PriceListDBModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() )
        return false;

    if( role == Qt::EditRole ){
        QString queryStr;
        if( (m_d->visibleColsList.at(index.column()).priceColType == codeCol) ||
                (m_d->visibleColsList.at(index.column()).priceColType == shortDescCol) ){
            queryStr = QString("UPDATE priceListTable SET %1='%2' WHERE id=%3").arg(
                        m_d->visibleColsList.at(index.column()).sqlName,
                        value.toString(),
                        QString::number(index.internalId()) );
        } else {
            queryStr = QString("UPDATE priceListTable SET %1=%2 WHERE id=%3").arg(
                        m_d->visibleColsList.at(index.column()).sqlName,
                        value.toString(),
                        QString::number(index.internalId()) );
        }
        execTransaction( queryStr );
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

QVariant PriceListDBModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole ||
            section <0 || section > m_d->visibleColsList.size())
        return QVariant();

    return QVariant( m_d->visibleColsList.at(section).userName);
}

bool PriceListDBModel::insertRows(int row, int count, const QModelIndex &parent) {
    if( count < 1 || row < 0 ){
        return false;
    }

    int parentId = 0;
    if( parent.isValid() ){
        parentId = parent.internalId();
    }
    QString queryStr = QString("SELECT %1, orderNum FROM priceListTable WHERE parentId=%2 ORDER BY orderNum").arg( "orderNum", QString::number( parent.internalId()) );
    QSqlQuery query( queryStr, m_d->db );

    int counter = 0;
    double before = 0.0, after = 1.0;
    while( query.next() && counter <= row ){
        if( row == counter ){
            after = query.value("orderNum").toDouble();
        }
        if( row == (counter+1) ){
            before = query.value("orderNum").toDouble();
        }
        counter++;
    }
    if( before != after ){
        beginInsertRows( parent, row, row+count-1);
        for( int i=0; i < count; ++i){
            if( after < before ){
                after = before + 2.0;
            }
            double ord = (before + after) * 0.50;
            QString queryStr = QString("INSERT INTO priceListTable (id, parentId, orderNum, code, shortDesc, longDesc, unitMeasure, priceTotal, priceHuman, priceEquipment, priceMaterial) VALUES (%1, %2, %3, '', '', '', 0, 0.0, 0.0, 0.0, 0.0)").arg(
                        QString::number(nextId()), QString::number( parentId ), QString::number(ord) );
            execTransaction( queryStr );
            before = ord;
        }
        endInsertRows();
        return true;
    }
    return false;
}

bool PriceListDBModel::removeRows(int row, int count, const QModelIndex &parent) {
    if( count < 1 ){
        return false;
    }
    int parentId = 0;
    if( parent.isValid() ){
        parentId = parent.internalId();
    }
    QString queryStr = QString("SELECT id, orderNum FROM priceListTable WHERE parentId=%1 ORDER BY orderNum" ).arg( parentId  );
    QSqlQuery query( queryStr, m_d->db );

    int i = 0;
    while( query.next() && i < row ){
        ++i;
    }
    if( query.isValid() ){
        QList<int> idToBeDeleted;
        idToBeDeleted << query.value( 0 ).toInt();
        i = 1;
        while( query.next() && i < count ){
            idToBeDeleted << query.value( 0 ).toInt();
            ++i;
        }
        for( int i = 0; i < idToBeDeleted.size(); ++i ){
            deleteChildren( createIndex(row+i, 0, idToBeDeleted.at(i)) );
        }
        beginRemoveRows( parent, row, row+count-1);
        for( int i = 0; i < idToBeDeleted.size(); ++i ){
            queryStr = QString("DELETE FROM priceListTable WHERE id=%1").arg(  QString::number( idToBeDeleted.at(i)) );
            execTransaction( queryStr );
        }
        endRemoveRows();
        return true;
    }

    return false;
}

QList< QPair<QString, int> > PriceListDBModel::unitMeasureList() {
    QSqlQuery query( "SELECT unitMeasureId, unitMeasureOrderNum, unitMeasureTag FROM unitMeasureTable ORDER BY unitMeasureOrderNum", m_d->db );
    QList<QPair<QString, int> > ret;
    while( query.next() ){
        ret << qMakePair( query.value( "unitMeasureTag").toString(), query.value( "unitMeasureId").toInt() );
    }
    return ret;
}

UnitMeasureModel *PriceListDBModel::unitMeasureModel() {
    return m_d->unitMeasureModel;
}

void PriceListDBModel::clear() {
    beginResetModel();
    QString queryStr( "DELETE FROM priceListTable" );
    m_d->db.exec( queryStr );
    endResetModel();
    m_d->pendingTransactions.clear();

    m_d->unitMeasureModel->clear();

    if( !(m_d->currentFile.isEmpty()) ){
        QString queryStr = QString("DETACH DATABASE fileDB");
        m_d->db.exec( queryStr );
        m_d->currentFile.clear();
    }
}

QString PriceListDBModel::code(const QModelIndex &index) {
    // non e' necessario ordinare
    QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "code", QString::number( index.internalId() ));
    QSqlQuery query( queryStr, m_d->db );
    if( query.next() ){
        QString v = query.value(0).toString();
        v.replace("'", "'");;
        return v;
    }
    return QString();
}

QString PriceListDBModel::shortDescription(const QModelIndex &index) {
    // non e' necessario ordinare
    QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "shortDesc", QString::number( index.internalId() ));
    QSqlQuery query( queryStr, m_d->db );
    if( query.next() ){
        QString v = query.value(0).toString();
        v.replace("'", "'");;
        return v;
    }
    return QString();
}

QString PriceListDBModel::longDescription(const QModelIndex &index) {
    // non e' necessario ordinare
    QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "longDesc", QString::number( index.internalId() ));
    QSqlQuery query( queryStr, m_d->db );
    if( query.next() ){
        QString v = query.value(0).toString();
        v.replace("'", "'");;
        return v;
    }
    return QString();
}

int PriceListDBModel::unitMeasure(const QModelIndex &index) {
    // non e' necessario ordinare
    QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "unitMeasure", QString::number( index.internalId() ));
    QSqlQuery query( queryStr, m_d->db );
    if( query.next() ){
        return query.value(0).toInt();
    }
    return 0;
}

double PriceListDBModel::priceTotal(const QModelIndex &index) {
    // non e' necessario ordinare
    QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "priceTotal", QString::number( index.internalId() ));
    QSqlQuery query( queryStr, m_d->db );
    if( query.next() ){
        return query.value(0).toDouble();
    }
    return 0.0;
}

double PriceListDBModel::priceHuman(const QModelIndex &index) {
    // non e' necessario ordinare
    QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "priceHuman", QString::number( index.internalId() ));
    QSqlQuery query( queryStr, m_d->db );
    if( query.next() ){
        return query.value(0).toDouble();
    }
    return 0.0;
}

double PriceListDBModel::priceEquipment(const QModelIndex &index) {
    // non e' necessario ordinare
    QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "priceEquipment", QString::number( index.internalId() ));
    QSqlQuery query( queryStr, m_d->db );
    if( query.next() ){
        return query.value(0).toDouble();
    }
    return 0.0;
}

double PriceListDBModel::priceMaterial(const QModelIndex &index) {
    // non e' necessario ordinare
    QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "priceMaterial", QString::number( index.internalId() ));
    QSqlQuery query( queryStr, m_d->db );
    if( query.next() ){
        return query.value(0).toDouble();
    }
    return 0.0;
}

double PriceListDBModel::overheads(const QModelIndex &index) {
    // non e' necessario ordinare
    QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "overheads", QString::number( index.internalId() ));
    QSqlQuery query( queryStr, m_d->db );
    if( query.next() ){
        return query.value(0).toDouble();
    }
    return 0.0;
}

double PriceListDBModel::profits(const QModelIndex &index) {
    // non e' necessario ordinare
    QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "profits", QString::number( index.internalId() ));
    QSqlQuery query( queryStr, m_d->db );
    if( query.next() ){
        return query.value(0).toDouble();
    }
    return 0.0;
}

void PriceListDBModel::setCode(const QString &v, const QPersistentModelIndex &index) {
    if( index.isValid() ){
        QString mv = v;
        mv.replace("'", "''");
        QString queryStr = QString("UPDATE priceListTable SET %1='%2' WHERE id=%3").arg( "code", v, QString::number( index.internalId()));
        execTransaction( queryStr );
        QModelIndex i = createIndex( index.row(), m_d->indexColVisible(PriceListDBModel::codeCol ), index.internalId() );
        emit dataChanged( i, i);
    }
}

void PriceListDBModel::setShortDescription(const QString &v, const QPersistentModelIndex &index) {
    if( index.isValid() ){
        QString mv = v;
        mv.replace("'", "''");
        QString queryStr = QString("UPDATE priceListTable SET %1='%2' WHERE id=%3").arg( "shortDesc", mv, QString::number(index.internalId()) );
        execTransaction( queryStr );
        QModelIndex i = createIndex( index.row(), m_d->indexColVisible(PriceListDBModel::shortDescCol), index.internalId() );
        emit dataChanged( i, i);
    }
}

void PriceListDBModel::setLongDescription(const QString &v, const QPersistentModelIndex &index) {
    if( index.isValid() ){
        QString mv = v;
        mv.replace("'", "''");
        QString queryStr = QString("UPDATE priceListTable SET %1='%2' WHERE id=%3").arg( "longDesc", mv, QString::number(index.internalId()) );
        execTransaction( queryStr );
    }
}

void PriceListDBModel::setUnitMeasure(int v, const QPersistentModelIndex &index) {
    if( index.isValid() ){
        QString queryStr = QString("SELECT unitMeasureTag FROM unitMeasureTable WHERE unitMeasureId=%1").arg( v );
        QSqlQuery queryUM( queryStr, m_d->db );
        queryUM.next();
        if( queryUM.isValid() ){
            queryStr = QString("UPDATE priceListTable SET %1='%2' WHERE id=%3").arg( "unitMeasure", QString::number(v), QString::number(index.internalId()) );
            execTransaction( queryStr );
            QModelIndex i = createIndex( index.row(), m_d->indexColVisible(PriceListDBModel::unitMeasureCol), index.internalId() );
            emit dataChanged( i, i);
        }
    }
}

void PriceListDBModel::setPriceTotal(double v, const QPersistentModelIndex &index) {
    if( index.isValid() ){
        QString queryStr = QString("UPDATE priceListTable SET %1='%2' WHERE id=%3").arg( "priceTotal", QString::number(v), QString::number(index.internalId()) );
        execTransaction( queryStr );
        QModelIndex i = createIndex( index.row(), m_d->indexColVisible(PriceListDBModel::priceTotalCol), index.internalId() );
        emit dataChanged( i, i);
    }
}

void PriceListDBModel::setPriceHuman(double v, const QPersistentModelIndex &index) {
    if( index.isValid() ){
        QString queryStr = QString("UPDATE priceListTable SET %1='%2' WHERE id=%3").arg( "priceHuman", QString::number(v), QString::number(index.internalId()) );
        execTransaction( queryStr );
        QModelIndex i = createIndex( index.row(), m_d->indexColVisible(PriceListDBModel::priceHumanCol), index.internalId() );
        emit dataChanged( i, i);
    }
}

void PriceListDBModel::setPriceEquipment(double v, const QPersistentModelIndex &index) {
    if( index.isValid() ){
        QString queryStr = QString("UPDATE priceListTable SET %1='%2' WHERE id=%3").arg( "priceEquipment", QString::number(v), QString::number(index.internalId()) );
        execTransaction( queryStr );
        QModelIndex i = createIndex( index.row(), m_d->indexColVisible(PriceListDBModel::priceEquipmentCol), index.internalId() );
        emit dataChanged( i, i);
    }
}

void PriceListDBModel::setPriceMaterial(double v, const QPersistentModelIndex &index) {
    if( index.isValid() ){
        QString queryStr = QString("UPDATE priceListTable SET %1='%2' WHERE id=%3").arg( "priceMaterial", QString::number(v), QString::number(index.internalId()) );
        execTransaction( queryStr );
        QModelIndex i = createIndex( index.row(), m_d->indexColVisible(PriceListDBModel::priceMaterialCol), index.internalId() );
        emit dataChanged( i, i);
    }
}

void PriceListDBModel::setOverheads(double v, const QPersistentModelIndex &index) {
    if( index.isValid() ){
        QString queryStr = QString("UPDATE priceListTable SET %1='%2' WHERE id=%3").arg( "overheads", QString::number(v), QString::number(index.internalId()) );
        execTransaction( queryStr );
        QModelIndex i = createIndex( index.row(), m_d->indexColVisible(PriceListDBModel::overheadsCol), index.internalId() );
        emit dataChanged( i, i);
    }
}

void PriceListDBModel::setProfits(double v, const QPersistentModelIndex &index) {
    if( index.isValid() ){
        QString queryStr = QString("UPDATE priceListTable SET %1='%2' WHERE id=%3").arg( "profits", QString::number(v), QString::number(index.internalId()) );
        execTransaction( queryStr );
        QModelIndex i = createIndex( index.row(), m_d->indexColVisible(PriceListDBModel::profitsCol), index.internalId() );
        emit dataChanged( i, i);
    }
}

bool PriceListDBModel::setCurrentFile(const QString & newFile, bool openExisting) {
    if( !(m_d->currentFile.isEmpty()) ){
        QString queryStr = QString("DETACH DATABASE fileDB");
        m_d->db.exec( queryStr );
    }

    m_d->currentFile = newFile;

    if( !(m_d->currentFile.isEmpty()) ){
        QString queryStr = QString("ATTACH DATABASE '%1' AS fileDB").arg(m_d->currentFile);
        m_d->db.exec( queryStr );

        queryStr = QString("CREATE TABLE IF NOT EXISTS fileDB.unitMeasureTable (%1)").arg( m_d->createUMTableQuery );
        m_d->db.exec( queryStr );

        queryStr = QString("CREATE TABLE IF NOT EXISTS fileDB.priceListTable (%1)").arg( m_d->createPLTableQuery );
        m_d->db.exec( queryStr );
        m_d->db.exec( "CREATE INDEX IF NOT EXISTS fileDB.index_id on priceListTable (id)" );
        m_d->db.exec( "CREATE INDEX IF NOT EXISTS fileDB.index_parentId on priceListTable (parentId)" );

        if( openExisting ){
            beginResetModel();
            m_d->db.exec( "DELETE FROM unitMeasureTable" );
            m_d->db.exec( "INSERT INTO unitMeasureTable SELECT * FROM fileDB.unitMeasureTable" );
            m_d->db.exec( "DELETE FROM priceListTable" );
            m_d->db.exec( "INSERT INTO priceListTable SELECT * FROM fileDB.priceListTable" );
            endResetModel();
        } else {
            m_d->db.exec( "DELETE FROM fileDB.unitMeasureTable" );
            m_d->db.exec( "INSERT INTO fileDB.unitMeasureTable SELECT * FROM unitMeasureTable" );
            m_d->db.exec( "DELETE FROM fileDB.priceListTable" );
            m_d->db.exec( "INSERT INTO fileDB.priceListTable SELECT * FROM priceListTable" );
        }
        m_d->pendingTransactions.clear();
        return true;
    }
    return false;
}

bool PriceListDBModel::saveCurrentFile() {
    if( !(m_d->currentFile.isEmpty()) ){
        QString queryStr;
        for( int i=0; i < m_d->pendingTransactions.size(); ++i ){
            queryStr = m_d->pendingTransactions.at(i);
            queryStr.replace( "unitMeasureTable", "fileDB.unitMeasureTable");
            queryStr.replace( "priceListTable", "fileDB.priceListTable");
            m_d->db.exec( queryStr );
        }
        m_d->pendingTransactions.clear();

        return true;
    }
    return false;
}

int PriceListDBModel::unitMeasureColumn() {
    return m_d->indexColVisible( unitMeasureCol );
}

QLocale *PriceListDBModel::locale() {
    return m_d->locale;
}

void PriceListDBModel::importFromDB(const QString & fileName ){
    beginResetModel();

    QString importFileDBName("importFileDB");
    QString queryStr = QString("ATTACH DATABASE '%1' AS %2").arg(fileName, importFileDBName);
    m_d->db.exec( queryStr );

    bool changed = loadFromDBChildren( 0, 0, importFileDBName );

    m_d->db.exec( QString("DETACH DATABASE %1").arg(importFileDBName));

    endResetModel();

    emit modelChanged( changed );
}

bool PriceListDBModel::loadFromDBChildren(int fileParentId,
                                          int parentId,
                                          const QString & importFileDBName ){
    QString queryStr = QString("SELECT * FROM %1.priceListTable WHERE parentId=%2 ORDER BY orderNum").arg(importFileDBName, QString::number(fileParentId));
    QSqlQuery filePLQuery = m_d->db.exec( queryStr );
    bool ret = false;

    while( filePLQuery.next() ){
        if( !ret ){
            ret = true;
        }
        int id = nextId();
        int fileId = filePLQuery.value("id").toInt();

        int UMId = 0;
        QString fileUMQueryStr = QString("SELECT unitMeasureId, unitMeasureTag FROM %1.unitMeasureTable WHERE unitMeasureId=%2").arg( importFileDBName, filePLQuery.value("unitMeasure").toString());
        QSqlQuery fileUMQuery = m_d->db.exec( fileUMQueryStr );
        if( fileUMQuery.next() ){
            QString fileUMTag = fileUMQuery.value("unitMeasureTag").toString();
            QString UMQueryStr = QString("SELECT unitMeasureId, unitMeasureTag FROM unitMeasureTable WHERE unitMeasureTag='%1'").arg(fileUMTag);
            QSqlQuery UMQuery = m_d->db.exec( UMQueryStr );
            if( UMQuery.next() ){
                UMId = UMQuery.value("unitMeasureId").toInt();
            } else {
                m_d->unitMeasureModel->appendRow( &UMId, "fileUMTag");
            }
        }

        double ord = 1.0;
        queryStr = QString("SELECT orderNum, parentId FROM priceListTable WHERE parentID=%1 ORDER BY orderNum").arg( parentId );
        QSqlQuery queryOrderNum = m_d->db.exec( queryStr );
        if( queryOrderNum.last() ){
            ord = queryOrderNum.value( "orderNum" ).toDouble() + 1.0;
        }

        QString fileCode = filePLQuery.value("code").toString();
        fileCode.replace("'", "''");
        QString fileShortDesc = filePLQuery.value("shortDesc").toString();
        fileShortDesc.replace("'", "''");
        QString fileLongDesc = filePLQuery.value("longDesc").toString();
        fileLongDesc.replace("'", "''");
        queryStr = QString("INSERT INTO priceListTable (id, parentId, orderNum, code, shortDesc, longDesc, unitMeasure, priceTotal, priceHuman, priceEquipment, priceMaterial) VALUES (");
        queryStr.append( QString::number(id) + ", ");
        queryStr.append( QString::number( parentId ) + ", ");
        queryStr.append( QString::number(ord) + ", ");
        queryStr.append( "\"" + fileCode + "\", ");
        queryStr.append( "\"" + fileShortDesc + "\", ");
        queryStr.append( "\"" + fileLongDesc + "\", ");
        queryStr.append( QString::number(UMId) + ", ");
        queryStr.append( filePLQuery.value("priceTotal").toString() + ", ");
        queryStr.append( filePLQuery.value("priceHuman").toString() + ", ");
        queryStr.append( filePLQuery.value("priceEquipment").toString() + ", ");
        queryStr.append( filePLQuery.value("priceMaterial").toString()  + ")");
        m_d->db.exec( queryStr );

        loadFromDBChildren( fileId, id, importFileDBName );
    }
    return ret;
}

void PriceListDBModel::importFromTXT(const QString & decimalSeparator,
                                     const QString & thousandSeparator,
                                     double ovh, double prf,
                                     bool setShortDescFromLong,
                                     QList<PriceColType> * pCols,
                                     QTextStream * input) {
    beginResetModel();

    input->seek(0);
    int p=1;
    while( !input->atEnd() ){
        QStringList line = input->readLine().split("\t");
        loadFromTXTChildren( &p, line, 0, "", decimalSeparator, thousandSeparator, ovh, prf, setShortDescFromLong, pCols, input );
    }

    endResetModel();
}

QStringList PriceListDBModel::loadFromTXTChildren(int * progNumber,
                                                  QStringList line,
                                                  int parentId,
                                                  QString parentCode,
                                                  const QString & decimalSeparator,
                                                  const QString & thousandSeparator,
                                                  double ovh, double prf,
                                                  bool setShortDescFromLong,
                                                  QList<PriceColType> * pCols,
                                                  QTextStream * input){
    if( line.isEmpty() ){
        return line;
    }

    bool lineHasData = false;
    if( line.size() > 1 ){
        lineHasData = true;
    } else if( line.size() == 1 ){
        if( !(line.first().isEmpty()) ){
            lineHasData = true;
        }
    }

    while( !(input->atEnd()) || lineHasData ){
        int i=0;

        QString queryStr = QString("SELECT id FROM priceListTable WHERE id=%1").arg( *progNumber ) ;
        while( m_d->db.exec( queryStr ).next() ){
            (*progNumber)++;
            queryStr = QString("SELECT id FROM priceListTable WHERE id=%1").arg( *progNumber ) ;
        }

        QString before("id, parentId, orderNum");
        QString after( QString("%1, %2, %1").arg( QString::number(*progNumber), QString::number(parentId)));
        QString currentCode;

        double _priceTotal = 0.0;

        bool priceHumanWasSet = false;
        double _perPriceHuman = 0.0;
        bool perPriceHumanWasSet = false;

        bool priceEquipmentWasSet = false;
        double _perPriceEquipment = 0.0;
        bool perPriceEquipmentWasSet = false;

        bool priceMaterialWasSet = false;
        double _perPriceMaterial = 0.0;
        bool perPriceMaterialWasSet = false;

        QString longDesc;
        bool shortDescSet = false;

        while( (i<line.size()) && (i<pCols->size())){
            if( (pCols->at(i) != nullCol) && (pCols->at(i) != perPriceHumanCol)){
                QString beforeApp = m_d->sqlName( pCols->at(i) );
                QString afterApp;
                if( pCols->at(i) == unitMeasureCol ){
                    QString umTag = line.at(i);
                    while( umTag.startsWith(" ")){
                        umTag.remove(0, 0);
                    }
                    while( umTag.endsWith(" ")){
                        umTag.chop( 1 );
                    }
                    if( !umTag.isEmpty() ){
                        QString queryStr = QString("SELECT unitMeasureId, unitMeasureTag FROM unitMeasureTable WHERE unitMeasureTag='%1'").arg( umTag ) ;
                        QSqlQuery query = m_d->db.exec( queryStr );
                        if( query.next() ){
                            afterApp = query.value( "unitMeasureId" ).toString();
                        } else {
                            int newId;
                            if( m_d->unitMeasureModel->appendRow( &newId, umTag ) ){
                                afterApp = QString::number(newId);
                            } else {
                                afterApp = "0";
                            }
                        }
                    } else {
                        afterApp = "0";
                    }
                } else if( (pCols->at(i) == codeCol) ||
                           (pCols->at(i) == shortDescCol) ||
                           (pCols->at(i) == longDescCol)){
                    afterApp = line.at(i);
                    if( pCols->at(i) == longDescCol ){
                        longDesc = line.at(i);
                    }
                    if( pCols->at(i) == shortDescCol ){
                        shortDescSet = true;
                    }
                    afterApp.replace("'", "''");
                    afterApp.prepend("'");
                    afterApp.append("'");
                } else if( (pCols->at(i) == priceTotalCol) ||
                           (pCols->at(i) == priceHumanCol)){
                    QString l = line.at(i);
                    l.remove(thousandSeparator);
                    l.replace( decimalSeparator, ".");
                    if(pCols->at(i) == priceTotalCol){
                        _priceTotal = l.toDouble();
                    }
                    afterApp = l;
                }

                if(pCols->at(i) == codeCol){
                    currentCode = line.at(i);
                } else if(pCols->at(i) == priceHumanCol){
                    priceHumanWasSet = true;
                } else if(pCols->at(i) == priceEquipmentCol){
                    priceEquipmentWasSet = true;
                } else if(pCols->at(i) == priceMaterialCol){
                    priceMaterialWasSet = true;
                }

                if( !beforeApp.isEmpty() && !afterApp.isEmpty() ){
                    before.append(QString(", %1").arg(beforeApp));
                    after.append(QString(", %1").arg(afterApp));
                }
            } else if( pCols->at(i) == perPriceHumanCol ){
                QString l = line.at(i);
                l.remove("%");
                l.remove(thousandSeparator);
                l.replace( decimalSeparator, ".");
                _perPriceHuman = l.toDouble() / 100.0;
                perPriceHumanWasSet = true;
            } else if( pCols->at(i) == perPriceEquipmentCol ){
                QString l = line.at(i);
                l.remove("%");
                l.remove(thousandSeparator);
                l.replace( decimalSeparator, ".");
                _perPriceEquipment = l.toDouble() / 100.0;
                perPriceEquipmentWasSet = true;
            } else if( pCols->at(i) == perPriceMaterialCol ){
                QString l = line.at(i);
                l.remove("%");
                l.remove(thousandSeparator);
                l.replace( decimalSeparator, ".");
                _perPriceMaterial = l.toDouble() / 100.0;
                perPriceMaterialWasSet = true;
            }

            ++i;
        }

        if( !shortDescSet && setShortDescFromLong && !longDesc.isEmpty()){
            before.append(QString(", %1").arg(m_d->sqlName( shortDescCol )));
            QString shortDesc = longDesc;
            int maxSize = 70;
            if( shortDesc.size() > maxSize ){
                QString app("...");
                shortDesc = shortDesc.left(maxSize-app.size() ) + app;
            }
            shortDesc.replace("'", "''");
            after.append(QString(", '%1'").arg( shortDesc ));
        }

        if( !priceHumanWasSet && perPriceHumanWasSet ){
            double _priceHuman = _priceTotal * _perPriceHuman;
            if( before.isEmpty() ){
                before = QString( "%1").arg( "priceHuman");
            } else {
                before.append( QString( ", %1").arg( "priceHuman") );
            }
            if( after.isEmpty() ){
                before = QString( "%1").arg( _priceHuman );
            } else {
                after.append( QString( ", %1").arg( _priceHuman ) );
            }
        }
        if( !priceEquipmentWasSet && perPriceEquipmentWasSet ){
            double _priceEquipment = _priceTotal * _perPriceEquipment;
            if( before.isEmpty() ){
                before = QString( "%1").arg( "priceEquipment");
            } else {
                before.append( QString( ", %1").arg( "priceEquipment") );
            }
            if( after.isEmpty() ){
                before = QString( "%1").arg( _priceEquipment );
            } else {
                after.append( QString( ", %1").arg( _priceEquipment ) );
            }
        }
        if( !priceMaterialWasSet && perPriceMaterialWasSet ){
            double _priceMaterial = _priceTotal * _perPriceMaterial;
            if( before.isEmpty() ){
                before = QString( "%1").arg( "priceMaterial");
            } else {
                before.append( QString( ", %1").arg( "priceMaterial") );
            }
            if( after.isEmpty() ){
                before = QString( "%1").arg( _priceMaterial );
            } else {
                after.append( QString( ", %1").arg( _priceMaterial ) );
            }
        }

        if( currentCode.isEmpty() ){
            // se non c'e' il codice ignoriamo la linea
            line = input->readLine().split("\t");
        } else if( (currentCode.startsWith( parentCode )) && (currentCode.size() > parentCode.size()) ){
            // spese generali
            if( before.isEmpty() ){
                before = QString( "%1").arg("overheads");
            } else {
                before.append( QString( ", %1").arg("overheads") );
            }
            if( after.isEmpty() ){
                before = QString( "%1").arg( ovh );
            } else {
                after.append( QString( ", %1").arg( ovh ) );
            }

            // utili
            if( before.isEmpty() ){
                before = QString( "%1").arg("profits");
            } else {
                before.append( QString( ", %1").arg("profits") );
            }
            if( after.isEmpty() ){
                before = QString( "%1").arg( prf );
            } else {
                after.append( QString( ", %1").arg( prf ) );
            }

            queryStr = QString("INSERT INTO priceListTable (%1) VALUES (%2)").arg( before, after );
            execTransaction( queryStr );

            int currentId = (*progNumber)++;

            line = input->readLine().split("\t");
            line = loadFromTXTChildren( progNumber, line, currentId, currentCode, decimalSeparator, thousandSeparator, ovh, prf, setShortDescFromLong, pCols, input );
        } else {
            return line;
        }

        // controlla se ci sono dati in line
        lineHasData = false;
        if( line.size() > 1 ){
            lineHasData = true;
        } else if( line.size() == 1 ){
            if( !(line.first().isEmpty()) ){
                lineHasData = true;
            }
        }
    }
    return QStringList();
}

QList< QPair<PriceListDBModel::PriceColType, QString> > PriceListDBModel::inputColsUserName() {
    QList<QPair<PriceListDBModel::PriceColType, QString> > ret;
    for( int i=0; i < m_d->inputColsList.size(); ++i ){
        ret << qMakePair( m_d->inputColsList.at(i).priceColType, m_d->inputColsList.at(i).userName );
    }
    return ret;
}

QString PriceListDBModel::currentFile() {
    return m_d->currentFile;
}

int PriceListDBModel::nextId() {
    int ret = m_d->nextId++;
    QString queryStr = QString("SELECT id FROM priceListTable WHERE id=%1").arg( ret );
    QSqlQuery query( queryStr, m_d->db );
    while( query.next() ){
        ret = m_d->nextId++;
        queryStr = QString("SELECT id FROM priceListTable WHERE id=%1").arg( ret );
        query.exec( queryStr );
    }
    return ret;
}

void PriceListDBModel::deleteChildren(const QModelIndex &parent) {
    QString queryStr = QString("SELECT id FROM priceListTable WHERE parentId=%1").arg( parent.internalId() );
    QSqlQuery query( queryStr );
    removeRows( 0, m_d->querySize(&query), parent );
}

QSqlQuery PriceListDBModel::execTransaction(const QString &queryStr) {
    QSqlQuery query = m_d->db.exec( queryStr );

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
