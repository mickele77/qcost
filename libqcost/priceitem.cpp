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
#include "priceitem.h"

#include "bill.h"
#include "pricefieldmodel.h"
#include "priceitemdatasetmodel.h"
#include "unitmeasuremodel.h"
#include "unitmeasure.h"
#include "mathparser.h"

#include <QXmlStreamAttributes>
#include <QXmlStreamReader>
#include <QTextCursor>
#include <QTextTable>
#include <QTextStream>
#include <QVariant>
#include <QList>
#include <QObject>

#include <cmath>

class PriceItemPrivate{
public:
    PriceItemPrivate( PriceItem * parent, PriceFieldModel * pfm, MathParser * prs ):
        parentItem(parent),
        priceFieldModel(pfm),
        parser(prs),
        inheritCodeFromParent(true),
        shortDescription( QObject::trUtf8( "Denominazione" ) ),
        inheritShortDescFromParent(false),
        longDescription( "" ),
        inheritLongDescFromParent( false ),
        unitMeasure(NULL),
        codeCol(0),
        sDescCol(1),
        umCol(2),
        firstValueCol(3),
        nextCode(1){
        if( parent != NULL ){
            id = 1;
        } else {
            id = 0;
        }
    }
    PriceItemPrivate(){
        qDeleteAll( childrenContainer.begin(), childrenContainer.end());
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

    static QString boolToQString(bool v){
        if( v ){
            return QString("true");
        } else {
            return QString("false");
        }
    }

    static bool QStringToBool(const QString & str ) {
        if( str.toUpper() == "TRUE" ){
            return true;
        } else {
            return false;
        }
    }

    // Funzione di utilita per semplificare il codice
    static void insertEmptyRow( int cellCount, QTextCursor *cursor, const QTextTableCellFormat &leftFormat, const QTextTableCellFormat &centralFormat, const QTextTableCellFormat &rightFormat ) {
        QTextTable *table = cursor->currentTable();
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        cursor->movePosition(QTextCursor::NextCell);
        table->cellAt( *cursor ).setFormat( leftFormat );
        for( int i=0; i<(cellCount-2); ++i ){
            cursor->movePosition(QTextCursor::NextCell);
            table->cellAt( *cursor ).setFormat( centralFormat );
        }
        cursor->movePosition(QTextCursor::NextCell);
        table->cellAt( *cursor ).setFormat( rightFormat );
    }

    // Funzione di utilita per semplificare il codice
    static void writeCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &cellFormat, const QTextBlockFormat &blockFormat, const QString & txt = QString(), bool nextCell = true ) {
        if( nextCell ){
            cursor->movePosition(QTextCursor::NextCell);
        }
        table->cellAt( *cursor ).setFormat( cellFormat );
        cursor->setBlockFormat( blockFormat );
        if( !txt.isEmpty() ){
            cursor->insertText( txt );
        }
    }

    // Funzione di utilita per semplificare il codice
    static void writeCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &cellFormat, const QTextBlockFormat &blockFormat, QList< QPair<QString, QTextCharFormat> > txt, bool nextCell = true ) {
        if( nextCell ){
            cursor->movePosition(QTextCursor::NextCell);
        }
        table->cellAt( *cursor ).setFormat( cellFormat );
        cursor->setBlockFormat( blockFormat );
        for( QList< QPair<QString, QTextCharFormat> >::iterator i = txt.begin(); i != txt.end(); ++i ){
            cursor->insertText( (*i).first, (*i).second );
        }
    }

    PriceItem * parentItem;
    QList<PriceItem *> childrenContainer;
    PriceItemDataSetModel * dataModel;
    PriceFieldModel * priceFieldModel;
    MathParser * parser;

    QString code;
    bool inheritCodeFromParent;
    QString shortDescription;
    bool inheritShortDescFromParent;
    QString longDescription;
    bool inheritLongDescFromParent;
    UnitMeasure * unitMeasure;

    int codeCol;
    int sDescCol;
    int umCol;
    int firstValueCol;

    unsigned int id;
    int nextCode;
};

