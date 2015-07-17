#include "billitemmeasuresmodel.h"

#include "billitemmeasure.h"
#include "unitmeasure.h"
#include "mathparser.h"

#include <QXmlStreamWriter>
#include <QVariant>
#include <QList>
#include <QLocale>

class BillItemMeasuresModelPrivate{
public:
    BillItemMeasuresModelPrivate( MathParser * p, UnitMeasure * ump ):
        parserWasCreated(false),
        unitMeasure(ump),
        quantity( 0.0 ){
        if( p == NULL ){
            parser = new MathParser( QLocale::system() );
            parserWasCreated = true;
        } else {
            parser = p;
        }
    };
    ~BillItemMeasuresModelPrivate(){
        if( parserWasCreated ){
            delete parser;
        }
    }

    MathParser * parser;
    bool parserWasCreated;
    UnitMeasure * unitMeasure;
    QList<BillItemMeasure *> linesContainer;
    double quantity;
};

BillItemMeasuresModel::BillItemMeasuresModel(MathParser * p, UnitMeasure * ump, QObject *parent) :
    QAbstractTableModel(parent),
    m_d(new BillItemMeasuresModelPrivate( p, ump )){
    insertRows(0);

    if( m_d->unitMeasure != NULL ){
        connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &BillItemMeasuresModel::updateAllQuantities );
    }
}

BillItemMeasuresModel::~BillItemMeasuresModel(){
    delete m_d;
}

BillItemMeasuresModel &BillItemMeasuresModel::operator=(const BillItemMeasuresModel &cp) {
    if( &cp != this ){
        setUnitMeasure( cp.m_d->unitMeasure );
        if( m_d->linesContainer.size() > cp.m_d->linesContainer.size() ){
            removeRows( 0, m_d->linesContainer.size() - cp.m_d->linesContainer.size() );
        } else if( m_d->linesContainer.size() < cp.m_d->linesContainer.size() ){
            insertRows( 0, cp.m_d->linesContainer.size() - m_d->linesContainer.size() );
        }
        for( int i = 0; i < m_d->linesContainer.size(); ++i ){
            *(m_d->linesContainer[i]) = *(cp.m_d->linesContainer.at(i));
        }
    }

    return *this;
}

int BillItemMeasuresModel::rowCount(const QModelIndex &) const {
    return m_d->linesContainer.size();
}

int BillItemMeasuresModel::columnCount(const QModelIndex &) const {
    return 3;
}

Qt::ItemFlags BillItemMeasuresModel::flags(const QModelIndex &index) const {
    if( index.isValid() ){
        if( index.column() == 0 || index.column() == 1 ){
            return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
        } else if( index.column() == 2 ){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
    }
    return Qt::NoItemFlags;
}

QVariant BillItemMeasuresModel::data(const QModelIndex &index, int role) const {
    if( index.isValid() ){
        if( index.row() >= 0 && index.row() < m_d->linesContainer.size() ){
            if( role == Qt::EditRole || role == Qt::DisplayRole ){
                if( index.column() == 0 ){
                    return QVariant( m_d->linesContainer.at(index.row())->comment());
                }
                if( index.column() == 1 ){
                    return QVariant( m_d->linesContainer.at(index.row())->formula() );
                }
                if( index.column() == 2 ){
                    return QVariant( m_d->linesContainer.at(index.row())->quantityStr());
                }
            }
            if( role == Qt::TextAlignmentRole ){
                if( index.column() == 0 ){
                    return Qt::AlignLeft + Qt::AlignVCenter;
                }
                if( index.column() == 1 ){
                    return Qt::AlignLeft + Qt::AlignVCenter;
                }
                if( index.column() == 2 ){
                    return Qt::AlignRight + Qt::AlignVCenter;
                }
            }
        }
    }
    return QVariant();
}

bool BillItemMeasuresModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if( index.isValid() ){
        if( index.row() >= 0 && index.row() < m_d->linesContainer.size() ){
            if( role == Qt::EditRole  ){
                if( index.column() == 0 ){
                    m_d->linesContainer.at(index.row())->setComment( value.toString() );
                    emit dataChanged( index, index );
                    emit modelChanged();
                    return true;
                }
                if( index.column() == 1 ){
                    m_d->linesContainer.at(index.row())->setFormula( value.toString() );
                    QModelIndex bottomRight = createIndex( index.row(), 2 );
                    emit dataChanged( index, bottomRight );
                    updateQuantity();
                    emit modelChanged();
                    return true;
                }
            }
        }
    }
    return false;
}

