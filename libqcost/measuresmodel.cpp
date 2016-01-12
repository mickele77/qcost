#include "measuresmodel.h"

#include "measure.h"
#include "unitmeasure.h"
#include "mathparser.h"

#include <QXmlStreamWriter>
#include <QVariant>
#include <QList>
#include <QLocale>

class MeasuresModelPrivate{
public:
    MeasuresModelPrivate( BillItem * bItem, MathParser * p, UnitMeasure * ump ):
        parserWasCreated(false),
        billItem(bItem),
        accountingBillItem(NULL),
        unitMeasure(ump),
        quantity( 0.0 ){
        if( p == NULL ){
            parser = new MathParser( QLocale::system() );
            parserWasCreated = true;
        } else {
            parser = p;
        }
    }
    MeasuresModelPrivate( AccountingBillItem * accBItem, MathParser * p, UnitMeasure * ump ):
        parserWasCreated(false),
        billItem(NULL),
        accountingBillItem(accBItem),
        unitMeasure(ump),
        quantity( 0.0 ){
        if( p == NULL ){
            parser = new MathParser( QLocale::system() );
            parserWasCreated = true;
        } else {
            parser = p;
        }
    }
    ~MeasuresModelPrivate(){
        if( parserWasCreated ){
            delete parser;
        }
    }

    bool parserWasCreated;
    BillItem * billItem;
    AccountingBillItem * accountingBillItem;
    MathParser * parser;
    UnitMeasure * unitMeasure;
    QList<Measure *> linesContainer;
    double quantity;
};

MeasuresModel::MeasuresModel(BillItem * bItem, MathParser * p, UnitMeasure * ump) :
    QAbstractTableModel(),
    m_d(new MeasuresModelPrivate( bItem, p, ump )){
    insertRows(0);

    if( m_d->unitMeasure != NULL ){
        connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &MeasuresModel::updateAllQuantities );
    }
}

MeasuresModel::MeasuresModel(AccountingBillItem *accBItem, MathParser *p, UnitMeasure *ump) :
    QAbstractTableModel(),
    m_d(new MeasuresModelPrivate( accBItem, p, ump )){
    insertRows(0);

    if( m_d->unitMeasure != NULL ){
        connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &MeasuresModel::updateAllQuantities );
    }
}

MeasuresModel::~MeasuresModel(){
    delete m_d;
}

MeasuresModel &MeasuresModel::operator=(const MeasuresModel &cp) {
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

int MeasuresModel::rowCount(const QModelIndex &) const {
    return m_d->linesContainer.size();
}

int MeasuresModel::columnCount(const QModelIndex &) const {
    return 3;
}

Qt::ItemFlags MeasuresModel::flags(const QModelIndex &index) const {
    if( index.isValid() ){
        if( index.column() == 0 || index.column() == 1 ){
            return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
        } else if( index.column() == 2 ){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
    }
    return Qt::NoItemFlags;
}

QVariant MeasuresModel::data(const QModelIndex &index, int role) const {
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

bool MeasuresModel::setData(const QModelIndex &index, const QVariant &value, int role) {
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

QVariant MeasuresModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

bool MeasuresModel::insertRows(int row, int count, const QModelIndex &parent) {
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
        Measure * itemLine = NULL;
        if(m_d->billItem != NULL ){
        itemLine = new Measure( m_d->billItem, m_d->parser, m_d->unitMeasure );
        } else if( m_d->accountingBillItem != NULL ){
            itemLine = new Measure( m_d->accountingBillItem, m_d->parser, m_d->unitMeasure );
        }
        connect( itemLine, &Measure::quantityChanged, this, &MeasuresModel::updateQuantity );
        m_d->linesContainer.insert( row, itemLine );
    }
    endInsertRows();
    emit modelChanged();
    return true;
}

bool MeasuresModel::removeRows(int row, int count, const QModelIndex &parent) {
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

bool MeasuresModel::append( int count ){
    return insertRows( m_d->linesContainer.size(), count );
}

void MeasuresModel::updateQuantity() {
    double ret = 0.0;
    for( QList<Measure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        ret += (*i)->quantity();
    }
    if( ret != m_d->quantity ){
        m_d->quantity = ret;
        emit quantityChanged( ret );
    }
}

double MeasuresModel::quantity(){
    return m_d->quantity;
}

void MeasuresModel::updateAllQuantities() {
    if( m_d->linesContainer.size() != 0 ){
        emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
    }
    updateQuantity();
}

void MeasuresModel::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->unitMeasure != ump ){
        beginResetModel();
        if( m_d->unitMeasure != NULL ){
            disconnect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &MeasuresModel::updateAllQuantities );
        }
        m_d->unitMeasure = ump;
        for( QList<Measure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
            (*i)->setUnitMeasure( ump );
        }
        if( m_d->linesContainer.size() != 0 ){
            emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
        }
        updateQuantity();
        if( m_d->unitMeasure != NULL ){
            connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &MeasuresModel::updateAllQuantities );
        }
        endResetModel();
        emit modelChanged();
    }
}