PriceItem::PriceItem(PriceItem *parentItem, PriceFieldModel * pfm, MathParser *prs):
    m_d( new PriceItemPrivate(parentItem, pfm, prs ) ){

    m_d->dataModel = new PriceItemDataSetModel(prs, pfm, this);

    connect( m_d->dataModel, &PriceItemDataSetModel::overheadsChanged, this, &PriceItem::overheadsChanged );
    connect( m_d->dataModel, &PriceItemDataSetModel::inheritOverheadsFromRootChanged, this, &PriceItem::inheritOverheadsFromRootChanged );
    connect( m_d->dataModel, &PriceItemDataSetModel::profitsChanged, this, &PriceItem::profitsChanged );
    connect( m_d->dataModel, &PriceItemDataSetModel::inheritProfitsFromRootChanged, this, &PriceItem::inheritProfitsFromRootChanged );

    connect( m_d->dataModel, &PriceItemDataSetModel::priceDataSetCountChanged, this, &PriceItem::priceDataSetCountChanged );
    connect( m_d->dataModel, &PriceItemDataSetModel::beginInsertPriceDataSets, this, &PriceItem::beginInsertPriceDataSets );
    connect( m_d->dataModel, &PriceItemDataSetModel::endInsertPriceDataSets, this, &PriceItem::endInsertPriceDataSets );
    connect( m_d->dataModel, &PriceItemDataSetModel::beginRemovePriceDataSets, this, &PriceItem::beginRemovePriceDataSets );
    connect( m_d->dataModel, &PriceItemDataSetModel::endRemovePriceDataSets, this, &PriceItem::endRemovePriceDataSets );

    if( parentItem != NULL ){
        m_d->code = giveMeUniqueCode();
    }

    connect( m_d->dataModel, &PriceItemDataSetModel::modelChanged, this, &PriceItem::itemChanged );
    connect( this, &PriceItem::codeFullChanged, this, &PriceItem::itemChanged );
    connect( this, &PriceItem::inheritCodeFromParentChanged, this, &PriceItem::itemChanged );
    connect( this, &PriceItem::shortDescriptionFullChanged, this, &PriceItem::itemChanged );
    connect( this, &PriceItem::inheritShortDescFromParentChanged, this, &PriceItem::itemChanged );
    connect( this, &PriceItem::longDescriptionFullChanged, this, &PriceItem::itemChanged );
    connect( this, &PriceItem::inheritLongDescFromParentChanged, this, &PriceItem::itemChanged );
    connect( this, &PriceItem::unitMeasureChanged, this, &PriceItem::itemChanged );
}

PriceItem::~PriceItem() {
    for( QList<PriceItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
        delete *i;
    }
    m_d->childrenContainer.clear();

    emit aboutToBeDeleted();

    delete m_d->dataModel;
    delete m_d;
}

QString PriceItem::giveMeUniqueCode(){
    if( m_d->parentItem ){
        return m_d->parentItem->giveMeUniqueCode();
    } else {
        QString ret = nextCode();
        while( priceItemFullCode(ret)){
            ret = nextCode();
        }
        return ret;
    }
}

void PriceItem::setId( unsigned int ii ) {
    m_d->id = ii;
}

QString PriceItem::nextCode(){
    int numLength = 3;

    int num = m_d->nextCode % int(pow(10, numLength));

    QString numStr = QString::number(num);
    if( numStr.length() < numLength ){
        numStr.prepend( QString(numLength-numStr.length(), '0') );
    }

    QString letStr;
    int division = int( floor( m_d->nextCode / pow(10, numLength) ) );
    int base = QChar('B').unicode() - QChar('A').unicode() + 1;

    int a = division % base;
    division = division / base;

    letStr.prepend( QChar( QChar('A').unicode() + ushort(a) ) );

    while( division > 0 ){
        a = division % base;
        division = division / base;
        if( a == 0 ){
            a = base;
            division--;
        }
        letStr.prepend( QChar( QChar('A').unicode() + ushort(a-1) ) );
    }
    m_d->nextCode++;
    return letStr + numStr;
}

PriceItem &PriceItem::operator=(const PriceItem &cp) {
    if( & cp != this ){
        if( cp.hasChildren() ){
            removeChildren( 0, childrenCount() );
            insertChildren(0, cp.childrenCount() );
            for( int i=0; i < childrenCount(); ++i ){
                *(m_d->childrenContainer.at(i)) = *(cp.m_d->childrenContainer.at(i));
            }
        }

        setCode( cp.m_d->code );
        setInheritCodeFromParent( cp.m_d->inheritCodeFromParent );
        setShortDescription( cp.m_d->shortDescription );
        setInheritShortDescFromParent( cp.m_d->inheritShortDescFromParent );
        setLongDescription( cp.m_d->longDescription );
        setInheritLongDescFromParent( cp.m_d->inheritLongDescFromParent );
        setUnitMeasure( cp.m_d->unitMeasure );

        *(m_d->dataModel) = *(cp.m_d->dataModel);
    }

    return *this;
}

TreeItem *PriceItem::parentInternal() {
    return m_d->parentItem;
}

PriceItem *PriceItem::parentItem() {
    return m_d->parentItem;
}

bool PriceItem::isDescending(PriceItem *ancestor) {
    if( m_d->parentItem == NULL ){
        return (m_d->parentItem == ancestor);
    } else {
        if( m_d->parentItem == ancestor ){
            return true;
        } else {
            return m_d->parentItem->isDescending( ancestor );
        }
    }
}

double PriceItem::overheads( int priceDataSet ) {
    return m_d->dataModel->overheads( priceDataSet );
}

QString PriceItem::overheadsStr( int priceDataSet ) {
    return m_d->dataModel->overheadsStr( priceDataSet );
}

void PriceItem::setOverheads(int priceDataSet, double newVal) {
    m_d->dataModel->setOverheads( priceDataSet, newVal );
}

void PriceItem::setOverheads( int priceDataSet, const QString &newVal) {
    m_d->dataModel->setOverheads( priceDataSet, newVal );
}

bool PriceItem::inheritOverheadsFromRoot( int priceDataSet ) {
    return m_d->dataModel->inheritOverheadsFromRoot( priceDataSet );
}

