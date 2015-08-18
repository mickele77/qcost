#include "measureslsmodel.h"

#include "accountinglsitemmeasure.h"
#include "unitmeasure.h"
#include "mathparser.h"

#include <QXmlStreamWriter>
#include <QVariant>
#include <QList>
#include <QDate>
#include <QLocale>

class MeasuresLSModelPrivate{
public:
    MeasuresLSModelPrivate( MathParser * p, UnitMeasure * ump ):
        parserWasCreated(false),
        unitMeasure(ump),
        projQuantity( 0.0 ){
        if( p == NULL ){
            parser = new MathParser( QLocale::system() );
            parserWasCreated = true;
        } else {
            parser = p;
        }
    }
    ~MeasuresLSModelPrivate(){
        if( parserWasCreated ){
            delete parser;
        }
    }

    MathParser * parser;
    bool parserWasCreated;
    UnitMeasure * unitMeasure;
    QList<AccountingLSItemMeasure *> linesContainer;
    double projQuantity;
    double accQuantity;

    static int commentCol;
    static int projFormulaCol;
    static int projQuantityCol;
    static int accDateCol;
    static int accFormulaCol;
    static int accQuantityCol;
};

int MeasuresLSModelPrivate::commentCol = 0;
int MeasuresLSModelPrivate::projFormulaCol = 1;
int MeasuresLSModelPrivate::projQuantityCol = 2;
int MeasuresLSModelPrivate::accDateCol = 3;
int MeasuresLSModelPrivate::accFormulaCol = 4;
int MeasuresLSModelPrivate::accQuantityCol = 5;

int MeasuresLSModel::accDateCol(){
    return 3;
}

MeasuresLSModel::MeasuresLSModel(MathParser * p, UnitMeasure * ump, QObject *parent) :
    QAbstractTableModel(parent),
    m_d(new MeasuresLSModelPrivate( p, ump )){
    insertRows(0);

    if( m_d->unitMeasure != NULL ){
        connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &MeasuresLSModel::updateAllProjQuantities );
        connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &MeasuresLSModel::updateAllAccQuantities );
    }
}

MeasuresLSModel::~MeasuresLSModel(){
    delete m_d;
}

MeasuresLSModel &MeasuresLSModel::operator=(const MeasuresLSModel &cp) {
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

int MeasuresLSModel::rowCount(const QModelIndex &) const {
    return m_d->linesContainer.size();
}

int MeasuresLSModel::columnCount(const QModelIndex &) const {
    return 3+3;
}

Qt::ItemFlags MeasuresLSModel::flags(const QModelIndex &index) const {
    if( index.isValid() ){
        if( index.column() == MeasuresLSModelPrivate::commentCol || index.column() == MeasuresLSModelPrivate::projFormulaCol ){
            return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
        } else if( index.column() == MeasuresLSModelPrivate::projQuantityCol || index.column() == MeasuresLSModelPrivate::accQuantityCol || index.column() == MeasuresLSModelPrivate::accDateCol ){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        } else if( index.column() == MeasuresLSModelPrivate::accFormulaCol ){
            if( data(index, Qt::CheckStateRole).toInt() == Qt::Checked ){
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
            } else {
                return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
            }
        }
    }
    return Qt::NoItemFlags;
}

QVariant MeasuresLSModel::data(const QModelIndex &index, int role) const {
    if( index.isValid() ){
        if( index.row() >= 0 && index.row() < m_d->linesContainer.size() ){
            if( role == Qt::EditRole || role == Qt::DisplayRole ){
                if( index.column() == MeasuresLSModelPrivate::commentCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->comment());
                }
                if( index.column() == MeasuresLSModelPrivate::projFormulaCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->projFormula() );
                }
                if( index.column() == MeasuresLSModelPrivate::projQuantityCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->projQuantityStr());
                }
                if( index.column() == MeasuresLSModelPrivate::accDateCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->accDateStr() );
                }
                if( index.column() == MeasuresLSModelPrivate::accFormulaCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->accFormula() );
                }
                if( index.column() == MeasuresLSModelPrivate::accQuantityCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->accQuantityStr() );
                }
            }
            if( role == Qt::CheckStateRole ){
                if( index.column() == MeasuresLSModelPrivate::accFormulaCol ){
                    if( m_d->linesContainer.at(index.row())->accFormulaFromProj() ){
                        return QVariant( Qt::Checked );
                    } else {
                        return QVariant( Qt::Unchecked );
                    }
                }
            }
            if( role == Qt::TextAlignmentRole ){
                if( index.column() == MeasuresLSModelPrivate::commentCol ){
                    return Qt::AlignLeft + Qt::AlignVCenter;
                }
                if( (index.column() == MeasuresLSModelPrivate::accFormulaCol) ||
                        (index.column() == MeasuresLSModelPrivate::projFormulaCol) ){
                    return Qt::AlignLeft + Qt::AlignVCenter;
                }
                if( (index.column() == MeasuresLSModelPrivate::accQuantityCol) ||
                        (index.column() == MeasuresLSModelPrivate::projQuantityCol) ){
                    return Qt::AlignRight + Qt::AlignVCenter;
                }
            }
        }
    }
    return QVariant();
}