void MeasuresModel::writeXml10(QXmlStreamWriter *writer) const {
    writer->writeStartElement( "BillItemMeasuresModel" );
    for( QList<Measure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->writeXml10( writer );
    }
    writer->writeEndElement();
}

void MeasuresModel::readXml10(QXmlStreamReader *reader) {
    bool firstLine = true;
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILLITEMMEASURESMODEL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "BILLITEMMEASURE" && reader->isStartElement()) {
            if( firstLine ){
                m_d->linesContainer.last()->loadFromXml10( reader->attributes() );
                firstLine = false;
            } else {
                if(append()){
                    m_d->linesContainer.last()->loadFromXml10( reader->attributes() );
                }
            }
        }
    }
}

void MeasuresModel::writeXml20(QXmlStreamWriter *writer) const {
    writer->writeStartElement( "MeasuresModel" );
    for( QList<Measure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->writeXml20( writer );
    }
    writer->writeEndElement();
}

void MeasuresModel::readXmlTmp20(QXmlStreamReader *reader) {
    bool firstLine = true;
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "MEASURESMODEL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "MEASURE" && reader->isStartElement()) {
            if( firstLine ){
                m_d->linesContainer.last()->loadXmlTmp20( reader->attributes() );
                firstLine = false;
            } else {
                if(append()){
                    m_d->linesContainer.last()->loadXmlTmp20( reader->attributes() );
                }
            }
        }
    }
}

void MeasuresModel::readFromXmlTmp20() {
    for( QList<Measure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->loadFromXmlTmp20();
    }
}

int MeasuresModel::measuresCount() {
    return m_d->linesContainer.size();
}

Measure * MeasuresModel::measure(int i) {
    if( i >= 0 && i < m_d->linesContainer.size() ){
        return m_d->linesContainer.at(i);
    }
    return NULL;
}

QList<BillItem *> MeasuresModel::connectedBillItems() {
    QList<BillItem *> ret;
    for( QList<Measure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        QList<BillItem *> connItems = (*i)->connectedBillItems();
        for( QList<BillItem *>::iterator j = connItems.begin(); j != connItems.end(); ++j ){
            if( !(ret.contains(*j)) ){
                ret.append( *j );
            }
        }
    }
    return ret;
}

QList<AccountingBillItem *> MeasuresModel::connectedAccBillItems() {
    QList<AccountingBillItem *> ret;
    for( QList<Measure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        QList<AccountingBillItem *> connItems = (*i)->connectedAccBillItems();
        for( QList<AccountingBillItem *>::iterator j = connItems.begin(); j != connItems.end(); ++j ){
            if( !(ret.contains(*j)) ){
                ret.append( *j );
            }
        }
    }
    return ret;
}
