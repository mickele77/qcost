#include "accountingtammeasuresmodel.h"

#include "accountingtambillitem.h"
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
    AccountingTAMMeasuresModelPrivate( AccountingTAMBillItem * tamBItem, MathParser * p, UnitMeasure * ump ):
        tamBillItem( tamBItem ),
        parserWasCreated(false),
        unitMeasure(ump),
        quantity( 0.0 ){
        if( p == nullptr ){
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

    int quantityCol(){
        return firstFormulaCol + tamBillItem->daysCount() ;
    }

    int lastFormulaCol(){
        return firstFormulaCol + tamBillItem->daysCount() - 1;
    }

    AccountingTAMBillItem * tamBillItem;
    bool parserWasCreated;
    UnitMeasure * unitMeasure;
    QList<AccountingTAMMeasure *> linesContainer;
    MathParser * parser;
    double quantity;


    static int commentCol;
    static int firstFormulaCol;
};

int AccountingTAMMeasuresModelPrivate::commentCol = 0;
int AccountingTAMMeasuresModelPrivate::firstFormulaCol = 1;

AccountingTAMMeasuresModel::AccountingTAMMeasuresModel( AccountingTAMBillItem * tamBItem, MathParser * p, UnitMeasure * ump, QObject *parent) :
    QAbstractTableModel(parent),
    m_d(new AccountingTAMMeasuresModelPrivate( tamBItem, p, ump )){
    insertRows(0);

    if( m_d->unitMeasure != nullptr ){
        connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &AccountingTAMMeasuresModel::updateQuantity );
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
    return m_d->tamBillItem->daysCount() + 2;
}

Qt::ItemFlags AccountingTAMMeasuresModel::flags(const QModelIndex &index) const {
    if( index.isValid() ){
        if( index.column() == AccountingTAMMeasuresModelPrivate::commentCol ||
                (index.column() >= AccountingTAMMeasuresModelPrivate::firstFormulaCol && index.column() >= m_d->lastFormulaCol() ) ){
            return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
        } else if( index.column() == m_d->quantityCol() ){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
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
                if(index.column() >= AccountingTAMMeasuresModelPrivate::firstFormulaCol && index.column() >= m_d->lastFormulaCol() ){
                    return QVariant( m_d->linesContainer.at(index.row())->formula( index.column() - AccountingTAMMeasuresModelPrivate::firstFormulaCol ) );
                }
                if( index.column() == m_d->quantityCol() ){
                    return QVariant( m_d->linesContainer.at(index.row())->quantityStr());
                }
            }
            if( role == Qt::TextAlignmentRole ){
                if( index.column() == AccountingTAMMeasuresModelPrivate::commentCol ){
                    return Qt::AlignLeft + Qt::AlignVCenter;
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
                if(index.column() >= AccountingTAMMeasuresModelPrivate::firstFormulaCol && index.column() >= m_d->lastFormulaCol() ){
                    m_d->linesContainer.at(index.row())->setFormula( index.column() - AccountingTAMMeasuresModelPrivate::firstFormulaCol, value.toString() );
                    emit dataChanged( index, index );
                    QModelIndex changedIndex = createIndex( index.row(), m_d->quantityCol() );
                    emit dataChanged( changedIndex, changedIndex );
                    updateQuantity();
                    emit modelChanged();
                    return true;
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
            return tr("Commento");
        }
        if( section >= AccountingTAMMeasuresModelPrivate::firstFormulaCol && section >= m_d->lastFormulaCol() ) {
            return m_d->tamBillItem->dayStr( section - AccountingTAMMeasuresModelPrivate::firstFormulaCol );
        }
        if( section == m_d->quantityCol() ) {
            return tr("Quantità");
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}

bool AccountingTAMMeasuresModel::insertRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED(parent)
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
        connect( itemLine, &AccountingTAMMeasure::quantityChanged, this, &AccountingTAMMeasuresModel::updateQuantity );
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
    updateQuantity();
    emit modelChanged();
    return true;
}

bool AccountingTAMMeasuresModel::append( int count ){
    return insertRows( m_d->linesContainer.size(), count );
}

double AccountingTAMMeasuresModel::quantity() {
    return m_d->quantity;
}

void AccountingTAMMeasuresModel::updateQuantity() {
    double ret = 0.0;
    for( QList<AccountingTAMMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        ret += (*i)->quantity();
    }
    if( ret != m_d->quantity ){
        m_d->quantity = ret;
        emit quantityChanged( ret );
    }
}

void AccountingTAMMeasuresModel::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->unitMeasure != ump ){
        beginResetModel();
        if( m_d->unitMeasure != nullptr ){
            disconnect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &AccountingTAMMeasuresModel::updateQuantity );
        }
        m_d->unitMeasure = ump;
        for( QList<AccountingTAMMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
            (*i)->setUnitMeasure( ump );
        }
        if( m_d->linesContainer.size() != 0 ){
            emit dataChanged( createIndex(0, 2), createIndex(m_d->linesContainer.size()-1, 2) );
        }
        updateQuantity();
        if( m_d->unitMeasure != nullptr ){
            connect( m_d->unitMeasure, &UnitMeasure::precisionChanged, this, &AccountingTAMMeasuresModel::updateQuantity );
        }
        endResetModel();
        emit modelChanged();
    }
}

void AccountingTAMMeasuresModel::writeXml20(QXmlStreamWriter *writer) {
    writer->writeStartElement( "AccountingTAMMeasuresModel" );
    for( QList<AccountingTAMMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->writeXml( writer );
    }
    writer->writeEndElement();
}

void AccountingTAMMeasuresModel::readXmlTmp20(QXmlStreamReader *reader) {
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

void AccountingTAMMeasuresModel::readFromXmlTmp20() {
    for( QList<AccountingTAMMeasure *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->loadFromXmlTmp20();
    }
}

int AccountingTAMMeasuresModel::measuresCount() {
    return m_d->linesContainer.size();
}

AccountingTAMMeasure * AccountingTAMMeasuresModel::measure(int i) {
    if( i >= 0 && i < m_d->linesContainer.size() ){
        return m_d->linesContainer.at(i);
    }
    return nullptr;
}