bool MeasuresLSModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if( index.isValid() ){
        if( index.row() >= 0 && index.row() < m_d->linesContainer.size() ){
            if( role == Qt::EditRole  ){
                if( index.column() == MeasuresLSModelPrivate::commentCol ){
                    m_d->linesContainer.at(index.row())->setComment( value.toString() );
                    emit dataChanged( index, index );
                    emit modelChanged();
                    return true;
                }
                if( index.column() == MeasuresLSModelPrivate::projFormulaCol ){
                    m_d->linesContainer.at(index.row())->setProjFormula( value.toString() );
                    emit dataChanged( index, index );
                    QModelIndex changedIndex = createIndex( index.row(), MeasuresLSModelPrivate::projQuantityCol );
                    emit dataChanged( changedIndex, changedIndex );
                    updateProjQuantity();
                    if( m_d->linesContainer.at(index.row())->accFormulaFromProj() ){
                        changedIndex = createIndex( index.row(), MeasuresLSModelPrivate::accFormulaCol );
                        emit dataChanged( changedIndex, changedIndex );
                        changedIndex = createIndex( index.row(), MeasuresLSModelPrivate::accQuantityCol );
                        emit dataChanged( changedIndex, changedIndex );
                        updateAccQuantity();
                    }
                    emit modelChanged();
                    return true;
                }
                if( index.column() == MeasuresLSModelPrivate::accFormulaCol ){
                    m_d->linesContainer.at(index.row())->setAccFormula( value.toString() );
                    emit dataChanged( index, index );
                    QModelIndex quantityIndex = createIndex( index.row(), MeasuresLSModelPrivate::accQuantityCol );
                    emit dataChanged( quantityIndex, quantityIndex );
                    updateAccQuantity();
                    emit modelChanged();
                    return true;
                }
            } else if( role == Qt::CheckStateRole  ){
                if( index.column() == MeasuresLSModelPrivate::accFormulaCol ){
                    m_d->linesContainer.at(index.row())->setAccFormulaFromProj( value.toInt() == Qt::Checked );
                    emit dataChanged( index, index );
                    QModelIndex quantityIndex = createIndex( index.row(), MeasuresLSModelPrivate::accQuantityCol );
                    emit dataChanged( quantityIndex, quantityIndex );
                }
            }
        }
    }
    return false;
}

QVariant MeasuresLSModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if( section == MeasuresLSModelPrivate::commentCol ) {
            return trUtf8("Commento");
        }
        if( section == MeasuresLSModelPrivate::projFormulaCol ) {
            return trUtf8("Misura prog.");
        }
        if( section == MeasuresLSModelPrivate::projQuantityCol ) {
            return trUtf8("Quantità prog.");
        }
        if( section == MeasuresLSModelPrivate::accDateCol ) {
            return trUtf8("Data Misura");
        }
        if( section == MeasuresLSModelPrivate::accFormulaCol ) {
            return trUtf8("Misura");
        }
        if( section == MeasuresLSModelPrivate::accQuantityCol ) {
            return trUtf8("Quantità");
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}