void PriceItem::setInheritOverheadsFromRoot(int priceDataSet, bool newVal ) {
    m_d->dataModel->setInheritOverheadsFromRoot( priceDataSet, newVal );
}

double PriceItem::profits( int priceDataSet ) {
    return m_d->dataModel->profits( priceDataSet );
}

QString PriceItem::profitsStr( int priceDataSet ) {
    return m_d->dataModel->profitsStr( priceDataSet );
}

void PriceItem::setProfits(int priceDataSet, double newVal) {
    m_d->dataModel->setProfits( priceDataSet, newVal );
}

void PriceItem::setProfits(int priceDataSet, const QString &newVal) {
    m_d->dataModel->setProfits( priceDataSet, newVal );
}

bool PriceItem::inheritProfitsFromRoot(int priceDataSet) {
    return m_d->dataModel->inheritProfitsFromRoot( priceDataSet );
}

void PriceItem::setInheritProfitsFromRoot(int priceDataSet, bool newVal) {
    m_d->dataModel->setInheritProfitsFromRoot( priceDataSet, newVal );
}

void PriceItem::setParentItem(PriceItem * newParent, int position ) {
    if( position < 0 || position > newParent->childrenCount() ){
        position = newParent->childrenCount();
    }
    if( m_d->parentItem != newParent ){
        if( m_d->parentItem != NULL ){
            m_d->parentItem->removeChild( childNumber() );
        }
        m_d->parentItem = newParent;
        if( newParent != NULL ){
            newParent->addChild( this, position);
        }
    } else {
        int oldPosition = childNumber();
        if( oldPosition != position ){
            if( oldPosition > position ){
                oldPosition++;
            }
            m_d->parentItem->addChild( this, position );
            m_d->parentItem->removeChild( oldPosition );
        }
    }
}

unsigned int PriceItem::id() {
    return m_d->id;
}

PriceFieldModel *PriceItem::priceFieldModel() {
    return m_d->priceFieldModel;
}

PriceItemDataSetModel *PriceItem::dataModel() {
    return m_d->dataModel;
}

int PriceItem::priceDataSetCount() {
    return m_d->dataModel->priceDataSetCount();
}

int PriceItem::columnCount() const {
    // codice + descrizione breve + unita' di misura + campi
    return m_d->firstValueCol + \
            m_d->priceFieldModel->fieldCount()*m_d->dataModel->priceDataSetCount();
}

TreeItem *PriceItem::child(int number) {
    if( number >= 0 && number < m_d->childrenContainer.size() ){
        return m_d->childrenContainer[number];
    }
    return NULL;
}

PriceItem * PriceItem::childItem(int number){
    if( number >= 0 && number < m_d->childrenContainer.size() ){
        return m_d->childrenContainer[number];
    }
    return NULL;
}

PriceItem *PriceItem::lastChild() {
    if( m_d->childrenContainer.size() > 0 ){
        return m_d->childrenContainer.last();
    }
    return NULL;
}

UnitMeasure *PriceItem::unitMeasure() {
    return m_d->unitMeasure;
}

void PriceItem::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->unitMeasure != ump ){
        m_d->unitMeasure = ump;
        emit unitMeasureChanged( ump );
        emit dataChanged( this, m_d->umCol );
    }
}

bool PriceItem::isUsingUnitMeasure(UnitMeasure * ump ) {
    if( hasChildren() ){
        for( QList<PriceItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            if( (*i)->isUsingUnitMeasure(ump) ){
                return true;
            }
        }
        if( m_d->unitMeasure == ump ){
            m_d->unitMeasure = NULL;
        }
    } else {
        if( m_d->unitMeasure == ump ){
            return true;
        }
    }
    return false;
}

int PriceItem::firstValueCol() {
    return m_d->firstValueCol;
}

double PriceItem::value( int priceField, int priceDataSet ) const{
    return m_d->dataModel->value( priceField, priceDataSet );
}

QString PriceItem::valueStr(int priceField, int priceDataSet ) const {
    return m_d->dataModel->valueStr( priceField, priceDataSet );
}

bool PriceItem::setValue( int priceField, int priceDataSet, double newValue ){
    return m_d->dataModel->setValue( priceField, priceDataSet, newValue );
}

bool PriceItem::setValue(int priceField, int priceDataSet, const QString &newValue) {
    return m_d->dataModel->setValue( priceField, priceDataSet, newValue );
}

void PriceItem::emitValueChanged(int priceField, int priceDataSet, double newValue ) {
    emit valueChanged( priceField, priceDataSet, m_d->toString( newValue, 'f', m_d->priceFieldModel->precision(priceField) ) );
    if( priceField == 0 ){
        emit dataChanged( this, priceDataSet+m_d->firstValueCol );
    }
}

void PriceItem::setValue(PriceFieldModel::FieldType fType, int priceDataSet, double newValue) {
    for( int i=0; i < m_d->priceFieldModel->fieldCount(); ++i ){
        if( m_d->priceFieldModel->fieldType(i) == fType ){
            setValue( i, priceDataSet, newValue );
        }
    }
}