QVariant BillItemMeasuresModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if( section == 0 ) {
            return trUtf8("Commento");
        }
        if( section == 1 ) {
            return trUtf8("Misure");
        }
        if( section == 2 ) {
            return trUtf8("Quantità");
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}

bool BillItemMeasuresModel::insertRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED(parent);
    if( count < 1 ){
        return false;
    }
    if( row < 0 ){
        row = 0;
    }
    if( row > m_d->linesContainer.size() ){
        row = m_d->linesContainer.size();
    }
    beginInsertRows(QModelIndex(), row, row+count-1 );
    for(int i=0; i < count; ++i){
        BillItemMeasure * itemLine = new BillItemMeasure( m_d->parser, m_d->unitMeasure );
        connect( itemLine, &BillItemMeasure::quantityChanged, this, &BillItemMeasuresModel::updateQuantity );
        m_d->linesContainer.insert( row, itemLine );
    }
    endInsertRows();
    emit modelChanged();
    return true;
}

bool BillItemMeasuresModel::removeRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED(parent);

    if( count < 1 || row < 0 || row > m_d->linesContainer.size() ){
        return false;
    }

    if( (row+count) > m_d->linesContainer.size() ){
        count = m_d->linesContainer.size() - row;
    }

    // lasciamo almeno una linea
    if( count == m_d->linesContainer.size() ){
        count--;
        row = 1;
    }
    if( count < 1 ){
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row+count-1);
    for(int i=0; i < count; ++i){
        delete m_d->linesContainer.at(row);
        m_d->linesContainer.removeAt( row );
    }
    endRemoveRows();
    updateQuantity();
    emit modelChanged();
    return true;
}

bool BillItemMeasuresModel::append( int count ){
    return insertRows( m_d->linesContainer.size(), count );
}

void BillItemMeasuresModel::updateQuantity() {
    double ret = 0.0;
    for( QList<BillItemMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        ret += (*i)->quantity();
    }
    if( ret != m_d->quantity ){
        m_d->quantity = ret;
        emit quantityChanged( ret );
    }
}

double BillItemMeasuresModel::quantity(){
    return m_d->quantity;
}

void BillItemMeasuresModel::updateAllQuantities() {
    if( m_d->linesContainer.size() != 0 ){
        emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
    }
    updateQuantity();
}

void BillItemMeasuresModel::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->unitMeasure != ump ){
        beginResetModel();
        if( m_d->unitMeasure != NULL ){
            disconnect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &BillItemMeasuresModel::updateAllQuantities );
        }
        m_d->unitMeasure = ump;
        for( QList<BillItemMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
            (*i)->setUnitMeasure( ump );
        }
        if( m_d->linesContainer.size() != 0 ){
            emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
        }
        updateQuantity();
        if( m_d->unitMeasure != NULL ){
            connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &BillItemMeasuresModel::updateAllQuantities );
        }
        endResetModel();
        emit modelChanged();
    }
}

void BillItemMeasuresModel::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "BillItemMeasuresModel" );
    for( QList<BillItemMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->writeXml( writer );
    }
    writer->writeEndElement();
}

void BillItemMeasuresModel::readXml(QXmlStreamReader *reader) {
    bool firstLine = true;
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILLITEMMEASURESMODEL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "BILLITEMMEASURE" && reader->isStartElement()) {
            if( firstLine ){
                m_d->linesContainer.last()->loadFromXml( reader->attributes() );
                firstLine = false;
            } else {
                if(append()){
                    m_d->linesContainer.last()->loadFromXml( reader->attributes() );
                }
            }
        }
    }
}

int BillItemMeasuresModel::billItemMeasureCount() {
    return m_d->linesContainer.size();
}

BillItemMeasure * BillItemMeasuresModel::measure(int i) {
    if( i >= 0 && i < m_d->linesContainer.size() ){
        return m_d->linesContainer.at(i);
    }
    return NULL;
}
