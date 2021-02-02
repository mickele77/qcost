#include "accountinglsmeasuresmodel.h"

#include "accountinglsmeasure.h"
#include "unitmeasure.h"
#include "mathparser.h"

#include <QXmlStreamWriter>
#include <QVariant>
#include <QList>
#include <QDate>
#include <QLocale>

class AccountingLSMeasuresModelPrivate{
public:
    AccountingLSMeasuresModelPrivate( MathParser * p, UnitMeasure * ump ):
        parserWasCreated(false),
        unitMeasure(ump),
        projQuantity( 0.0 ),
        accQuantity(0.0){
        if( p == nullptr ){
            parser = new MathParser( QLocale::system() );
            parserWasCreated = true;
        } else {
            parser = p;
        }
    }
    ~AccountingLSMeasuresModelPrivate(){
        if( parserWasCreated ){
            delete parser;
        }
    }

    MathParser * parser;
    bool parserWasCreated;
    UnitMeasure * unitMeasure;
    QList<AccountingLSMeasure *> linesContainer;
    double projQuantity;
    double accQuantity;

    static int commentCol;
    static int projFormulaCol;
    static int projQuantityCol;
    static int accDateCol;
    static int accFormulaCol;
    static int accQuantityCol;
};

int AccountingLSMeasuresModelPrivate::commentCol = 0;
int AccountingLSMeasuresModelPrivate::projFormulaCol = 1;
int AccountingLSMeasuresModelPrivate::projQuantityCol = 2;
int AccountingLSMeasuresModelPrivate::accDateCol = 3;
int AccountingLSMeasuresModelPrivate::accFormulaCol = 4;
int AccountingLSMeasuresModelPrivate::accQuantityCol = 5;

int AccountingLSMeasuresModel::accDateCol(){
    return 3;
}

AccountingLSMeasuresModel::AccountingLSMeasuresModel(MathParser * p, UnitMeasure * ump, QObject *parent) :
    QAbstractTableModel(parent),
    m_d(new AccountingLSMeasuresModelPrivate( p, ump )){
    insertRows(0);

    if( m_d->unitMeasure != nullptr ){
        connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &AccountingLSMeasuresModel::updateAllProjQuantities );
        connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &AccountingLSMeasuresModel::updateAllAccQuantities );
    }
}

AccountingLSMeasuresModel::~AccountingLSMeasuresModel(){
    delete m_d;
}

AccountingLSMeasuresModel &AccountingLSMeasuresModel::operator=(const AccountingLSMeasuresModel &cp) {
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

int AccountingLSMeasuresModel::rowCount(const QModelIndex &) const {
    return m_d->linesContainer.size();
}

int AccountingLSMeasuresModel::columnCount(const QModelIndex &) const {
    return 3+3;
}

Qt::ItemFlags AccountingLSMeasuresModel::flags(const QModelIndex &index) const {
    if( index.isValid() ){
        if( index.column() == AccountingLSMeasuresModelPrivate::commentCol || index.column() == AccountingLSMeasuresModelPrivate::projFormulaCol ){
            return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
        } else if( index.column() == AccountingLSMeasuresModelPrivate::projQuantityCol || index.column() == AccountingLSMeasuresModelPrivate::accQuantityCol || index.column() == AccountingLSMeasuresModelPrivate::accDateCol ){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        } else if( index.column() == AccountingLSMeasuresModelPrivate::accFormulaCol ){
            if( data(index, Qt::CheckStateRole).toInt() == Qt::Checked ){
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
            } else {
                return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
            }
        }
    }
    return Qt::NoItemFlags;
}

QVariant AccountingLSMeasuresModel::data(const QModelIndex &index, int role) const {
    if( index.isValid() ){
        if( index.row() >= 0 && index.row() < m_d->linesContainer.size() ){
            if( role == Qt::EditRole || role == Qt::DisplayRole ){
                if( index.column() == AccountingLSMeasuresModelPrivate::commentCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->comment());
                }
                if( index.column() == AccountingLSMeasuresModelPrivate::projFormulaCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->formula() );
                }
                if( index.column() == AccountingLSMeasuresModelPrivate::projQuantityCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->projQuantityStr());
                }
                if( index.column() == AccountingLSMeasuresModelPrivate::accDateCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->accDateStr() );
                }
                if( index.column() == AccountingLSMeasuresModelPrivate::accFormulaCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->accFormula() );
                }
                if( index.column() == AccountingLSMeasuresModelPrivate::accQuantityCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->accQuantityStr() );
                }
            }
            if( role == Qt::CheckStateRole ){
                if( index.column() == AccountingLSMeasuresModelPrivate::accFormulaCol ){
                    if( m_d->linesContainer.at(index.row())->accFormulaFromProj() ){
                        return QVariant( Qt::Checked );
                    } else {
                        return QVariant( Qt::Unchecked );
                    }
                }
            }
            if( role == Qt::TextAlignmentRole ){
                if( index.column() == AccountingLSMeasuresModelPrivate::commentCol ){
                    return Qt::AlignLeft + Qt::AlignVCenter;
                }
                if( (index.column() == AccountingLSMeasuresModelPrivate::accFormulaCol) ||
                        (index.column() == AccountingLSMeasuresModelPrivate::projFormulaCol) ){
                    return Qt::AlignLeft + Qt::AlignVCenter;
                }
                if( (index.column() == AccountingLSMeasuresModelPrivate::accQuantityCol) ||
                        (index.column() == AccountingLSMeasuresModelPrivate::projQuantityCol) ){
                    return Qt::AlignRight + Qt::AlignVCenter;
                }
            }
        }
    }
    return QVariant();
}