bool PriceItem::associateAP(int priceDataSet ) {
    return m_d->dataModel->associateAP(priceDataSet);
}

void PriceItem::setAssociateAP(int priceDataSet, bool newValue) {
    m_d->dataModel->setAssociateAP( priceDataSet, newValue );
}

void PriceItem::emitAssociateAPChanged(int col, bool newVal) {
    emit associateAPChanged(col, newVal );
}

Bill *PriceItem::associatedAP(int priceDataSet) {
    return m_d->dataModel->associatedAP( priceDataSet );
}

QList<PriceItem *> PriceItem::connectedPriceItems() {
    QList<PriceItem *> ret;

    ret.append( this );

    if( hasChildren() ){
        for( QList<PriceItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            ret.append( (*i)->connectedPriceItems() );
        }
    } else {
        for( int i=0; i < m_d->dataModel->priceDataSetCount(); ++i){
            if( m_d->dataModel->associateAP(i) ){
                ret.append( m_d->dataModel->associatedAP(i)->connectedPriceItems() );
            }
        }
    }
    return ret;
}

int PriceItem::childrenCount() const {
    return m_d->childrenContainer.size();
}

bool PriceItem::hasChildren() const {
    return (m_d->childrenContainer.size() > 0);
}

bool PriceItem::insertChildren(int position, int count){
    if (position < 0 || position > m_d->childrenContainer.size())
        return false;

    bool hadChildren = m_d->childrenContainer.size() > 0;

    for (int row = 0; row < count; ++row) {
        PriceItem *item = new PriceItem( this, m_d->priceFieldModel, m_d->parser );
        while( priceItemId( item->id() ) != NULL ){
            item->setId( item->id() + 1 );
        }
        m_d->childrenContainer.insert(position+row, item);
        int Dn = m_d->dataModel->columnCount() > item->dataModel()->columnCount();
        if( Dn > 0 ){
            item->dataModel()->insertPriceDataSetPrivate( -1, Dn );
        }
        connect( item, &PriceItem::dataChanged, this, &PriceItem::dataChanged );
        connect( item, &PriceItem::itemChanged, this, &PriceItem::itemChanged );
    }

    if( m_d->parentItem ){
        bool hasChildren = m_d->childrenContainer.size() > 0;
        if( hadChildren ^ hasChildren ){
            emit hasChildrenChanged( hasChildren );
            QList<int> indexes;
            indexes << m_d->umCol;
            for( int i=0; i < m_d->dataModel->priceDataSetCount(); ++i ){
                indexes << m_d->firstValueCol + i;
            }
            setHasChildrenChanged( this, indexes );
        }
    }

    emit itemChanged();

    return true;
}

bool PriceItem::appendChildren( int count ) {
    return insertChildren( m_d->childrenContainer.size(), count);
}

bool PriceItem::removeChildren(int position, int count) {
    if (position < 0 || position + count > m_d->childrenContainer.size())
        return false;

    bool hadChildren = m_d->childrenContainer.size() > 0;

    for (int row = 0; row < count; ++row){
        m_d->childrenContainer.takeAt( position );
    }

    if( m_d->parentItem ){
        bool hasChildren = m_d->childrenContainer.size() > 0;
        if( hadChildren ^ hasChildren ){
            emit hasChildrenChanged( hasChildren );
            QList<int> indexes;
            indexes << m_d->umCol;
            for( int i=0; i < m_d->dataModel->priceDataSetCount(); ++i ){
                indexes << m_d->firstValueCol + i;
            }
            setHasChildrenChanged( this, indexes );
        }
    }

    emit itemChanged();

    return true;
}

int PriceItem::childNumber() const {
    if (m_d->parentItem)
        return m_d->parentItem->m_d->childrenContainer.indexOf( const_cast<PriceItem *>(this) );

    return 0;
}

QList<PriceItem *> PriceItem::allChildrenList() {
    QList<PriceItem *> ret;
    ret.append(this);
    for( QList<PriceItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i) {
        ret.append( (*i)->allChildrenList() );
    }
    return ret;
}

PriceItem *PriceItem::priceItemId(unsigned int dd) {
    if( m_d->parentItem == NULL ){
        return priceItemIdChildren(dd);
    } else {
        return m_d->parentItem->priceItemId(dd);
    }
}

PriceItem *PriceItem::priceItemIdChildren(unsigned int dd) {
    if( dd == m_d->id ){
        return this;
    }
    for( QList<PriceItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
        PriceItem * pItem = (*i)->priceItemIdChildren(dd);
        if( pItem != NULL ){
            return pItem;
        }
    }
    return NULL;
}

PriceItem *PriceItem::priceItemFullCode(const QString & c) {
    if( m_d->parentItem == NULL ){
        return priceItemFullCodeChildren(c);
    } else {
        return m_d->parentItem->priceItemFullCode(c);
    }
}

PriceItem *PriceItem::priceItemFullCodeChildren(const QString & c) {
    if( c == codeFull() ){
        return this;
    }
    for( QList<PriceItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
        PriceItem * pItem = (*i)->priceItemFullCodeChildren(c);
        if( pItem != NULL ){
            return pItem;
        }
    }
    return NULL;
}

