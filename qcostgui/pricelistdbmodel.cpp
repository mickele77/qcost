#include "pricelistdbmodel.h"

#include "mathparser.h"
#include "priceitem.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QTextStream>
#include <QFileInfo>

#include <QSqlError>

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
    PriceListDBModelPrivate( MathParser * p, const QString & connectionName ):
        parser(p){
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName );
        db.setDatabaseName(":memory:");
        db.open();

        QString createUMTableQuery = "unitMeasureId INTEGER PRIMARY KEY, unitMeasureOrderNum FLOAT, unitMeasureTag TEXT";
        db.exec( QString("CREATE TABLE unitMeasureTable (%1)").arg(createUMTableQuery) );

        QString createPLTableQuery = "id INTEGER PRIMARY KEY, parentId INTEGER, orderNum FLOAT, code TEXT, shortDesc TEXT, longDesc TEXT, unitMeasure INT DEFAULT 0, priceTotal FLOAT DEFAULT 0.0, priceSafety FLOAT DEFAULT 0.0, priceHuman FLOAT DEFAULT 0.0, priceEquipment FLOAT DEFAULT 0.0, priceMaterial FLOAT DEFAULT 0.0, overheads FLOAT DEFAULT 0.15, profits FLOAT DEFAULT 0.10";
        db.exec( QString("CREATE TABLE priceListTable (%1)" ).arg(createPLTableQuery) );
        db.exec( "CREATE INDEX index_id on priceListTable (id)" );
        db.exec( "CREATE INDEX index_parentId on priceListTable (parentId)" );
        db.exec( "CREATE INDEX index_orderNum on priceListTable (orderNum)" );

        visibleColsList << TableCol( PriceListDBModel::codeCol, "code", "Codice")
                        << TableCol( PriceListDBModel::shortDescCol, "shortDesc", "Denominazione")
                        << TableCol( PriceListDBModel::unitMeasureCol, "unitMeasure", "UdM")
                        << TableCol( PriceListDBModel::priceTotalCol, "priceTotal", "Costo Unitario")
                        << TableCol( PriceListDBModel::priceHumanCol, "priceHuman", "C.U. Manodopera")
                        << TableCol( PriceListDBModel::priceEquipmentCol, "priceEquipment", "C.U. Mezzi d'opera")
                        << TableCol( PriceListDBModel::priceMaterialCol, "priceMaterial", "C.U. Materiali");
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

    MathParser * parser;
    QSqlDatabase db;
    QString currentFile;
    QList<TableCol> visibleColsList;
    QList<TableCol> colsList;
    QString filter;
};

PriceListDBModel::PriceListDBModel(MathParser *p, const QString & connectionName, QObject *parent) :
    QAbstractItemModel(parent),
    m_d( new PriceListDBModelPrivate( p, connectionName ) ) {
}

PriceListDBModel::~PriceListDBModel(){
    delete m_d;
}

QModelIndex PriceListDBModel::index(int row, int column, const QModelIndex &parent) const {
    if( m_d->db.isOpen() ){
        quintptr parentId;
        if (parent.isValid())
            parentId = parent.internalId();
        else
            parentId = 0;

        QString queryStr = QString("SELECT id, orderNum FROM priceListTable WHERE parentId=%1 ORDER BY orderNum").arg( parentId );
        QSqlQuery query = m_d->db.exec( queryStr );
        if (query.seek(row))
            return createIndex(row, column, query.value("id").toUInt());
    }
    return QModelIndex();
}

QModelIndex PriceListDBModel::parent(const QModelIndex &child) const {
    if( m_d->db.isOpen() ){
        if (!child.isValid())
            return QModelIndex();

        QString queryStr = QString("SELECT parentId FROM priceListTable WHERE id=%1").arg( child.internalId() );
        QSqlQuery query = m_d->db.exec( queryStr );
        if( query.next() ){
            quintptr parent = query.value(0).toUInt();
            if (parent == 0)
                return QModelIndex();

            queryStr = QString("SELECT parentId FROM priceListTable WHERE id=%1").arg( parent );
            query = m_d->db.exec( queryStr);
            if( query.next() ){
                quintptr parentOfParent = query.value(0).toUInt();

                int row = 0;
                queryStr = QString("SELECT id FROM priceListTable WHERE parentId=%1").arg( parentOfParent );
                query = m_d->db.exec( queryStr );
                for (int i=0; query.next(); i++) {
                    if (query.value(0).toUInt() == parent) {
                        row = i;
                        break;
                    }
                }

                return createIndex(row, 0, parent);
            }
        }
    }
    return QModelIndex();
}

int PriceListDBModel::rowCount(const QModelIndex &parent) const {
    if( m_d->db.isOpen() ){
        quintptr parentId;
        if (parent.isValid())
            parentId = parent.internalId();
        else
            parentId = 0;

        QString queryStr = QString( "SELECT id FROM priceListTable WHERE parentId=%1").arg(parentId);
        QSqlQuery query( queryStr, m_d->db );
        return m_d->querySize( &query );
    }
    return 0;
}

int PriceListDBModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_d->visibleColsList.size();
}

Qt::ItemFlags PriceListDBModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PriceListDBModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() )
        return QVariant();

    if( role == Qt::TextAlignmentRole ){
        if( (m_d->visibleColsList.at(index.column()).priceColType == priceTotalCol) ||
                (m_d->visibleColsList.at(index.column()).priceColType == priceHumanCol) ||
                (m_d->visibleColsList.at(index.column()).priceColType == priceEquipmentCol) ||
                (m_d->visibleColsList.at(index.column()).priceColType == priceMaterialCol) ){
            return QVariant(Qt::AlignRight | Qt::AlignTop);
        } else if(m_d->visibleColsList.at(index.column()).priceColType == unitMeasureCol) {
            return QVariant(Qt::AlignHCenter | Qt::AlignTop);
        } else {
            return QVariant(Qt::AlignLeft | Qt::AlignTop);
        }
    }

    if( role == Qt::DisplayRole ){
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
                    ret = m_d->parser->toString( v, 'f', 2);
                } else {
                    ret = query.value(0).toString();
                }
                return QVariant(ret);
            }
        }
    }
    return QVariant();
}

QVariant PriceListDBModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole ||
            section <0 || section > m_d->visibleColsList.size())
        return QVariant();

    return QVariant( m_d->visibleColsList.at(section).userName);
}

QList< QPair<QString, int> > PriceListDBModel::unitMeasureModel() {
    QSqlQuery query( "SELECT unitMeasureId, unitMeasureOrderNum, unitMeasureTag FROM unitMeasureTable ORDER BY unitMeasureOrderNum", m_d->db );
    QList<QPair<QString, int> > ret;
    while( query.next() ){
        ret << qMakePair( query.value( "unitMeasureTag").toString(), query.value( "unitMeasureId").toInt() );
    }
    return ret;
}

QString PriceListDBModel::code(const QModelIndex &index, bool inheritFromParent) {
    // non e' necessario ordinare
    if( inheritFromParent ){
        QString ret;
        QString queryStr = QString("SELECT %1, %2 FROM priceListTable WHERE id=%3").arg( "code", "parentId", QString::number( index.internalId() ));
        QSqlQuery query = m_d->db.exec( queryStr);
        if( query.next() ){
            ret = query.value("code").toString();
            int parId = query.value("parentId").toInt();
            queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "code", QString::number( parId ));
            query = m_d->db.exec( queryStr);
            if( query.next() ){
                ret = QString("%1%2%3").arg( query.value("code").toString(), PriceItem::codeSeparator(), ret );
            }
        }
        return ret;
    } else {
        QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "code", QString::number( index.internalId() ));
        QSqlQuery query( queryStr, m_d->db );
        if( query.next() ){
            return query.value(0).toString();
        }
    }
    return QString();
}

QString PriceListDBModel::shortDescription(const QModelIndex &index, bool inheritFromParent) {
    // non e' necessario ordinare
    if( inheritFromParent ){
        QString ret;
        QString queryStr = QString("SELECT %1, %2 FROM priceListTable WHERE id=%3").arg( "shortDesc", "parentId", QString::number( index.internalId() ));
        QSqlQuery query = m_d->db.exec( queryStr);
        if( query.next() ){
            ret = query.value("shortDesc").toString();
            int parId = query.value("parentId").toInt();
            queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "shortDesc", QString::number( parId ));
            query = m_d->db.exec( queryStr);
            if( query.next() ){
                ret = QString("%1%2%3").arg( query.value("shortDesc").toString(), PriceItem::shortDescSeparator(), ret );
            }
        }
        return ret;
    } else {
        QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "shortDesc", QString::number( index.internalId() ));
        QSqlQuery query( queryStr, m_d->db );
        if( query.next() ){
            return query.value(0).toString();
        }
    }
    return QString();
}

QString PriceListDBModel::longDescription(const QModelIndex &index, bool inheritFromParent) {
    // non e' necessario ordinare
    if( inheritFromParent ){
        QString ret;
        QString queryStr = QString("SELECT %1, %2 FROM priceListTable WHERE id=%3").arg( "longDesc", "parentId", QString::number( index.internalId() ));
        QSqlQuery query = m_d->db.exec( queryStr);
        if( query.next() ){
            ret = query.value("longDesc").toString();
            int parId = query.value("parentId").toInt();
            queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "longDesc", QString::number( parId ));
            query = m_d->db.exec( queryStr);
            if( query.next() ){
                ret = QString("%1%2%3").arg( query.value("longDesc").toString(), PriceItem::longDescSeparator(), ret );
            }
        }
        return ret;
    } else {
        QString queryStr = QString("SELECT %1 FROM priceListTable WHERE id=%2").arg( "longDesc", QString::number( index.internalId() ));
        QSqlQuery query( queryStr, m_d->db );
        if( query.next() ){
            return query.value(0).toString();
        }
    }
    return QString();
}

