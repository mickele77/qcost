#include "varsmodel.h"

#include "var.h"
#include "mathparser.h"

#include <QXmlStreamWriter>
#include <QVariant>
#include <QList>
#include <QLocale>

class VarsModelPrivate{
public:
    VarsModelPrivate( BillItem * bItem, MathParser * p ):
        parserWasCreated(false),
        billItem(bItem),
        unitVar(ump),
        quantity( 0.0 ){
        if( p == NULL ){
            parser = new MathParser( QLocale::system() );
            parserWasCreated = true;
        } else {
            parser = p;
        }
    }
    ~VarsModelPrivate(){
        if( parserWasCreated ){
            delete parser;
        }
    }

    bool parserWasCreated;
    BillItem * billItem;
    MathParser * parser;
    UnitVar * unitVar;
    QList<Var *> linesContainer;
    double quantity;
};

VarsModel::VarsModel(BillItem * bItem, MathParser * p) :
    QAbstractTableModel(),
    m_d(new VarsModelPrivate( bItem, p, ump )){
    insertRows(0);

    if( m_d->unitVar != NULL ){
        connect( m_d->unitVar, &UnitVar::precisionChanged, this, &VarsModel::updateAllQuantities );
    }
}

VarsModel::~VarsModel(){
    delete m_d;
}

VarsModel &VarsModel::operator=(const VarsModel &cp) {
    if( &cp != this ){
        setUnitVar( cp.m_d->unitVar );
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

int VarsModel::rowCount(const QModelIndex &) const {
    return m_d->linesContainer.size();
}

int VarsModel::columnCount(const QModelIndex &) const {
    return 3;
}

Qt::ItemFlags VarsModel::flags(const QModelIndex &index) const {
    if( index.isValid() ){
        if( index.column() == 0 || index.column() == 1 ){
            return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
        } else if( index.column() == 2 ){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
    }
    return Qt::NoItemFlags;
}

QVariant VarsModel::data(const QModelIndex &index, int role) const {
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

bool VarsModel::setData(const QModelIndex &index, const QVariant &value, int role) {
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

QVariant VarsModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

bool VarsModel::insertRows(int row, int count, const QModelIndex &parent) {
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
        Var * itemLine = new Var( m_d->billItem, m_d->parser, m_d->unitVar );
        connect( itemLine, &Var::quantityChanged, this, &VarsModel::updateQuantity );
        m_d->linesContainer.insert( row, itemLine );
    }
    endInsertRows();
    emit modelChanged();
    return true;
}

bool VarsModel::removeRows(int row, int count, const QModelIndex &parent) {
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

bool VarsModel::append( int count ){
    return insertRows( m_d->linesContainer.size(), count );
}

void VarsModel::updateQuantity() {
    double ret = 0.0;
    for( QList<Var *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        ret += (*i)->quantity();
    }
    if( ret != m_d->quantity ){
        m_d->quantity = ret;
        emit quantityChanged( ret );
    }
}

double VarsModel::quantity(){
    return m_d->quantity;
}

void VarsModel::updateAllQuantities() {
    if( m_d->linesContainer.size() != 0 ){
        emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
    }
    updateQuantity();
}

void VarsModel::setUnitVar(UnitVar *ump) {
    if( m_d->unitVar != ump ){
        beginResetModel();
        if( m_d->unitVar != NULL ){
            disconnect( m_d->unitVar, &UnitVar::precisionChanged, this, &VarsModel::updateAllQuantities );
        }
        m_d->unitVar = ump;
        for( QList<Var *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
            (*i)->setUnitVar( ump );
        }
        if( m_d->linesContainer.size() != 0 ){
            emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
        }
        updateQuantity();
        if( m_d->unitVar != NULL ){
            connect( m_d->unitVar, &UnitVar::precisionChanged, this, &VarsModel::updateAllQuantities );
        }
        endResetModel();
        emit modelChanged();
    }
}

void VarsModel::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "VarsModel" );
    for( QList<Var *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->writeXml( writer );
    }
    writer->writeEndElement();
}

void VarsModel::readXmlTmp(QXmlStreamReader *reader) {
    bool firstLine = true;
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "MEASURESMODEL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "MEASURE" && reader->isStartElement()) {
            if( firstLine ){
                m_d->linesContainer.last()->loadXmlTmp( reader->attributes() );
                firstLine = false;
            } else {
                if(append()){
                    m_d->linesContainer.last()->loadXmlTmp( reader->attributes() );
                }
            }
        }
    }
}

void VarsModel::readFromXmlTmp() {
    for( QList<Var *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->loadFromXmlTmp();
    }
}

int VarsModel::varsCount() {
    return m_d->linesContainer.size();
}

Var * VarsModel::var(int i) {
    if( i >= 0 && i < m_d->linesContainer.size() ){
        return m_d->linesContainer.at(i);
    }
    return NULL;
}

QList<BillItem *> VarsModel::connectedBillItems() {
    QList<BillItem *> ret;
    for( QList<Var *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        QList<BillItem *> connItems = (*i)->connectedBillItems();
        for( QList<BillItem *>::iterator j = connItems.begin(); j != connItems.end(); ++j ){
            if( !(ret.contains(*j)) ){
                ret.append( *j );
            }
        }
    }
    return ret;
}