Qt::ItemFlags PriceItem::flags( int column ) const {
    if( hasChildren() ){
        if( (column == m_d->codeCol) || (column == m_d->sDescCol) ){
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        } else {
            return Qt::NoItemFlags;
        }
    } else {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }
}

QVariant PriceItem::data(int column, int role) const {
    if( (column >= m_d->firstValueCol + m_d->priceFieldModel->fieldCount()*m_d->dataModel->priceDataSetCount()) ||
            (column < 0) ||
            (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::TextAlignmentRole ) ){
        return QVariant();
    }

    if( column == m_d->codeCol ){
        if( role == Qt::TextAlignmentRole ){
            return Qt::AlignLeft + Qt::AlignVCenter;
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant(QObject::trUtf8("Codice") );
            } else {
                if( role == Qt::EditRole ){
                    return QVariant(m_d->code);
                } else {
                    return QVariant( codeFull() );
                }
            }
        }
    } else if( column == m_d->sDescCol ){
        if( role == Qt::TextAlignmentRole ){
            return Qt::AlignLeft + Qt::AlignVCenter;
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant(QObject::trUtf8("Denominazione") );
            } else {
                return QVariant(m_d->shortDescription);
            }
        }
    } else if( column == m_d->umCol ){
        if( role == Qt::TextAlignmentRole ){
            return Qt::AlignCenter + Qt::AlignVCenter;
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant(QObject::trUtf8("UdM") );
            }
            if( hasChildren() ){
                return QVariant();
            } else {
                if( m_d->unitMeasure ){
                    return QVariant(m_d->unitMeasure->tag() );
                } else {
                    return QVariant("---");
                }
            }
        }
    } else {
        if( role == Qt::TextAlignmentRole ){
            if( m_d->parentItem == NULL ){
                return Qt::AlignCenter + Qt::AlignVCenter;
            } else {
                return Qt::AlignRight + Qt::AlignVCenter;
            }
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            int priceField = column - m_d->firstValueCol;
            int priceDataSet = priceField / m_d->priceFieldModel->fieldCount();
            priceField = priceField % m_d->priceFieldModel->fieldCount();
            if( m_d->parentItem == NULL ){
                if( priceField < m_d->priceFieldModel->fieldCount() ){
                    return QVariant( m_d->priceFieldModel->priceName( priceField ) );
                }
            }
            if( hasChildren() ){
                return QVariant();
            } else {
                return QVariant( valueStr(priceField, priceDataSet ) );
            }
        }
    }
}

bool PriceItem::setData(int column, const QVariant &value) {
    if( (column >= m_d->firstValueCol + m_d->priceFieldModel->fieldCount()*m_d->dataModel->priceDataSetCount()) ||
            (column < 0) ){
        return false;
    }

    if( column == m_d->codeCol ){
        setCode( value.toString() );
        return true;
    } else if( column == m_d->sDescCol ){
        setShortDescription( value.toString() );
        return true;
    } else if( column == m_d->umCol ){
        setUnitMeasure( static_cast<UnitMeasure *>(value.value<void *>()) );
        return true;
    } else {
        int priceField = column - m_d->firstValueCol;
        int priceDataSet = priceField / m_d->priceFieldModel->fieldCount();
        priceField = priceField % m_d->priceFieldModel->fieldCount();
        return setValue( priceField, priceDataSet, value.toString() );
    }
    return false;
}

QString PriceItem::code() const {
    return m_d->code;
}

QString PriceItem::codeFull() const {
    if( m_d->parentItem != NULL ){
        if( m_d->parentItem->parentInternal() != NULL ){
            if( m_d->inheritCodeFromParent ){
                return m_d->parentItem->codeFull() + codeSeparator() + code();
            }
        }
    }
    return code();
}

void PriceItem::setCode(const QString & v) {
    if( m_d->code != v ){
        m_d->code = v;
        emit codeChanged( v );
        emit codeFullChanged( codeFull());
        emit dataChanged( this, m_d->codeCol );
    }
}

bool PriceItem::inheritCodeFromParent() {
    return m_d->inheritCodeFromParent;
}

void PriceItem::setInheritCodeFromParent(bool v ) {
    if( m_d->inheritCodeFromParent != v ){
        m_d->inheritCodeFromParent = v;
        emit inheritCodeFromParentChanged( v );
        emit codeFullChanged( codeFull() );
        emit dataChanged( this, m_d->codeCol );
        if( v && m_d->parentItem ){
            connect( m_d->parentItem, &PriceItem::codeFullChanged, this, &PriceItem::emitCodeFullChanged );
        } else {
            disconnect( m_d->parentItem, &PriceItem::codeFullChanged, this, &PriceItem::emitCodeFullChanged );
        }
    }
}

void PriceItem::emitCodeFullChanged( const QString & str ){
    emit codeFullChanged( str + codeSeparator() + m_d->code );
}

QString PriceItem::shortDescription() {
    return m_d->shortDescription;
}