bool AccountingLSMeasuresModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if( index.isValid() ){
        if( index.row() >= 0 && index.row() < m_d->linesContainer.size() ){
            if( role == Qt::EditRole  ){
                if( index.column() == AccountingLSMeasuresModelPrivate::commentCol ){
                    m_d->linesContainer.at(index.row())->setComment( value.toString() );
                    emit dataChanged( index, index );
                    emit modelChanged();
                    return true;
                }
                if( index.column() == AccountingLSMeasuresModelPrivate::projFormulaCol ){
                    m_d->linesContainer.at(index.row())->setProjFormula( value.toString() );
                    emit dataChanged( index, index );
                    QModelIndex changedIndex = createIndex( index.row(), AccountingLSMeasuresModelPrivate::projQuantityCol );
                    emit dataChanged( changedIndex, changedIndex );
                    updateProjQuantity();
                    if( m_d->linesContainer.at(index.row())->accFormulaFromProj() ){
                        changedIndex = createIndex( index.row(), AccountingLSMeasuresModelPrivate::accFormulaCol );
                        emit dataChanged( changedIndex, changedIndex );
                        changedIndex = createIndex( index.row(), AccountingLSMeasuresModelPrivate::accQuantityCol );
                        emit dataChanged( changedIndex, changedIndex );
                        updateAccQuantity();
                    }
                    emit modelChanged();
                    return true;
                }
                if( index.column() == AccountingLSMeasuresModelPrivate::accFormulaCol ){
                    m_d->linesContainer.at(index.row())->setAccFormula( value.toString() );
                    emit dataChanged( index, index );
                    QModelIndex quantityIndex = createIndex( index.row(), AccountingLSMeasuresModelPrivate::accQuantityCol );
                    emit dataChanged( quantityIndex, quantityIndex );
                    updateAccQuantity();
                    emit modelChanged();
                    return true;
                }
            } else if( role == Qt::CheckStateRole  ){
                if( index.column() == AccountingLSMeasuresModelPrivate::accFormulaCol ){
                    m_d->linesContainer.at(index.row())->setAccFormulaFromProj( value.toInt() == Qt::Checked );
                    emit dataChanged( index, index );
                    QModelIndex quantityIndex = createIndex( index.row(), AccountingLSMeasuresModelPrivate::accQuantityCol );
                    emit dataChanged( quantityIndex, quantityIndex );
                }
            }
        }
    }
    return false;
}

