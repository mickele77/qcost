/*
   QCost is a cost estimating software.
   Copyright (C) 2013-2016 Mocciola Michele

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
#include "priceitemdatasetmodel.h"

#include "bill.h"
#include "priceitem.h"
#include "mathparser.h"
#include "pricefieldmodel.h"
#include "unitmeasure.h"

#include <QXmlStreamAttributes>
#include <QTextStream>

class PriceItemDataSetPrivate{
public:
    PriceItemDataSetPrivate( PriceItem * pItem ):
        overheads(0.13),
        profits(0.10) {
        if( pItem->parentItem() == NULL ){
            inheritOverheadsFromRoot = false;
            inheritProfitsFromRoot = false;
        } else {
            inheritOverheadsFromRoot = true;
            inheritProfitsFromRoot = true;
        }
    }
    ~PriceItemDataSetPrivate(){
    }
    // valori dei campi associati
    QList<double> value;
    // spese generali
    double overheads;
    // ci dice se l'istanza ha le stesse spese generali dell'oggetto radice
    bool inheritOverheadsFromRoot;
    // utili di impresa
    double profits;
    // ci dice se l'istanza ha gli stessi utili dell'oggetto radice
    bool inheritProfitsFromRoot;
};

// ciascun PriceItemDataSet contiene i valori che il prezzo
// associa a ciascun campo
class PriceItemDataSet{
public:
    // costruttore
    PriceItemDataSet( PriceItem *pItem, PriceFieldModel * pfm, MathParser * prs ):
        associateAP(false),
        associatedAP( new Bill( QObject::trUtf8("Analisi Prezzi"), NULL, pfm, prs ) ),
        m_d( new PriceItemDataSetPrivate( pItem ) ){
        for( int i=0; i < pfm->rowCount(); ++i ){
            m_d->value.append( 0.0 );
        }
    }

    // distruttore
    ~PriceItemDataSet(){
        delete associatedAP;
        delete m_d;
    }

    // overloading dell'operatore di assegnazione
    PriceItemDataSet & operator= (const PriceItemDataSet & );

    // spese generali
    double overheads(){ return m_d->overheads; }
    // imposta spese generali
    void setOverheads(double newVal);
    // ci dice se l'oggetto eredita le spese generali dall'oggetto radice
    bool inheritOverheadsFromRoot() { return m_d->inheritOverheadsFromRoot; }
    // Imposta l'ereditarieta' delle spese generali dall'oggetto radice
    void setInheritOverheadsFromRoot( bool newVal = true );

    // utili di impresa
    double profits() { return m_d->profits; }
    // imposta utili di impresa
    void setProfits(double newVal);
    // ci dice se l'oggetto eredita gli utili dall'oggetto radice
    bool inheritProfitsFromRoot() { return m_d->inheritProfitsFromRoot; }
    // imposta l'ereditarieta' degli utili dall'oggetto radice
    void setInheritProfitsFromRoot( bool newVal = true );

    // restituisce il numero dei campi prezzo associati
    int valueCount() { return m_d->value.size(); }
    // restituisce il valore del campo prezzo priceField
    double value( int priceField ) const;
    // imposta il valore del campo prezzo priceField
    bool setValue(int priceField, double newVal);

    // esiste un'analisi prezzi associata?
    bool associateAP;
    // puntatore all'analisi prezzi associata
    Bill * associatedAP;

    // aggiunge un campo prezzo
    void removePriceField(int row) { m_d->value.removeAt( row ) ; }
    // rimuove un campo prezzo
    void insertPriceField(int row) { m_d->value.insert( row, 0.0 ); }

    void writeXml10(QXmlStreamWriter *writer, bool isRootItem) const;
    // scrive su flusso XML
    void writeXml20(QXmlStreamWriter * writer , bool isRootItem) const;
    // legge attributi XML
    void loadFromXml(const QXmlStreamAttributes &attrs);

private:
    PriceItemDataSetPrivate * m_d;
};

PriceItemDataSet &PriceItemDataSet::operator=(const PriceItemDataSet & cp) {
    // ci assicuriamo che le due liste abbiano la stessa dimensione
    for( int i=0; i < (cp.m_d->value.size() - m_d->value.size()); ++i ){
        m_d->value.append(0.0);
    }
    for( int i=0; i < (m_d->value.size() - cp.m_d->value.size()); ++i ){
        m_d->value.removeLast();
    }
    // copiamo le liste
    m_d->value = cp.m_d->value;

    associateAP = cp.associateAP;
    *associatedAP = *(cp.associatedAP);

    setInheritOverheadsFromRoot( cp.m_d->inheritOverheadsFromRoot );
    if( m_d->inheritOverheadsFromRoot ){
        setOverheads( cp.m_d->overheads );
    }
    setInheritProfitsFromRoot( cp.m_d->inheritProfitsFromRoot );
    if( m_d->inheritProfitsFromRoot ){
        setProfits( cp.m_d->profits );
    }
    return *this;
}

void PriceItemDataSet::setOverheads( double newVal ) {
    if( !m_d->inheritOverheadsFromRoot ){
        if( newVal != m_d->overheads ){
            m_d->overheads = newVal;
        }
    }
}

void PriceItemDataSet::setInheritOverheadsFromRoot(bool newVal) {
    if( m_d->inheritOverheadsFromRoot != newVal ){
        m_d->inheritOverheadsFromRoot = newVal;
    }
}

void PriceItemDataSet::setProfits(double newVal) {
    if( !m_d->inheritProfitsFromRoot ){
        if( m_d->profits != newVal ){
            m_d->profits = newVal;
        }
    }
}

void PriceItemDataSet::setInheritProfitsFromRoot(bool newVal) {
    if( m_d->inheritProfitsFromRoot != newVal ){
        m_d->inheritProfitsFromRoot = newVal;
    }
}

void PriceItemDataSet::writeXml10(QXmlStreamWriter *writer, bool isRootItem ) const {
    writer->writeStartElement( "PriceItemDataSet" );

    if( !isRootItem ){
        if( associateAP ){
            associatedAP->writeXml10( writer );
        } else {
            for( int i=0; i < m_d->value.size(); ++i ){
                writer->writeAttribute( QString("value%1").arg(i), QString::number( m_d->value.at(i) ) );
            }
        }
    }

    if( !(m_d->inheritOverheadsFromRoot) ){
        writer->writeAttribute( QString("overheads"), QString::number(m_d->overheads) );
    }
    if( !(m_d->inheritProfitsFromRoot) ){
        writer->writeAttribute( QString("profits"), QString::number(m_d->profits) );
    }

    writer->writeEndElement();
}

void PriceItemDataSet::writeXml20(QXmlStreamWriter *writer, bool isRootItem ) const {
    writer->writeStartElement( "PriceItemDataSet" );

    if( !isRootItem ){
        if( associateAP ){
            associatedAP->writeXml20( writer );
        } else {
            for( int i=0; i < m_d->value.size(); ++i ){
                writer->writeAttribute( QString("value%1").arg(i), QString::number( m_d->value.at(i) ) );
            }
        }
    }

    if( !(m_d->inheritOverheadsFromRoot) ){
        writer->writeAttribute( QString("overheads"), QString::number(m_d->overheads) );
    }
    if( !(m_d->inheritProfitsFromRoot) ){
        writer->writeAttribute( QString("profits"), QString::number(m_d->profits) );
    }

    writer->writeEndElement();
}

void PriceItemDataSet::loadFromXml(const QXmlStreamAttributes &attrs ){
    for( int i=0; i<attrs.size(); ++i){
        QString nStr = attrs.at(i).qualifiedName().toString().toUpper();
        QString strVal("VALUE");
        if( nStr.startsWith(strVal) ){
            nStr.remove(0,strVal.size() );
            bool ok = false;
            int n = nStr.toInt( & ok );
            if( ok ){
                for( int j=0; j < (n + 1 - m_d->value.size()); ++j ){
                    m_d->value.append(0.0);
                }
                setValue(n, attrs.at(i).value().toDouble() );
            }
        } else if( nStr == "OVERHEADS" ){
            setInheritOverheadsFromRoot( true );
            setOverheads( attrs.at(i).value().toDouble() );
        } else if( nStr == "PROFITS" ){
            setInheritProfitsFromRoot( true );
            setProfits( attrs.at(i).value().toDouble() );
        }
    }
}

double PriceItemDataSet::value(int priceField) const{
    if( priceField > -1 && priceField < m_d->value.size() ){
        return m_d->value.at(priceField);
    }
    return 0.0;
}

bool PriceItemDataSet::setValue(int priceField, double newVal ){
    if( priceField > -1 && priceField < m_d->value.size() ){
        if( m_d->value.at(priceField) != newVal ){
            m_d->value[priceField] = newVal;
            return true;
        }
    }
    return false;
}

class PriceItemDataSetModelPrivate{
public:
    PriceItemDataSetModelPrivate( PriceItem * item, PriceFieldModel * pfm, MathParser * p ):
        priceFieldModel(pfm),
        parser(p),
        priceItem(item),
        basePriceDataSet(0){
    }
    ~PriceItemDataSetModelPrivate(){
        qDeleteAll( dataSetContainer.begin(), dataSetContainer.end() );
    }

    int rowCount() {
        return priceFieldModel->fieldCount() + 3;
    }

    int lastValueRow(){
        return priceFieldModel->fieldCount() -1;
    }

    int associatedAPRow(){
        return priceFieldModel->fieldCount();
    }

    int overheadsRow(){
        return priceFieldModel->fieldCount() + 1;
    }

    int profitsRow(){
        return priceFieldModel->fieldCount() + 2;
    }

    int firstValueRow(){
        return 0;
    }

    int lastRow(){
        return profitsRow();
    }

    QString	toString(double i, char f = 'g', int prec = 6) const{
        if( parser == NULL ){
            return QString::number( i, f, prec );
        } else {
            return parser->toString( i, f, prec );
        }
    }

    double	toDouble( const QString & str ) const{
        if( parser == NULL ){
            return str.toDouble();
        } else {
            return parser->evaluate( str );
        }
    }

    PriceFieldModel * priceFieldModel;
    MathParser * parser;
    PriceItem * priceItem;
    QList<PriceItemDataSet *>  dataSetContainer;
    int basePriceDataSet;
};

PriceItemDataSetModel::PriceItemDataSetModel(MathParser * prs, PriceFieldModel * pfm,  PriceItem * pItem ):
    QAbstractTableModel( pItem ),
    m_d( new PriceItemDataSetModelPrivate( pItem, pfm, prs )){
    if( pItem->parent() == NULL ){
        insertPriceDataSetPrivate( 0 );
    } else {
        insertPriceDataSetPrivate( 0, pItem->parentItem()->dataModel()->priceDataSetCount() );
    }
    connect( m_d->priceFieldModel, &PriceFieldModel::endInsertPriceField, this, &PriceItemDataSetModel::insertPriceField );
    connect( m_d->priceFieldModel, &PriceFieldModel::endRemovePriceField, this, &PriceItemDataSetModel::removePriceField );
    connect( m_d->priceFieldModel, static_cast<void(PriceFieldModel::*)(int)>(&PriceFieldModel::formulaChanged), this, &PriceItemDataSetModel::updateValueFormula );
    connect( m_d->priceFieldModel, static_cast<void(PriceFieldModel::*)(int, int)>(&PriceFieldModel::precisionChanged), this, &PriceItemDataSetModel::updateValueFormula );
    connect( m_d->priceFieldModel, static_cast<void(PriceFieldModel::*)(int, bool)>(&PriceFieldModel::applyFormulaChanged), this, &PriceItemDataSetModel::updateValueFormula );

    connect( this, &PriceItemDataSetModel::dataChanged, this, &PriceItemDataSetModel::modelChanged );
    connect( this, &PriceItemDataSetModel::rowsInserted, this, &PriceItemDataSetModel::modelChanged );
    connect( this, &PriceItemDataSetModel::rowsRemoved, this, &PriceItemDataSetModel::modelChanged );
    connect( this, &PriceItemDataSetModel::rowsMoved, this, &PriceItemDataSetModel::modelChanged );
    connect( this, &PriceItemDataSetModel::columnsInserted, this, &PriceItemDataSetModel::modelChanged );
    connect( this, &PriceItemDataSetModel::columnsMoved, this, &PriceItemDataSetModel::modelChanged );
}

PriceItemDataSetModel::~PriceItemDataSetModel(){
    delete m_d;
}

PriceItemDataSetModel &PriceItemDataSetModel::operator=(const PriceItemDataSetModel &cp) {
    if( m_d->dataSetContainer.size() > cp.m_d->dataSetContainer.size() ){
        removePriceDataSet( 0, m_d->dataSetContainer.size() - cp.m_d->dataSetContainer.size() );
    } else if( m_d->dataSetContainer.size() < cp.m_d->dataSetContainer.size() ){
        insertPriceDataSet( 0, cp.m_d->dataSetContainer.size() - m_d->dataSetContainer.size() );
    }
    for( int i=0; i < m_d->dataSetContainer.size(); ++i ){
        *(m_d->dataSetContainer.at(i)) = *(cp.m_d->dataSetContainer.at(i));
    }
    return *this;
}

int PriceItemDataSetModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_d->rowCount();
}

int PriceItemDataSetModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return priceDataSetCount();
}

double PriceItemDataSetModel::value(int priceField, int priceDataSet) const {
    if( priceDataSet > -1 && priceDataSet < m_d->dataSetContainer.size() ){
        if( priceField > -1 && priceField < m_d->dataSetContainer.at(priceDataSet)->valueCount() ){
            return m_d->dataSetContainer.at(priceDataSet)->value(priceField);
        }
    }
    return 0.0;
}

QString PriceItemDataSetModel::valueStr( int priceField, int priceDataSet) const {
    int prec = 2;
    if( priceField > -1 && priceField < m_d->priceFieldModel->fieldCount() ){
        prec = m_d->priceFieldModel->precision( priceField );
    }
    return m_d->toString( value(priceField, priceDataSet), 'f', prec );
}

int PriceItemDataSetModel::priceDataSetCount() const {
    return m_d->dataSetContainer.size();
}

int PriceItemDataSetModel::lastValueRow() const {
    return m_d->lastValueRow();
}

int PriceItemDataSetModel::associatedAPRow() const {
    return m_d->associatedAPRow();
}

bool PriceItemDataSetModel::insertPriceDataSet(int column, int count ) {
    if( count > 0 ){
        if(m_d->priceItem){
            if( m_d->priceItem->parentItem() == NULL ){
                insertPriceDataSetPrivate( column, count );
            } else {
                m_d->priceItem->parentItem()->dataModel()->insertPriceDataSet(column, count );
            }
            return true;
        }
    }
    return false;
}

bool PriceItemDataSetModel::appendPriceDataSet(int count) {
    return insertPriceDataSet( m_d->dataSetContainer.size(), count );
}

bool PriceItemDataSetModel::appendPriceDataSet(const QXmlStreamAttributes &attrs) {
    bool ret = insertPriceDataSet( m_d->dataSetContainer.size(), 1 );
    if( ret ){
        m_d->dataSetContainer.last()->loadFromXml( attrs );
    }
    return ret;
}

bool PriceItemDataSetModel::insertPriceDataSetPrivate(int columnInput, int count ) {
    if( count > 0 ){
        int column = columnInput;
        if( column < 0 ){
            column = m_d->dataSetContainer.size();
        }

        emit beginInsertPriceDataSets( column, column + count - 1);
        beginInsertColumns(QModelIndex(), column, column + count - 1);
        for( int i=0; i < count; ++i ){
            PriceItemDataSet * dataSet = new PriceItemDataSet( m_d->priceItem, m_d->priceFieldModel, m_d->parser);
            if( m_d->priceItem->parentItem() == NULL ){
                dataSet->setInheritOverheadsFromRoot( false );
                dataSet->setInheritProfitsFromRoot( false );
            } else {
                dataSet->setInheritOverheadsFromRoot( true );
                dataSet->setInheritProfitsFromRoot( true );
            }
            m_d->dataSetContainer.insert( column, dataSet );
        }
        endInsertColumns();
        emit endInsertPriceDataSets( column, column + count - 1);
        emit priceDataSetCountChanged( m_d->dataSetContainer.size() );

        if( m_d->priceItem ){
            for( int i=0; i < m_d->priceItem->childrenCount(); ++i){
                PriceItem * item = dynamic_cast<PriceItem *>(m_d->priceItem->child(i));
                if( item ){
                    item->dataModel()->insertPriceDataSetPrivate( column, count );
                }
            }
        }
        return true;
    }
    return false;
}

bool PriceItemDataSetModel::removePriceDataSet(int column, int count ) {
    if( count > 0 ){
        if(m_d->priceItem){
            if( m_d->priceItem->parentItem() == NULL ){
                m_d->priceItem->dataModel()->removePriceDataSetPrivate( column, count );
            } else {
                m_d->priceItem->parentItem()->dataModel()->removePriceDataSet(column, count );
            }
            return true;
        }
    }
    return false;
}

bool PriceItemDataSetModel::removePriceDataSetPrivate(int column, int count ) {
    if( column < 0 || column > m_d->dataSetContainer.size() - 1 ){
        return false;
    }

    int realCount = count;

    if( m_d->priceItem ){
        for( int i=0; i < m_d->priceItem->childrenCount(); ++i){
            PriceItem * item = dynamic_cast<PriceItem *>(m_d->priceItem->child(i));
            if( item ){
                item->dataModel()->removePriceDataSetPrivate( column, count );
            }
        }
    }

    if( column == 0 ){
        // lasciamo sempre almeno un prezzo
        if( realCount >= m_d->dataSetContainer.size() ){
            realCount = m_d->dataSetContainer.size() - 1;
        }
    } else if( realCount >  m_d->dataSetContainer.size() - column ){
        // se il numero di prezzi da eliminare è eccessivo, si ricalcola il numero
        realCount = m_d->dataSetContainer.size() - column;
    }

    if( realCount > 0 ){
        emit beginRemovePriceDataSets( column, column + realCount - 1);
        beginRemoveColumns(QModelIndex(), column, column + realCount - 1);
        for( int i=realCount; i > 0; -- i ){
            m_d->dataSetContainer.takeAt( column );
        }
        endRemoveColumns();
        emit endRemovePriceDataSets( column, column + realCount - 1);

        return true;
    } else {
        return false;
    }
}

void PriceItemDataSetModel::insertPriceField( int row ){
    emit beginInsertPriceField( row, row );
    beginInsertRows( QModelIndex(), row, row );
    for( QList<PriceItemDataSet *>::iterator i = m_d->dataSetContainer.begin(); i != m_d->dataSetContainer.end(); ++i){
        (*i)->insertPriceField( row );
    }
    endInsertRows();
    emit endInsertPriceField( row, row );
}

void PriceItemDataSetModel::removePriceField( int row ){
    emit beginRemovePriceField( row, row );
    beginRemoveRows( QModelIndex(), row, row );
    for( QList<PriceItemDataSet *>::iterator i = m_d->dataSetContainer.begin(); i != m_d->dataSetContainer.end(); ++i){
        (*i)->removePriceField( row );
    }
    endRemoveRows();
    emit endRemovePriceField( row, row );
}

Qt::ItemFlags PriceItemDataSetModel::flags(const QModelIndex &index) const {
    if( index.row() == m_d->associatedAPRow() ){
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    } else if( index.row() == m_d->overheadsRow() ){
        if( index.column() < m_d->dataSetContainer.size() ){
            return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        }
    } else if( index.row() == m_d->profitsRow() ){
        if( index.column() < m_d->dataSetContainer.size() ){
            return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        }
    } else if( index.column() < m_d->dataSetContainer.size() ){
        if( m_d->dataSetContainer.at(index.column())->associateAP ){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        } else {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        }
    }
    return QAbstractTableModel::flags( index );
}

QVariant PriceItemDataSetModel::data(const QModelIndex &index, int role) const {
    if( !index.isValid() ){
        return QVariant();
    }

    if( (index.column() > -1 && index.column() < m_d->dataSetContainer.size()) &&
            (index.row() > -1 && index.row() < m_d->rowCount()) ){

        if( role == Qt::TextAlignmentRole ){
            return Qt::AlignRight + Qt::AlignVCenter ;
        }

        if( role == Qt::CheckStateRole ){
            if( index.row() == m_d->associatedAPRow() ){
                if( m_d->dataSetContainer.at(index.column())->associateAP ){
                    return QVariant( Qt::Checked );
                } else {
                    return QVariant( Qt::Unchecked );
                }
            }
            if( index.row() == m_d->overheadsRow() ){
                if( m_d->dataSetContainer.at(index.column())->inheritOverheadsFromRoot() ){
                    return QVariant( Qt::Checked );
                } else {
                    return QVariant( Qt::Unchecked );
                }
            }
            if( index.row() == m_d->profitsRow() ){
                if( m_d->dataSetContainer.at(index.column())->inheritProfitsFromRoot() ){
                    return QVariant( Qt::Checked );
                } else {
                    return QVariant( Qt::Unchecked );
                }
            }
        }

        if( role == Qt::DisplayRole || role == Qt::EditRole ){
            if( index.row() == m_d->associatedAPRow() ){
                return QVariant();
            }
            if( index.row() == m_d->overheadsRow() ){
                return QVariant( overheadsStr(index.column()) );
            }
            if( index.row() == m_d->profitsRow() ){
                return QVariant( profitsStr(index.column()) );
            }

            int pField = index.row();
            int pDataSet = index.column();

            if( (pField > -1) && (pField < m_d->dataSetContainer.at(pDataSet)->valueCount()) ){
                return QVariant( m_d->toString( m_d->dataSetContainer.at(pDataSet)->value(pField), 'f', m_d->priceFieldModel->precision(pField) ) );
            }
        }
    }

    return QVariant();
}

bool PriceItemDataSetModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if( ! index.isValid() ){
        return false;
    }

    if( (index.column() >= 0 && index.column() < m_d->dataSetContainer.size()) &&
            (index.row() >= 0 && index.row() < m_d->rowCount()) ){

        if( role == Qt::CheckStateRole ){
            if( index.row() == m_d->associatedAPRow() ){
                setAssociateAP( index.column(), (value.toInt() == Qt::Checked) );
                QModelIndex topLeft = createIndex( m_d->firstValueRow(), index.column() );
                QModelIndex bottomRight = createIndex( m_d->lastValueRow(), index.column() );
                emit dataChanged( topLeft, bottomRight );
                emit dataChanged( index, index );
                return true;
            }
            if( index.row() == m_d->overheadsRow() ){
                setInheritOverheadsFromRoot( index.column(), (value.toInt() == Qt::Checked) );
                emit dataChanged( index, index );
                return true;
            }
            if( index.row() == m_d->profitsRow() ){
                setInheritProfitsFromRoot( index.column(), (value.toInt() == Qt::Checked) );
                emit dataChanged( index, index );
                return true;
            }
        }

        if( role == Qt::EditRole  ){
            if( index.row() == m_d->associatedAPRow() ){
                return false;
            }
            if( index.row() == m_d->overheadsRow() ){
                setOverheads( index.column(), value.toString() );
                QModelIndex topLeft = createIndex( m_d->firstValueRow(), index.column() );
                QModelIndex bottomRight = createIndex( m_d->lastValueRow(), index.column() );
                emit dataChanged( topLeft, bottomRight );
                emit dataChanged( index, index );
                return true;
            }
            if( index.row() == m_d->profitsRow() ){
                setProfits( index.column(), value.toString() );
                QModelIndex topLeft = createIndex( m_d->firstValueRow(), index.column() );
                QModelIndex bottomRight = createIndex( m_d->lastValueRow(), index.column() );
                emit dataChanged( topLeft, bottomRight );
                emit dataChanged( index, index );
                return true;
            }

            int pf = index.row();

            if( (pf < m_d->dataSetContainer.at(index.column())->valueCount()) && (pf >= 0) ){
                double v = 0.0;
                if( m_d->parser ){
                    v = m_d->parser->evaluate( value.toString() );
                } else {
                    v = value.toDouble();
                }
                setValue( pf, index.column(), v );
                return true;
            }
        }
    }
    return false;
}

QVariant PriceItemDataSetModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if( role == Qt::DisplayRole ){
        if (orientation == Qt::Horizontal ){
            return QVariant( section + 1 );
        } else {
            if( section == m_d->associatedAPRow() ){
                return QVariant( trUtf8("Analisi"));
            } else if( section == m_d->overheadsRow() ){
                return QVariant( trUtf8("Spese Generali [%]"));
            } else if( section == m_d->profitsRow() ){
                return QVariant( trUtf8("Utili [%]"));
            } else {
                int pf = (section-m_d->firstValueRow()) / 2;
                if( (pf < m_d->priceFieldModel->rowCount()) && (pf >= 0)){
                    return QVariant( QString("%1 [%2]").arg(m_d->priceFieldModel->priceName(pf), m_d->priceFieldModel->unitMeasure(pf)) );
                }
            }
        }
    }
    return QVariant();
}

int PriceItemDataSetModel::associatedAPRow() {
    return m_d->associatedAPRow();
}

bool PriceItemDataSetModel::setValue(int priceField, int priceDataSet, double newValInput ) {
    if( (priceDataSet > -1) && (priceDataSet < m_d->dataSetContainer.size()) ){
        if( (priceField > -1) && (priceField < m_d->priceFieldModel->fieldCount()) ){
            double newVal = UnitMeasure::applyPrecision( newValInput, m_d->priceFieldModel->precision(priceField) );
            if( m_d->dataSetContainer.at(priceDataSet)->value( priceField ) != newVal ){
                m_d->dataSetContainer.at(priceDataSet)->setValue( priceField, newVal );
                emit valueChanged(priceField, priceDataSet, newVal);
                QModelIndex topLeft = createIndex( 2 * priceField, priceDataSet );
                QModelIndex bottomRight = createIndex( 2 * priceField + 1, priceDataSet );
                emit dataChanged(topLeft, bottomRight );
                if( priceDataSet == m_d->basePriceDataSet ){
                    topLeft = createIndex( 2 * priceField + 1, 0 );
                    bottomRight = createIndex( 2 * priceField + 1, m_d->dataSetContainer.size() );
                }
                emit dataChanged(topLeft, bottomRight );
                m_d->priceItem->emitValueChanged( priceField, priceDataSet, newVal );

                // copia in filedValues i valori dei campi
                QList<double> fieldValues;
                for( int i=0; i < m_d->priceFieldModel->fieldCount(); i++ ){
                    fieldValues.append( m_d->dataSetContainer.at(priceDataSet)->value(i) );
                }

                for( int i=0; i < m_d->priceFieldModel->fieldCount(); i++ ){
                    if( i != priceField && m_d->priceFieldModel->applyFormula(i) ){
                        bool ok = false;
                        double v = m_d->priceFieldModel->calcFormula( &ok, i, fieldValues, m_d->priceItem->overheads(priceDataSet), m_d->priceItem->profits(priceDataSet) );
                        if( ok ){
                            setValue( i, priceDataSet, v );
                            fieldValues[i] = v;
                        }
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool PriceItemDataSetModel::setValue(int priceField, int priceDataSet, const QString & newVal) {
    return setValue( priceField, priceDataSet, m_d->toDouble( newVal ) );
}

void PriceItemDataSetModel::updateValueFormula( int priceField ) {
    if( m_d->priceFieldModel->applyFormula(priceField) ){
        for( int priceDataSet = 0; priceDataSet < m_d->dataSetContainer.size(); priceDataSet++) {
            QList<double> fieldValues;
            for( int pf=0; pf < m_d->dataSetContainer.at(priceDataSet)->valueCount(); ++pf ){
                fieldValues << m_d->dataSetContainer.at(priceDataSet)->value(pf);
            }
            bool ok = false;
            double newVal = m_d->priceFieldModel->calcFormula( &ok, priceField, fieldValues, m_d->priceItem->overheads(priceDataSet), m_d->priceItem->profits(priceDataSet) );
            if( (m_d->dataSetContainer.at(priceDataSet)->value( priceField ) != newVal) && ok ){
                setValue( priceField, priceDataSet, newVal );
            }
        }
    }
}

bool PriceItemDataSetModel::associateAP(int priceDataSet) {
    if( priceDataSet > -1 && priceDataSet < m_d->dataSetContainer.size() ){
        return m_d->dataSetContainer.at(priceDataSet)->associateAP;
    }
    return false;
}

void PriceItemDataSetModel::setAssociateAP(int priceDataSet, bool newVal ) {
    if( priceDataSet >= 0 && priceDataSet < m_d->dataSetContainer.size() ){
        if( m_d->dataSetContainer.at(priceDataSet)->associateAP != newVal ){
            m_d->dataSetContainer.at(priceDataSet)->associateAP = newVal;
            if( newVal ){
                for( int field=0; field < m_d->priceFieldModel->fieldCount(); ++field ){
                    setValue( field, priceDataSet,  m_d->dataSetContainer.at(priceDataSet)->associatedAP->amount(field) );
                }
                connect( m_d->dataSetContainer.at(priceDataSet)->associatedAP, static_cast<void(Bill::*)(int,double)> (&Bill::amountChanged), this, &PriceItemDataSetModel::setValueFromAP );
            } else {
                disconnect( m_d->dataSetContainer.at(priceDataSet)->associatedAP, static_cast<void(Bill::*)(int,double)> (&Bill::amountChanged), this, &PriceItemDataSetModel::setValueFromAP );
            }
            QModelIndex topLeft = createIndex(0, priceDataSet );
            QModelIndex bottomRight = createIndex( m_d->associatedAPRow(), priceDataSet );
            emit dataChanged(topLeft,bottomRight);
            m_d->priceItem->emitAssociateAPChanged( priceDataSet, newVal );
        }
    }
}

Bill * PriceItemDataSetModel::associatedAP(int priceDataSet) {
    if( priceDataSet > -1 && priceDataSet < m_d->dataSetContainer.size() ){
        return m_d->dataSetContainer.at(priceDataSet)->associatedAP;
    }
    return NULL;
}

double PriceItemDataSetModel::overheads(int priceDataSet) const {
    if( (priceDataSet > -1) && (priceDataSet < m_d->dataSetContainer.size()) ){
        if( m_d->dataSetContainer.at( priceDataSet )->inheritOverheadsFromRoot() ){
            return overheadsFromRoot( priceDataSet, m_d->priceItem );
        } else {
            return m_d->dataSetContainer.at(priceDataSet)->overheads();
        }
    }
    return 0.0;
}

double PriceItemDataSetModel::overheadsFromRoot(int priceDataSet, PriceItem * pItem ) const {
    if( pItem->parentItem() == NULL ){
        return pItem->dataModel()->m_d->dataSetContainer.at( priceDataSet )->overheads();
    } else {
        return overheadsFromRoot( priceDataSet, pItem->parentItem() );
    }
}

QString PriceItemDataSetModel::overheadsStr(int priceDataSet) const {
    return QString("%1 %").arg( m_d->toString( overheads(priceDataSet) * 100.0, 'f', 4) );
}

void PriceItemDataSetModel::setOverheads(int priceDataSet, double newVal) {
    if( priceDataSet > -1 && priceDataSet < m_d->dataSetContainer.size() ){
        if( m_d->dataSetContainer.at(priceDataSet)->inheritOverheadsFromRoot() ){
            setOverheadsToRoot( priceDataSet, newVal, m_d->priceItem );
        } else {
            m_d->dataSetContainer.at(priceDataSet)->setOverheads( newVal );
            emit overheadsChanged( priceDataSet, overheadsStr( priceDataSet ) );
        }
    }
}

void PriceItemDataSetModel::setOverheadsToRoot(int priceDataSet, double newVal, PriceItem * pItem ) {
    if( pItem->parentItem() == NULL ){
        pItem->dataModel()->m_d->dataSetContainer.at( priceDataSet )->setOverheads(newVal );
        setOverheadsFromRoot( priceDataSet, newVal );
    } else {
        setOverheadsToRoot( priceDataSet, newVal, pItem->parentItem() );
    }
}

void PriceItemDataSetModel::setOverheadsFromRoot(int priceDataSet, double newVal ) {
    PriceItemDataSet * data = m_d->dataSetContainer.at(priceDataSet);
    if( (data->overheads() != newVal) && (data->inheritOverheadsFromRoot()) ){
        data->setOverheads( newVal );
        emit overheadsChanged( priceDataSet, overheadsStr( priceDataSet ) );
    }

    for( int i=0; i < m_d->priceItem->childrenCount(); ++i ){
        m_d->priceItem->childItem(i)->dataModel()->setOverheadsFromRoot( priceDataSet, newVal );
    }
}

void PriceItemDataSetModel::setOverheads(int priceDataSet, const QString &newVal) {
    QString newValEff = newVal;
    newValEff.remove("%");
    setOverheads( priceDataSet, m_d->toDouble(newValEff) / 100.0 );
}

bool PriceItemDataSetModel::inheritOverheadsFromRoot(int priceDataSet) {
    if( priceDataSet > -1 && priceDataSet < m_d->dataSetContainer.size() ){
        return m_d->dataSetContainer.at(priceDataSet)->inheritOverheadsFromRoot();
    }
    return false;
}

void PriceItemDataSetModel::setInheritOverheadsFromRoot(int priceDataSet, bool newVal) {
    if( priceDataSet > -1 && priceDataSet < m_d->dataSetContainer.size() ){
        if( m_d->dataSetContainer.at(priceDataSet)->inheritOverheadsFromRoot() != newVal ){
            m_d->dataSetContainer.at(priceDataSet)->setInheritOverheadsFromRoot( newVal );
            emit inheritOverheadsFromRootChanged( priceDataSet, newVal );
            emit overheadsChanged( priceDataSet, overheadsStr(priceDataSet) );
        }
    }
}

double PriceItemDataSetModel::profits(int priceDataSet) const {
    if( priceDataSet >= 0 && priceDataSet < m_d->dataSetContainer.size() ){
        if( m_d->dataSetContainer.at(priceDataSet)->inheritProfitsFromRoot() ){
            return profitsFromRoot( priceDataSet, m_d->priceItem );
        } else {
            return m_d->dataSetContainer.at( priceDataSet )->profits();
        }
    }
    return 0.0;
}

QString PriceItemDataSetModel::profitsStr(int priceDataSet) const {
    return QString("%1 %").arg( m_d->toString( profits(priceDataSet) * 100.0, 'f', 4) );
}

double PriceItemDataSetModel::profitsFromRoot( int priceDataSet, PriceItem *pItem ) const {
    if( pItem->parentItem() == NULL ){
        return pItem->dataModel()->m_d->dataSetContainer.at( priceDataSet )->profits();
    } else {
        return profitsFromRoot( priceDataSet, pItem->parentItem() );
    }
}

void PriceItemDataSetModel::setProfits(int priceDataSet, double newVal) {
    if( priceDataSet > -1 && priceDataSet < m_d->dataSetContainer.size() ){
        if( m_d->dataSetContainer.at(priceDataSet)->inheritProfitsFromRoot() ){
            setProfitsToRoot( priceDataSet, newVal, m_d->priceItem );
        } else {
            m_d->dataSetContainer.at(priceDataSet)->setProfits( newVal );
            emit profitsChanged( priceDataSet, profitsStr( priceDataSet ));
        }
    }
}

void PriceItemDataSetModel::setProfits(int priceDataSet, const QString &newVal) {
    QString newValEff = newVal;
    newValEff.remove("%");
    setProfits( priceDataSet, m_d->toDouble(newValEff) / 100.0 );
}

void PriceItemDataSetModel::setProfitsToRoot(int priceDataSet, double newVal, PriceItem *pItem) {
    if( pItem->parentItem() == NULL ){
        pItem->dataModel()->m_d->dataSetContainer.at( priceDataSet )->setProfits(newVal );
        setProfitsFromRoot( priceDataSet, newVal );
    } else {
        setProfitsToRoot( priceDataSet, newVal, pItem->parentItem() );
    }
}

void PriceItemDataSetModel::setProfitsFromRoot(int priceDataSet, double newVal ) {
    PriceItemDataSet * data = m_d->dataSetContainer.at(priceDataSet);
    if( (data->profits() != newVal) && (data->inheritProfitsFromRoot()) ){
        data->setProfits( newVal );
        emit profitsChanged( priceDataSet, profitsStr( priceDataSet ) );
    }

    for( int i=0; i < m_d->priceItem->childrenCount(); ++i ){
        m_d->priceItem->childItem(i)->dataModel()->setProfitsFromRoot( priceDataSet, newVal );
    }
}

bool PriceItemDataSetModel::inheritProfitsFromRoot(int priceDataSet) {
    if( priceDataSet > -1 && priceDataSet < m_d->dataSetContainer.size() ){
        return m_d->dataSetContainer.at(priceDataSet)->inheritProfitsFromRoot();
    }
    return false;
}

void PriceItemDataSetModel::setInheritProfitsFromRoot(int priceDataSet, bool newVal) {
    if( priceDataSet > -1 && priceDataSet < m_d->dataSetContainer.size() ){
        if( m_d->dataSetContainer.at(priceDataSet)->inheritProfitsFromRoot() != newVal ){
            m_d->dataSetContainer.at(priceDataSet)->setInheritProfitsFromRoot( newVal );
            emit inheritProfitsFromRootChanged( priceDataSet, newVal );
            emit profitsChanged( priceDataSet, profitsStr(priceDataSet) );
        }
    }
}

void PriceItemDataSetModel::setValueFromAP(int priceField, double v){
    Bill * bill = dynamic_cast<Bill *>(sender());
    if( bill ){
        for( int i = 0; i < m_d->dataSetContainer.size(); ++i){
            if( m_d->dataSetContainer.at(i)->associatedAP == bill ){
                setValue( priceField, i, v );
                return;
            }
        }
    }
}

void PriceItemDataSetModel::writeXml10(QXmlStreamWriter *writer) const{
    for( QList<PriceItemDataSet *>::iterator i = m_d->dataSetContainer.begin(); i != m_d->dataSetContainer.end(); ++i){
        if( m_d->priceItem->parentItem() == NULL ){
            (*i)->writeXml10( writer, true );
        } else {
            (*i)->writeXml10( writer, false );
        }
    }
}

void PriceItemDataSetModel::writeXml20(QXmlStreamWriter *writer) const {
    for( QList<PriceItemDataSet *>::iterator i = m_d->dataSetContainer.begin(); i != m_d->dataSetContainer.end(); ++i){
        if( m_d->priceItem->parentItem() == NULL ){
            (*i)->writeXml20( writer, true );
        } else {
            (*i)->writeXml20( writer, false );
        }
    }
}

void PriceItemDataSetModel::loadXmlPriceDataSet( int priceDataSet, const QXmlStreamAttributes &attrs) {
    if( (priceDataSet > -1) && (priceDataSet < m_d->dataSetContainer.size()) ){
        m_d->dataSetContainer.at(priceDataSet)->loadFromXml( attrs );
    }
}

void PriceItemDataSetModel::readFromXmlTmp( ProjectPriceListParentItem *priceLists ) {
    for( QList<PriceItemDataSet *>::iterator i = m_d->dataSetContainer.begin(); i != m_d->dataSetContainer.end(); ++i){
        if( (*i)->associateAP ){
            (*i)->associatedAP->readFromXmlTmp( priceLists );
        }
    }
}