QString PriceItem::shortDescriptionFull() {
    if( m_d->parentItem != NULL ){
        if( m_d->parentItem->parentInternal() != NULL ){
            if( m_d->inheritShortDescFromParent ){
                return m_d->parentItem->shortDescriptionFull() + shortDescSeparator() + shortDescription();
            }
        }
    }
    return shortDescription();
}

void PriceItem::setShortDescription(const QString & v ) {
    if( m_d->shortDescription != v ){
        m_d->shortDescription = v;
        emit shortDescriptionChanged( v );
        emit shortDescriptionFullChanged( shortDescriptionFull() );
        emit dataChanged( this, m_d->sDescCol );
    }
}

bool PriceItem::inheritShortDescFromParent() {
    return m_d->inheritShortDescFromParent;
}

void PriceItem::setInheritShortDescFromParent(bool v) {
    if( m_d->inheritShortDescFromParent != v ){
        m_d->inheritShortDescFromParent = v;
        emit inheritShortDescFromParentChanged( v );
        emit shortDescriptionFullChanged( shortDescriptionFull() );
        if( v && m_d->parentItem ){
            connect( m_d->parentItem, &PriceItem::shortDescriptionFullChanged, this, &PriceItem::emitShortDescFullChanged );
        } else {
            disconnect( m_d->parentItem, &PriceItem::shortDescriptionFullChanged, this, &PriceItem::emitShortDescFullChanged );
        }
    }
}

void PriceItem::emitShortDescFullChanged(const QString & str ) {
    emit shortDescriptionFullChanged( str + shortDescSeparator() + m_d->shortDescription );
}

QString PriceItem::longDescription() {
    return m_d->longDescription;
}

QString PriceItem::longDescriptionFull() {
    if( m_d->parentItem != NULL ){
        if( m_d->parentItem->parentInternal() != NULL ){
            if( m_d->inheritLongDescFromParent ){
                return m_d->parentItem->longDescriptionFull() + longDescSeparator() + longDescription();
            }
        }
    }
    return longDescription();
}

void PriceItem::setLongDescription(const QString & v ) {
    if( v != m_d->longDescription ){
        m_d->longDescription = v;
        emit longDescriptionChanged( v );
        emit longDescriptionFullChanged( longDescriptionFull() );
    }
}

bool PriceItem::inheritLongDescFromParent() {
    return m_d->inheritLongDescFromParent;
}

void PriceItem::setInheritLongDescFromParent( bool v ) {
    if( m_d->inheritLongDescFromParent != v ){
        m_d->inheritLongDescFromParent = v;
        emit inheritLongDescFromParentChanged( v );
        emit longDescriptionFullChanged( longDescriptionFull() );
        if( v && m_d->parentItem ){
            connect( m_d->parentItem, &PriceItem::longDescriptionFullChanged, this, &PriceItem::emitLongDescFullChanged );
        } else {
            disconnect( m_d->parentItem, &PriceItem::longDescriptionFullChanged, this, &PriceItem::emitLongDescFullChanged );
        }
    }
}

void PriceItem::emitLongDescFullChanged(const QString & str ) {
    emit longDescriptionFullChanged( str + longDescSeparator() + m_d->longDescription );
}

void PriceItem::addChild(PriceItem * newChild, int position ) {
    m_d->childrenContainer.insert( position, newChild );
}

void PriceItem::removeChild( int position ) {
    m_d->childrenContainer.removeAt( position );
}

void PriceItem::setHasChildrenChanged(PriceItem *p, QList<int> indexes ) {
    if( m_d->parentItem ){
        // non è l'oggetto root - rimandiamo all'oggetto root
        m_d->parentItem->setHasChildrenChanged( p, indexes );
    } else {
        // è l'oggetto root - emette il segnale di numero figli cambiato
        emit hasChildrenChanged( p, indexes );
    }
}

void PriceItem::writeXml(QXmlStreamWriter *writer) {
    if( m_d->parentItem != NULL ){
        // se non e' l'elemento root
        writer->writeStartElement( "PriceItem" );
        writer->writeAttribute( "id", QString::number(m_d->id) );
        writer->writeAttribute( "code", m_d->code );
        writer->writeAttribute( "inheritCodeFromParent", PriceItemPrivate::boolToQString( m_d->inheritCodeFromParent ) );
        writer->writeAttribute( "shortDescription", m_d->shortDescription );
        writer->writeAttribute( "inheritShortDescFromParent", PriceItemPrivate::boolToQString( m_d->inheritShortDescFromParent ) );
        writer->writeAttribute( "longDescription", m_d->longDescription );
        writer->writeAttribute( "inheritLongDescFromParent", PriceItemPrivate::boolToQString( m_d->inheritLongDescFromParent ) );
        if( m_d->unitMeasure ){
            writer->writeAttribute( "unitMeasure", QString::number(m_d->unitMeasure->id()) );
        }

        if( !hasChildren() ){
            m_d->dataModel->writeXml( writer );
        }

        for( QList<PriceItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeXml( writer );
        }

        writer->writeEndElement();
    } else {
        // e' l'elemento root
        for( QList<PriceItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeXml( writer );
        }
    }
}