bool MeasuresLSModel::insertRows(int row, int count, const QModelIndex &parent) {
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
        AccountingLSItemMeasure * itemLine = new AccountingLSItemMeasure( m_d->parser, m_d->unitMeasure );
        connect( itemLine, &AccountingLSItemMeasure::projQuantityChanged, this, &MeasuresLSModel::updateProjQuantity );
        connect( itemLine, &AccountingLSItemMeasure::accQuantityChanged, this, &MeasuresLSModel::updateAccQuantity );
        m_d->linesContainer.insert( row, itemLine );
    }
    endInsertRows();
    emit modelChanged();
    return true;
}

bool MeasuresLSModel::removeRows(int row, int count, const QModelIndex &parent) {
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
    updateProjQuantity();
    emit modelChanged();
    return true;
}

bool MeasuresLSModel::append( int count ){
    return insertRows( m_d->linesContainer.size(), count );
}

void MeasuresLSModel::updateProjQuantity() {
    double ret = 0.0;
    for( QList<AccountingLSItemMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        ret += (*i)->projQuantity();
    }
    if( ret != m_d->projQuantity ){
        m_d->projQuantity = ret;
        emit projQuantityChanged( ret );
    }
}

double MeasuresLSModel::projQuantity(){
    return m_d->projQuantity;
}

void MeasuresLSModel::updateAllProjQuantities() {
    updateProjQuantity();
    if( m_d->linesContainer.size() != 0 ){
        emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
    }
}

void MeasuresLSModel::updateAccQuantity() {
    double ret = 0.0;
    for( QList<AccountingLSItemMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        ret += (*i)->accQuantity();
    }
    if( ret != m_d->accQuantity ){
        m_d->accQuantity = ret;
        emit accQuantityChanged( ret );
    }
}

double MeasuresLSModel::accQuantity() {
    return m_d->accQuantity;
}

double MeasuresLSModel::accQuantity(const QDate &dBegin, const QDate &dEnd) {
    double ret = 0.0;
    for( QList<AccountingLSItemMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        if( (*i)->accDate() >= dBegin && (*i)->accDate() <= dEnd ){
            ret += (*i)->accQuantity();
        }
    }
    return ret;
}

void MeasuresLSModel::updateAllAccQuantities() {
    if( m_d->linesContainer.size() != 0 ){
        emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
    }
    updateAccQuantity();
}

void MeasuresLSModel::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->unitMeasure != ump ){
        beginResetModel();
        if( m_d->unitMeasure != NULL ){
            disconnect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &MeasuresLSModel::updateAllProjQuantities );
        }
        m_d->unitMeasure = ump;
        for( QList<AccountingLSItemMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
            (*i)->setUnitMeasure( ump );
        }
        if( m_d->linesContainer.size() != 0 ){
            emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
        }
        updateProjQuantity();
        if( m_d->unitMeasure != NULL ){
            connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &MeasuresLSModel::updateAllProjQuantities );
        }
        endResetModel();
        emit modelChanged();
    }
}

void MeasuresLSModel::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "MeasuresLSModel" );
    for( QList<AccountingLSItemMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->writeXml( writer );
    }
    writer->writeEndElement();
}

void MeasuresLSModel::readXml(QXmlStreamReader *reader) {
    bool firstLine = true;
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "MEASURESLSMODEL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "MEASURELS" && reader->isStartElement()) {
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

int MeasuresLSModel::measuresCount() {
    return m_d->linesContainer.size();
}

AccountingLSItemMeasure * MeasuresLSModel::measure(int i) {
    if( i >= 0 && i < m_d->linesContainer.size() ){
        return m_d->linesContainer.at(i);
    }
    return NULL;
}