QVariant AccountingLSMeasuresModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if( section == AccountingLSMeasuresModelPrivate::commentCol ) {
            return tr("Commento");
        }
        if( section == AccountingLSMeasuresModelPrivate::projFormulaCol ) {
            return tr("Misura prog.");
        }
        if( section == AccountingLSMeasuresModelPrivate::projQuantityCol ) {
            return tr("Quantità prog.");
        }
        if( section == AccountingLSMeasuresModelPrivate::accDateCol ) {
            return tr("Data Misura");
        }
        if( section == AccountingLSMeasuresModelPrivate::accFormulaCol ) {
            return tr("Misura");
        }
        if( section == AccountingLSMeasuresModelPrivate::accQuantityCol ) {
            return tr("Quantità");
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}

bool AccountingLSMeasuresModel::insertRows(int row, int count, const QModelIndex &parent) {
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
        AccountingLSMeasure * itemLine = new AccountingLSMeasure( m_d->parser, m_d->unitMeasure );
        connect( itemLine, &AccountingLSMeasure::projQuantityChanged, this, &AccountingLSMeasuresModel::updateProjQuantity );
        connect( itemLine, &AccountingLSMeasure::accQuantityChanged, this, &AccountingLSMeasuresModel::updateAccQuantity );
        m_d->linesContainer.insert( row, itemLine );
    }
    endInsertRows();
    emit modelChanged();
    return true;
}

bool AccountingLSMeasuresModel::removeRows(int row, int count, const QModelIndex &parent) {
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

bool AccountingLSMeasuresModel::append( int count ){
    return insertRows( m_d->linesContainer.size(), count );
}

void AccountingLSMeasuresModel::updateProjQuantity() {
    double ret = 0.0;
    for( QList<AccountingLSMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        ret += (*i)->projQuantity();
    }
    if( ret != m_d->projQuantity ){
        m_d->projQuantity = ret;
        emit projQuantityChanged( ret );
    }
}

double AccountingLSMeasuresModel::projQuantity(){
    return m_d->projQuantity;
}

void AccountingLSMeasuresModel::updateAllProjQuantities() {
    updateProjQuantity();
    if( m_d->linesContainer.size() != 0 ){
        emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
    }
}

void AccountingLSMeasuresModel::updateAccQuantity() {
    double ret = 0.0;
    for( QList<AccountingLSMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        ret += (*i)->accQuantity();
    }
    if( ret != m_d->accQuantity ){
        m_d->accQuantity = ret;
        emit accQuantityChanged( ret );
    }
}

double AccountingLSMeasuresModel::accQuantity() {
    return m_d->accQuantity;
}

double AccountingLSMeasuresModel::accQuantity(const QDate &dBegin, const QDate &dEnd) {
    double ret = 0.0;
    for( QList<AccountingLSMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        if( (*i)->accDate() >= dBegin && (*i)->accDate() <= dEnd ){
            ret += (*i)->accQuantity();
        }
    }
    return ret;
}

void AccountingLSMeasuresModel::updateAllAccQuantities() {
    if( m_d->linesContainer.size() != 0 ){
        emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
    }
    updateAccQuantity();
}

void AccountingLSMeasuresModel::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->unitMeasure != ump ){
        beginResetModel();
        if( m_d->unitMeasure != nullptr ){
            disconnect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &AccountingLSMeasuresModel::updateAllProjQuantities );
        }
        m_d->unitMeasure = ump;
        for( QList<AccountingLSMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
            (*i)->setUnitMeasure( ump );
        }
        if( m_d->linesContainer.size() != 0 ){
            emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
        }
        updateProjQuantity();
        if( m_d->unitMeasure != nullptr ){
            connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &AccountingLSMeasuresModel::updateAllProjQuantities );
        }
        endResetModel();
        emit modelChanged();
    }
}

void AccountingLSMeasuresModel::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "AccountingLSMeasuresModel" );
    for( QList<AccountingLSMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->writeXml( writer );
    }
    writer->writeEndElement();
}

void AccountingLSMeasuresModel::readXml(QXmlStreamReader *reader) {
    bool firstLine = true;
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGLSMEASURESMODEL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "ACCOUNTINGLSMEASURE" && reader->isStartElement()) {
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

int AccountingLSMeasuresModel::measuresCount() {
    return m_d->linesContainer.size();
}

AccountingLSMeasure * AccountingLSMeasuresModel::measure(int i) {
    if( i >= 0 && i < m_d->linesContainer.size() ){
        return m_d->linesContainer.at(i);
    }
    return nullptr;
}