void PriceItem::readXml(QXmlStreamReader *reader, UnitMeasureModel * uml ) {
    if( m_d->parentItem != NULL ){
        if(reader->isStartElement() && reader->name().toString().toUpper() == "PRICEITEM"){
            loadFromXml( reader->attributes(), uml );
        }
        reader->readNext();
    }
    bool readingPriceDataSet = false;
    int currentPriceDataSet = -1;
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "PRICEITEM") &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "PRICELIST") ){
        if( reader->name().toString().toUpper() == "PRICEITEMDATASET" && reader->isStartElement()) {
            readingPriceDataSet = true;
            currentPriceDataSet++;
            if( currentPriceDataSet >= m_d->dataModel->priceDataSetCount() ) {
                m_d->dataModel->appendPriceDataSet( currentPriceDataSet - m_d->dataModel->priceDataSetCount() + 1);
            }
            m_d->dataModel->loadXmlPriceDataSet( currentPriceDataSet, reader->attributes() );
        }
        if( reader->name().toString().toUpper() == "PRICEITEMDATASET" && reader->isEndElement() ) {
            readingPriceDataSet = false;
        }
        if( readingPriceDataSet &&
                (reader->name().toString().toUpper() == "BILL" && reader->isStartElement())) {
            m_d->dataModel->setAssociateAP( currentPriceDataSet );
            m_d->dataModel->associatedAP( currentPriceDataSet )->readXmlTmp( reader );
        }
        if( reader->name().toString().toUpper() == "PRICEITEM" && reader->isStartElement()) {
            appendChildren();
            m_d->childrenContainer.last()->readXml( reader, uml );
        }

        reader->readNext();
    }
}

void PriceItem::loadFromXml(const QXmlStreamAttributes &attrs, UnitMeasureModel * uml ) {
    if( attrs.hasAttribute( "id" ) ){
        m_d->id = attrs.value( "id").toUInt();
    }
    if( attrs.hasAttribute( "code" ) ){
        setCode( attrs.value( "code").toString() );
    }
    if( attrs.hasAttribute( "inheritCodeFromParent" ) ){
        setInheritCodeFromParent( PriceItemPrivate::QStringToBool( attrs.value( "inheritCodeFromParent").toString() ) );
    }
    if( attrs.hasAttribute( "shortDescription" ) ){
        setShortDescription( attrs.value( "shortDescription").toString() );
    }
    if( attrs.hasAttribute( "inheritShortDescFromParent" ) ){
        setInheritShortDescFromParent( PriceItemPrivate::QStringToBool( attrs.value( "inheritShortDescFromParent").toString() ) );
    }
    if( attrs.hasAttribute( "longDescription" ) ){
        setLongDescription( attrs.value( "longDescription").toString() );
    }
    if( attrs.hasAttribute( "inheritLongDescFromParent" ) ){
        setInheritLongDescFromParent( PriceItemPrivate::QStringToBool( attrs.value( "inheritLongDescFromParent").toString() ) );
    }
    if( attrs.hasAttribute( "unitMeasure" ) ){
        setUnitMeasure( uml->unitMeasureId( attrs.value( "unitMeasure").toUInt() ) );
    }
}

void PriceItem::loadTmpData( ProjectPriceListParentItem * priceLists ) {
    if( hasChildren() ){
        for( QList<PriceItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->loadTmpData( priceLists );
        }
    } else if(m_d->parentItem != NULL ){
        m_d->dataModel->loadTmpData( priceLists );
    }
}

#include "qtextformatuserdefined.h"