QString PriceListDBModel::unitMeasure(const QModelIndex &index) {
    // non e' necessario ordinare
    QString queryStr = QString("SELECT unitMeasureTag, unitMeasureId, id, unitMeasure FROM priceListTable JOIN unitMeasureTable ON unitMeasureId=unitMeasure WHERE id=%1").arg( index.internalId() );
    QSqlQuery query( queryStr, m_d->db );
    if( query.next() ){
        return query.value(0).toString();
    }
    return QString();
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

QString PriceListDBModel::overheadsStr(const QModelIndex &index) {
        return QString("%1 %").arg( m_d->parser->toString( overheads(index) * 100.0, 'f', 4 ) );
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

QString PriceListDBModel::profitsStr(const QModelIndex &index) {
        return QString("%1 %").arg( m_d->parser->toString( profits(index) * 100.0, 'f', 4 ) );
}

bool PriceListDBModel::setCurrentFile(const QString & newFile ) {
    if( !(m_d->currentFile.isEmpty()) ){
        QString queryStr = QString("DETACH DATABASE fileDB");
        m_d->db.exec( queryStr );
    }

    m_d->currentFile = newFile;

    if( !(m_d->currentFile.isEmpty()) ){
        QString queryStr = QString("ATTACH DATABASE '%1' AS fileDB").arg(m_d->currentFile);
        m_d->db.exec( queryStr );

        beginResetModel();
        m_d->db.exec( "DELETE FROM unitMeasureTable" );
        m_d->db.exec( "INSERT INTO unitMeasureTable SELECT * FROM fileDB.unitMeasureTable" );
        createPriceListTable();
        endResetModel();
        qWarning( m_d->db.lastError().text().toLatin1() );
        return true;
    }
    return false;
}

void PriceListDBModel::applyFilter(const QString &filter) {
    if( filter != m_d->filter ){
        beginResetModel();
        m_d->filter = filter;
        createPriceListTable();
        endResetModel();
    }
}

void PriceListDBModel::createPriceListTable() {
    if( m_d->filter.isEmpty() ){
        m_d->db.exec( "DELETE FROM priceListTable" );
        m_d->db.exec( "INSERT INTO priceListTable SELECT * FROM fileDB.priceListTable" );
    } else {
        m_d->db.exec( "DELETE FROM priceListTable" );
        QString queryStr = QString("INSERT INTO priceListTable SELECT * FROM fileDB.priceListTable WHERE %1").arg(m_d->filter);
        m_d->db.exec( queryStr);
        addAllChildren();
        addAllParents();
    }
}

void PriceListDBModel::addAllParents(){
    QString queryStr = QString("SELECT id, parentId FROM priceListTable");
    QSqlQuery query = m_d->db.exec( queryStr );
    while( query.next() ){
        int parentId = query.value("parentId").toInt();
        if( parentId != 0 ){
            QString queryStrVerify = QString("SELECT id FROM priceListTable WHERE id=%1").arg( parentId );
            QSqlQuery queryVerify = m_d->db.exec( queryStrVerify );
            if( !queryVerify.next() ){
                QString queryStrAdd = QString("INSERT INTO priceListTable SELECT * FROM fileDB.priceListTable WHERE id=%1").arg( parentId );
                m_d->db.exec( queryStrAdd );
                query = m_d->db.exec( queryStr );
            }
        }
    }
}

void PriceListDBModel::addAllChildren(){
    QString queryStr = QString("SELECT id FROM priceListTable");
    QSqlQuery query = m_d->db.exec( queryStr );
    while( query.next() ){
        int parentId = query.value("id").toInt();
        QString queryStrVerifyFileDB = QString("SELECT id, parentId FROM fileDB.priceListTable WHERE parentId=%1").arg( parentId );
        QSqlQuery queryVerifyFileDB  = m_d->db.exec( queryStrVerifyFileDB );
        bool allIn = true;
        while( queryVerifyFileDB.next() ){
            int id = queryVerifyFileDB.value("id").toInt();
            QString queryStrVerifyMemory = QString("SELECT id FROM priceListTable WHERE id=%1").arg( id );
            QSqlQuery queryVerifyMemory  = m_d->db.exec( queryStrVerifyMemory );
            if ( !queryVerifyMemory.next()  ){
                QString queryStrAddMemory = QString("INSERT INTO priceListTable SELECT * FROM fileDB.priceListTable WHERE id=%1").arg( id );
                m_d->db.exec( queryStrAddMemory );
                allIn = false;
            }
        }
        if( !allIn ){
            query = m_d->db.exec( queryStr );
        }
    }
}

QString PriceListDBModel::currentFile() {
    return m_d->currentFile;
}

