#include "accountingtammeasuresmodel.h"

#include "accountingtammeasure.h"
#include "unitmeasure.h"
#include "mathparser.h"

#include <QXmlStreamWriter>
#include <QVariant>
#include <QList>
#include <QDate>
#include <QLocale>

class AccountingTAMMeasuresModelPrivate{
public:
    AccountingTAMMeasuresModelPrivate( MathParser * p, UnitMeasure * ump ):
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
    ~AccountingTAMMeasuresModelPrivate(){
        if( parserWasCreated ){
            delete parser;
        }
    }

    MathParser * parser;
    bool parserWasCreated;
    UnitMeasure * unitMeasure;
    QList<AccountingTAMMeasure *> linesContainer;
    double projQuantity;
    double accQuantity;

    static int commentCol;
    static int projFormulaCol;
    static int projQuantityCol;
    static int accDateCol;
    static int accFormulaCol;
    static int accQuantityCol;
};

int AccountingTAMMeasuresModelPrivate::commentCol = 0;
int AccountingTAMMeasuresModelPrivate::projFormulaCol = 1;
int AccountingTAMMeasuresModelPrivate::projQuantityCol = 2;
int AccountingTAMMeasuresModelPrivate::accDateCol = 3;
int AccountingTAMMeasuresModelPrivate::accFormulaCol = 4;
int AccountingTAMMeasuresModelPrivate::accQuantityCol = 5;

int AccountingTAMMeasuresModel::accDateCol(){
    return 3;
}

AccountingTAMMeasuresModel::AccountingTAMMeasuresModel(MathParser * p, UnitMeasure * ump, QObject *parent) :
    QAbstractTableModel(parent),
    m_d(new AccountingTAMMeasuresModelPrivate( p, ump )){
    insertRows(0);

    if( m_d->unitMeasure != NULL ){
        connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &AccountingTAMMeasuresModel::updateAllProjQuantities );
        connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &AccountingTAMMeasuresModel::updateAllAccQuantities );
    }
}

AccountingTAMMeasuresModel::~AccountingTAMMeasuresModel(){
    delete m_d;
}

AccountingTAMMeasuresModel &AccountingTAMMeasuresModel::operator=(const AccountingTAMMeasuresModel &cp) {
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

int AccountingTAMMeasuresModel::rowCount(const QModelIndex &) const {
    return m_d->linesContainer.size();
}

int AccountingTAMMeasuresModel::columnCount(const QModelIndex &) const {
    return 3+3;
}

Qt::ItemFlags AccountingTAMMeasuresModel::flags(const QModelIndex &index) const {
    if( index.isValid() ){
        if( index.column() == AccountingTAMMeasuresModelPrivate::commentCol || index.column() == AccountingTAMMeasuresModelPrivate::projFormulaCol ){
            return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
        } else if( index.column() == AccountingTAMMeasuresModelPrivate::projQuantityCol || index.column() == AccountingTAMMeasuresModelPrivate::accQuantityCol || index.column() == AccountingTAMMeasuresModelPrivate::accDateCol ){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        } else if( index.column() == AccountingTAMMeasuresModelPrivate::accFormulaCol ){
            if( data(index, Qt::CheckStateRole).toInt() == Qt::Checked ){
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
            } else {
                return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
            }
        }
    }
    return Qt::NoItemFlags;
}

QVariant AccountingTAMMeasuresModel::data(const QModelIndex &index, int role) const {
    if( index.isValid() ){
        if( index.row() >= 0 && index.row() < m_d->linesContainer.size() ){
            if( role == Qt::EditRole || role == Qt::DisplayRole ){
                if( index.column() == AccountingTAMMeasuresModelPrivate::commentCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->comment());
                }
                if( index.column() == AccountingTAMMeasuresModelPrivate::projFormulaCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->projFormula() );
                }
                if( index.column() == AccountingTAMMeasuresModelPrivate::projQuantityCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->projQuantityStr());
                }
                if( index.column() == AccountingTAMMeasuresModelPrivate::accDateCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->accDateStr() );
                }
                if( index.column() == AccountingTAMMeasuresModelPrivate::accFormulaCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->accFormula() );
                }
                if( index.column() == AccountingTAMMeasuresModelPrivate::accQuantityCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->accQuantityStr() );
                }
            }
            if( role == Qt::CheckStateRole ){
                if( index.column() == AccountingTAMMeasuresModelPrivate::accFormulaCol ){
                    if( m_d->linesContainer.at(index.row())->accFormulaFromProj() ){
                        return QVariant( Qt::Checked );
                    } else {
                        return QVariant( Qt::Unchecked );
                    }
                }
            }
            if( role == Qt::TextAlignmentRole ){
                if( index.column() == AccountingTAMMeasuresModelPrivate::commentCol ){
                    return Qt::AlignLeft + Qt::AlignVCenter;
                }
                if( (index.column() == AccountingTAMMeasuresModelPrivate::accFormulaCol) ||
                        (index.column() == AccountingTAMMeasuresModelPrivate::projFormulaCol) ){
                    return Qt::AlignLeft + Qt::AlignVCenter;
                }
                if( (index.column() == AccountingTAMMeasuresModelPrivate::accQuantityCol) ||
                        (index.column() == AccountingTAMMeasuresModelPrivate::projQuantityCol) ){
                    return Qt::AlignRight + Qt::AlignVCenter;
                }
            }
        }
    }
    return QVariant();
}

bool AccountingTAMMeasuresModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if( index.isValid() ){
        if( index.row() >= 0 && index.row() < m_d->linesContainer.size() ){
            if( role == Qt::EditRole  ){
                if( index.column() == AccountingTAMMeasuresModelPrivate::commentCol ){
                    m_d->linesContainer.at(index.row())->setComment( value.toString() );
                    emit dataChanged( index, index );
                    emit modelChanged();
                    return true;
                }
                if( index.column() == AccountingTAMMeasuresModelPrivate::projFormulaCol ){
                    m_d->linesContainer.at(index.row())->setProjFormula( value.toString() );
                    emit dataChanged( index, index );
                    QModelIndex changedIndex = createIndex( index.row(), AccountingTAMMeasuresModelPrivate::projQuantityCol );
                    emit dataChanged( changedIndex, changedIndex );
                    updateProjQuantity();
                    if( m_d->linesContainer.at(index.row())->accFormulaFromProj() ){
                        changedIndex = createIndex( index.row(), AccountingTAMMeasuresModelPrivate::accFormulaCol );
                        emit dataChanged( changedIndex, changedIndex );
                        changedIndex = createIndex( index.row(), AccountingTAMMeasuresModelPrivate::accQuantityCol );
                        emit dataChanged( changedIndex, changedIndex );
                        updateAccQuantity();
                    }
                    emit modelChanged();
                    return true;
                }
                if( index.column() == AccountingTAMMeasuresModelPrivate::accFormulaCol ){
                    m_d->linesContainer.at(index.row())->setAccFormula( value.toString() );
                    emit dataChanged( index, index );
                    QModelIndex quantityIndex = createIndex( index.row(), AccountingTAMMeasuresModelPrivate::accQuantityCol );
                    emit dataChanged( quantityIndex, quantityIndex );
                    updateAccQuantity();
                    emit modelChanged();
                    return true;
                }
            } else if( role == Qt::CheckStateRole  ){
                if( index.column() == AccountingTAMMeasuresModelPrivate::accFormulaCol ){
                    m_d->linesContainer.at(index.row())->setAccFormulaFromProj( value.toInt() == Qt::Checked );
                    emit dataChanged( index, index );
                    QModelIndex quantityIndex = createIndex( index.row(), AccountingTAMMeasuresModelPrivate::accQuantityCol );
                    emit dataChanged( quantityIndex, quantityIndex );
                }
            }
        }
    }
    return false;
}

QVariant AccountingTAMMeasuresModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if( section == AccountingTAMMeasuresModelPrivate::commentCol ) {
            return trUtf8("Commento");
        }
        if( section == AccountingTAMMeasuresModelPrivate::projFormulaCol ) {
            return trUtf8("Misura prog.");
        }
        if( section == AccountingTAMMeasuresModelPrivate::projQuantityCol ) {
            return trUtf8("Quantità prog.");
        }
        if( section == AccountingTAMMeasuresModelPrivate::accDateCol ) {
            return trUtf8("Data Misura");
        }
        if( section == AccountingTAMMeasuresModelPrivate::accFormulaCol ) {
            return trUtf8("Misura");
        }
        if( section == AccountingTAMMeasuresModelPrivate::accQuantityCol ) {
            return trUtf8("Quantità");
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}

bool AccountingTAMMeasuresModel::insertRows(int row, int count, const QModelIndex &parent) {
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
        AccountingTAMMeasure * itemLine = new AccountingTAMMeasure( m_d->parser, m_d->unitMeasure );
        connect( itemLine, &AccountingTAMMeasure::projQuantityChanged, this, &AccountingTAMMeasuresModel::updateProjQuantity );
        connect( itemLine, &AccountingTAMMeasure::accQuantityChanged, this, &AccountingTAMMeasuresModel::updateAccQuantity );
        m_d->linesContainer.insert( row, itemLine );
    }
    endInsertRows();
    emit modelChanged();
    return true;
}

bool AccountingTAMMeasuresModel::removeRows(int row, int count, const QModelIndex &parent) {
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

bool AccountingTAMMeasuresModel::append( int count ){
    return insertRows( m_d->linesContainer.size(), count );
}

void AccountingTAMMeasuresModel::updateProjQuantity() {
    double ret = 0.0;
    for( QList<AccountingTAMMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        ret += (*i)->projQuantity();
    }
    if( ret != m_d->projQuantity ){
        m_d->projQuantity = ret;
        emit projQuantityChanged( ret );
    }
}

double AccountingTAMMeasuresModel::projQuantity(){
    return m_d->projQuantity;
}

void AccountingTAMMeasuresModel::updateAllProjQuantities() {
    updateProjQuantity();
    if( m_d->linesContainer.size() != 0 ){
        emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
    }
}

void AccountingTAMMeasuresModel::updateAccQuantity() {
    double ret = 0.0;
    for( QList<AccountingTAMMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        ret += (*i)->accQuantity();
    }
    if( ret != m_d->accQuantity ){
        m_d->accQuantity = ret;
        emit accQuantityChanged( ret );
    }
}

double AccountingTAMMeasuresModel::accQuantity() {
    return m_d->accQuantity;
}

double AccountingTAMMeasuresModel::accQuantity(const QDate &dBegin, const QDate &dEnd) {
    double ret = 0.0;
    for( QList<AccountingTAMMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        if( (*i)->accDate() >= dBegin && (*i)->accDate() <= dEnd ){
            ret += (*i)->accQuantity();
        }
    }
    return ret;
}

void AccountingTAMMeasuresModel::updateAllAccQuantities() {
    if( m_d->linesContainer.size() != 0 ){
        emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
    }
    updateAccQuantity();
}

void AccountingTAMMeasuresModel::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->unitMeasure != ump ){
        beginResetModel();
        if( m_d->unitMeasure != NULL ){
            disconnect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &AccountingTAMMeasuresModel::updateAllProjQuantities );
        }
        m_d->unitMeasure = ump;
        for( QList<AccountingTAMMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
            (*i)->setUnitMeasure( ump );
        }
        if( m_d->linesContainer.size() != 0 ){
            emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
        }
        updateProjQuantity();
        if( m_d->unitMeasure != NULL ){
            connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &AccountingTAMMeasuresModel::updateAllProjQuantities );
        }
        endResetModel();
        emit modelChanged();
    }
}

void AccountingTAMMeasuresModel::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "MeasuresLSModel" );
    for( QList<AccountingTAMMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->writeXml( writer );
    }
    writer->writeEndElement();
}

void AccountingTAMMeasuresModel::readXml(QXmlStreamReader *reader) {
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

int AccountingTAMMeasuresModel::measuresCount() {
    return m_d->linesContainer.size();
}

AccountingTAMMeasure * AccountingTAMMeasuresModel::measure(int i) {
    if( i >= 0 && i < m_d->linesContainer.size() ){
        return m_d->linesContainer.at(i);
    }
    return NULL;
}