void PriceItem::writeODTOnTable( QTextCursor *cursor,
                                 PriceListPrinter::PrintPriceItemsOption printOption,
                                 const QList<int> fieldsToPrint,
                                 int priceDataSetToPrint ) {
    double borderWidth = 1.0f;

    // stile dell'unità di misura e del numero d'ordine
    static QTextBlockFormat headerBlockFormat;
    headerBlockFormat.setAlignment( Qt::AlignHCenter );
    // stile dell'unità di misura
    static QTextBlockFormat tagBlockFormat;
    tagBlockFormat.setAlignment( Qt::AlignHCenter );
    // stile dei testi
    static QTextBlockFormat txtBlockFormat;
    txtBlockFormat.setAlignment( Qt::AlignLeft );
    static QTextCharFormat txtCharFormat;
    static QTextCharFormat txtBoldCharFormat = txtCharFormat;
    txtBoldCharFormat.setFontWeight( QFont::Bold );
    // stile dei numero
    static QTextBlockFormat numBlockFormat;
    numBlockFormat.setAlignment( Qt::AlignRight );

    // stile della riga generica
    static QTextTableCellFormat centralFormat;
    static QTextTableCellFormat leftFormat;
    leftFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    static QTextTableCellFormat rightFormat;
    rightFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // stile dell'intestazione della tabella
    static QTextTableCellFormat centralHeaderFormat;
    centralHeaderFormat.setFontWeight( QFont::Bold );
    centralHeaderFormat.setBackground( Qt::lightGray );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(borderWidth) );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomWidth, QVariant(borderWidth) );
    static QTextTableCellFormat leftHeaderFormat = centralHeaderFormat;
    leftHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    static QTextTableCellFormat rightHeaderFormat = centralHeaderFormat;
    rightHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // stile della riga di chiusura
    static QTextTableCellFormat centralBottomFormat;
    centralBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    centralBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomWidth, QVariant(borderWidth) );
    static QTextTableCellFormat leftBottomFormat = centralBottomFormat;
    leftBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    static QTextTableCellFormat rightBottomFormat = centralBottomFormat;
    rightBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    QTextTable *table = cursor->currentTable();

    int cellCount = 3 + fieldsToPrint.size();

    if( m_d->parentItem == NULL ){
        PriceItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, QObject::trUtf8("Art. Elenco"), false );
        PriceItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, QObject::trUtf8("Descrizione"));
        PriceItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, QObject::trUtf8("Unità di Misura"));
        for( int i=0; i < fieldsToPrint.size(); ++i ){
            if( i == fieldsToPrint.size() - 1 ){
                PriceItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
            } else {
                PriceItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
            }
        }

        PriceItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat);

        QList<PriceItem *>::iterator i = m_d->childrenContainer.begin();
        while( i != m_d->childrenContainer.end() ){
            (*i)->writeODTOnTable( cursor, printOption, fieldsToPrint, priceDataSetToPrint  );
            QList<PriceItem *>::iterator j = i;
            ++i;
            if( (*j)->hasChildren() && i != m_d->childrenContainer.end() ){
                PriceItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat);
            }
        }
        PriceItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat);
    } else { // m_d->parentItem != NULL
        if( hasChildren() ){
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            PriceItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, codeFull() );
            if( printOption == PriceListPrinter::PrintShortDesc ){
                PriceItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, shortDescription() );
            } else if( printOption == PriceListPrinter::PrintLongDesc ){
                PriceItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, longDescription() );
            } else if( printOption == PriceListPrinter::PrintShortLongDesc ){
                QList< QPair<QString, QTextCharFormat> > txt;
                txt << qMakePair( shortDescription(), txtBoldCharFormat );
                txt << qMakePair( "\n" + longDescription(), txtCharFormat );
                PriceItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
            } else if( printOption == PriceListPrinter::PrintShortLongDescOpt ){
                QString sDesc = shortDescription();
                while( sDesc.endsWith(" ")){
                    sDesc.remove( sDesc.length()-1, 1);
                }
                if( sDesc.endsWith( "...") ){
                    sDesc.remove( sDesc.length()-3, 3);
                }
                QString lDesc = longDescription();
                if( lDesc.startsWith(sDesc) ){
                    lDesc.remove( 0, sDesc.length() );
                }
                QList< QPair<QString, QTextCharFormat> > txt;
                txt << qMakePair( sDesc, txtBoldCharFormat );
                txt << qMakePair( "\n" + lDesc, txtCharFormat );
                PriceItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
            }
            PriceItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                if( i == fieldsToPrint.size() - 1 ){
                    PriceItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                } else {
                    PriceItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                }
            }
            for( QList<PriceItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeODTOnTable( cursor, printOption, fieldsToPrint, priceDataSetToPrint  );
            }
        } else { // ! hasChildren()
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            PriceItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, codeFull() );

            if( printOption == PriceListPrinter::PrintShortDesc ){
                PriceItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, shortDescription() );
            } else if( printOption == PriceListPrinter::PrintLongDesc ){
                PriceItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, longDescription() );
            } else if( printOption == PriceListPrinter::PrintShortLongDesc ){
                QList< QPair<QString, QTextCharFormat> > txt;
                txt << qMakePair( shortDescription(), txtBoldCharFormat );
                txt << qMakePair( "\n" + longDescription(), txtCharFormat );
                PriceItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
            } else if( printOption == PriceListPrinter::PrintShortLongDescOpt ){
                QString sDesc = shortDescription();
                while( sDesc.endsWith(" ")){
                    sDesc.remove( sDesc.length()-1, 1);
                }
                if( sDesc.endsWith( "...") ){
                    sDesc.remove( sDesc.length()-3, 3);
                }
                QString lDesc = longDescription();
                if( lDesc.startsWith(sDesc) ){
                    lDesc.remove( 0, sDesc.length() );
                }
                QList< QPair<QString, QTextCharFormat> > txt;
                txt << qMakePair( sDesc, txtBoldCharFormat );
                txt << qMakePair( "\n" + lDesc, txtCharFormat );
                PriceItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
            }
            QString txt = m_d->unitMeasure == NULL ? "": m_d->unitMeasure->tag();
            PriceItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, txt );

            for( int i=0; i < fieldsToPrint.size(); ++i ){
                if( i == fieldsToPrint.size() - 1 ){
                    PriceItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, valueStr( fieldsToPrint.at(i), priceDataSetToPrint ) );
                } else {
                    PriceItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, valueStr( fieldsToPrint.at(i), priceDataSetToPrint ) );
                }
            }
        }
    }
}

QString PriceItem::codeSeparator(){
    return QString( ".");
}

QString PriceItem::shortDescSeparator(){
    return QString( " ");
}
QString PriceItem::longDescSeparator(){
    return QString( "\n");
}
