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
#include "pricefieldmodel.h"

#include "mathparser.h"
#include "unitmeasure.h"

#include <QXmlStreamAttributes>
#include <QColor>
#include <QList>

class PriceFieldData{
public:
    PriceFieldData( const QString &np = QObject::trUtf8( "Prezzo"),
                    const QString &na = QObject::trUtf8( "Importo"),
                    const QString &um = QObject::trUtf8( "€"),
                    int p = 2,
                    bool af = false,
                    const QString &f = QString(),
                    PriceFieldModel::FieldType ft = PriceFieldModel::None ):
        priceName(np),
        amountName(na),
        unitMeasure(um),
        precision(p),
        applyFormula(af),
        formula( f ),
        fieldType(ft) {
    }

    PriceFieldData &operator =(const PriceFieldData &cp){
        if( &cp != this ){
            priceName = cp.priceName;
            amountName = cp.amountName;
            unitMeasure = cp.unitMeasure;
            precision = cp.precision;
            applyFormula = cp.applyFormula;
            formula = cp.formula;
            fieldType = cp.fieldType;
        }
        return *this;
    }

    // restituisce il numero dei campi connessi senza ricorsioni
    QList<int> calcConnectedFields(){
        QList<int> ret;
        QString f = formula;
        int i=0;
        int lastLimit = -1;
        while( i < f.size() ){
            if( f.at(i) == fieldLimit ){
                if( lastLimit < 0 ){
                    lastLimit = i;
                } else {
                    if( (lastLimit+1) < f.size() ){
                        QString fieldStr = f.mid( lastLimit + 1,  i-lastLimit-1);
                        bool ok = false;
                        int fieldNum = fieldStr.toInt( &ok ) -1 ;
                        if( (!ret.contains( fieldNum )) && (fieldNum > -1) && ok ){
                            ret << fieldNum;
                        }
                        lastLimit = -1;
                    }
                }
            }
            ++i;
        }
        return ret;
    }

    bool appendConnectedFields( QList<int> * conFields ){
        bool ret = false;
        QList<int> myConFields = calcConnectedFields();
        for( int i=0; i < myConFields.size(); ++i ){
            if( !conFields->contains(myConFields.at(i)) ){
                if( !ret ){
                    ret = true;
                }
                conFields->append( myConFields.at(i) );
            }
        }
        return ret;
    }

    static QString fromBoolToQString( bool v ){
        if( v ){
            return QString("true");
        }
        return QString("false");
    }

    static bool fromQStringToBool( const QString & v ){
        if( v.toUpper() == "TRUE" ){
            return true;
        }
        return false;
    }

    static QString fromFieldTypeToQString( PriceFieldModel::FieldType v ){
        if( v == PriceFieldModel::PriceTotal ){
            return "PriceTotal";
        } else if( v == PriceFieldModel::PriceHuman ){
            return "PriceHuman";
        } else if( v == PriceFieldModel::PriceHumanNet ){
            return "PriceHumanNet";
        } else if( v == PriceFieldModel::PriceNoHumanNet ){
            return "PriceNoHumanNet";
        } else if( v == PriceFieldModel::PriceEquipment ){
            return "PriceEquipment";
        } else if( v == PriceFieldModel::PriceMaterial ){
            return "PriceMaterial";
        }
        return "None";
    }

    static PriceFieldModel::FieldType fromQStringToFieldType( const QString & v ){
        QString vUp = v.toUpper();
        if( vUp == "PRICETOTAL" ){
            return PriceFieldModel::PriceTotal;
        } else if( vUp == "PRICEHUMAN" ){
            return PriceFieldModel::PriceHuman;
        } else if( vUp == "PRICEHUMANNET" ){
            return PriceFieldModel::PriceHumanNet;
        } else if( vUp == "PRICENOHUMANNET" ){
            return PriceFieldModel::PriceNoHumanNet;
        } else if( vUp == "PRICEEQUIPMENT" ){
            return PriceFieldModel::PriceEquipment;
        } else if( vUp == "PRICEMATERIAL" ){
            return PriceFieldModel::PriceMaterial;
        }
        return PriceFieldModel::None;
    }

    void writeXml(QXmlStreamWriter *writer, MathParser * parser ) {
        writer->writeStartElement( "PriceFieldData" );
        writer->writeAttribute( "priceName", priceName );
        writer->writeAttribute( "amountName", amountName );
        writer->writeAttribute( "unitMeasure", unitMeasure );
        writer->writeAttribute( "precision", QString::number( precision ) );
        writer->writeAttribute( "applyFormula", fromBoolToQString( applyFormula ) );
        QString formulaToWrite = formula;
        if( parser != NULL ){
            formulaToWrite.replace( parser->decimalSeparator(), ".");
        }
        writer->writeAttribute( "formula", formulaToWrite );
        writer->writeAttribute( "fieldType", fromFieldTypeToQString( fieldType ) );
        writer->writeEndElement();
    }

    QString priceName;
    QString amountName;
    QString unitMeasure;
    int unitMeasureCol;
    int precision;
    bool applyFormula;
    QString formula;
    PriceFieldModel::FieldType fieldType;
    static QChar fieldLimit;
    QList<int> effectiveConnectedFields;
    bool isFormulaValid;
};

QChar PriceFieldData::fieldLimit = QChar('$');

class PriceFieldModelPrivate{
public:
    PriceFieldModelPrivate( MathParser * prs ):
        parser(prs){
    }

    bool isIndexValid( int i ){
        if( i > -1 && i < fieldsList.size() ){
            return true;
        }
        return false;
    }

    // restituisce il numero dei campi effettivamente connessi
    QList<int> calcEffectiveConnectedFields( int i ){
        QList<int> ret;
        if(isIndexValid(i)){
            ret = fieldsList.at(i)->calcConnectedFields();
            int j=0;
            while( j < ret.size() ){
                int conFieldJ = ret.at(j);
                if( isIndexValid(conFieldJ) ){
                    fieldsList.at(conFieldJ)->appendConnectedFields( &ret );
                }
                j++;
            }
        }
        return ret;
    }

    // controlla che nella formula non ci siano loop
    void updateIsFormulaValid( int i ){
        if( isIndexValid(i) ){
            updateEffectiveConnectedFields(i);
            fieldsList.at(i)->isFormulaValid = !fieldsList.at(i)->effectiveConnectedFields.contains(i);
        }
    }

    // calcola i campi effetivamente connessi
    void updateEffectiveConnectedFields( int i ){
        if( isIndexValid(i) ){
            fieldsList.at(i)->effectiveConnectedFields.clear();
            fieldsList.at(i)->effectiveConnectedFields = calcEffectiveConnectedFields( i );
        }
    }

    QString	toString(double i, char f = 'g', int prec = 6) const{
        if( parser == NULL ){
            return QString::number( i, f, prec );
        } else {
            return parser->toString( i, f, prec );
        }
    }

    MathParser * parser;
    QList<PriceFieldData *> fieldsList;
    static int priceNameCol;
    static int amountNameCol;
    static int unitMeasureCol;
    static int precisionCol;
    static int formulaCol;
    static int fieldTypeCol;
};

int PriceFieldModelPrivate::priceNameCol = 0;
int PriceFieldModelPrivate::amountNameCol = 1;
int PriceFieldModelPrivate::unitMeasureCol = 2;
int PriceFieldModelPrivate::precisionCol = 3;
int PriceFieldModelPrivate::formulaCol = 4;
int PriceFieldModelPrivate::fieldTypeCol = 5;

QList<QPair<PriceFieldModel::FieldType, QString> > PriceFieldModel::standardFieldTypeNames() {
    QList< QPair<PriceFieldModel::FieldType, QString> > ret;
    ret.append(qMakePair( None, QString("")));
    ret.append(qMakePair( PriceTotal, trUtf8("Costo Unitario")));
    ret.append(qMakePair( PriceHuman, trUtf8("Costo M.O.")));
    ret.append(qMakePair( PriceHumanNet, trUtf8("Costo M.O. netto")));
    ret.append(qMakePair( PriceNoHumanNet, trUtf8("Costo sogg. ribasso")));
    return ret;
}

int PriceFieldModel::fieldTypeCol() {
    return 5;
}

PriceFieldModel::PriceFieldModel(MathParser * prs, QObject *parent):
    QAbstractTableModel( parent ),
    m_d( new PriceFieldModelPrivate( prs ) ){
    insertRow( 0 );
    setFieldType( 0, PriceFieldModel::PriceTotal );

    connect( this, &PriceFieldModel::endInsertPriceField, this, &PriceFieldModel::modelChanged );
    connect( this, &PriceFieldModel::endRemovePriceField, this, &PriceFieldModel::modelChanged );
    connect( this, &PriceFieldModel::dataChanged, this, &PriceFieldModel::modelChanged );
}

PriceFieldModel::~PriceFieldModel() {
    delete m_d;
}

QString PriceFieldModel::priceName( int pf ) {
    if( (pf >= 0) && (pf < m_d->fieldsList.size()) ){
        return m_d->fieldsList.at(pf)->priceName;
    }
    return QString();
}

bool PriceFieldModel::setPriceName(int pf, const QString &newName) {
    if( pf < 0 || !(pf < m_d->fieldsList.size() )){
        return false;
    }
    if( m_d->fieldsList.at(pf)->priceName != newName ){
        m_d->fieldsList.at(pf)->priceName = newName;
        QModelIndex index = createIndex( pf, m_d->priceNameCol );
        emit dataChanged(index, index);
        emit priceNameChanged( pf, newName );
        return true;
    }
    return false;
}

QString PriceFieldModel::amountName(int pf) {
    if( (pf >= 0) && (pf < m_d->fieldsList.size()) ){
        return m_d->fieldsList.at(pf)->amountName;
    }
    return QString();
}

bool PriceFieldModel::setAmountName(int pf, const QString &newName) {
    if( pf < 0 || !(pf < m_d->fieldsList.size() )){
        return false;
    }
    if( m_d->fieldsList.at(pf)->amountName != newName ){
        m_d->fieldsList.at(pf)->amountName = newName;
        QModelIndex index = createIndex( pf, m_d->amountNameCol );
        emit dataChanged(index, index);
        emit amountNameChanged( pf, newName );
        return true;
    }
    return false;
}

QString PriceFieldModel::unitMeasure(int pf ) {
    if( (pf >= 0) && (pf < m_d->fieldsList.size()) ){
        return m_d->fieldsList.at(pf)->unitMeasure;
    }
    return QString();
}

bool PriceFieldModel::setUnitMeasure(int pf, const QString &newVal) {
    if( pf < 0 || !(pf < m_d->fieldsList.size() )){
        return false;
    }
    if( m_d->fieldsList.at(pf)->unitMeasure != newVal ){
        m_d->fieldsList.at(pf)->unitMeasure = newVal;
        QModelIndex index = createIndex( pf, m_d->unitMeasureCol );
        emit dataChanged(index, index);
        emit unitMeasureChanged( pf, newVal );
        return true;
    }
    return false;
}

int PriceFieldModel::precision(int pf ) {
    if( (pf >= 0) && (pf < m_d->fieldsList.size()) ){
        return m_d->fieldsList.at(pf)->precision;
    }
    return 2;
}

bool PriceFieldModel::setPrecision(int pf, int newVal) {
    if( pf < 0 || !(pf < m_d->fieldsList.size() )){
        return false;
    }
    if( m_d->fieldsList.at(pf)->precision != newVal ){
        m_d->fieldsList.at(pf)->precision = newVal;
        QModelIndex index = createIndex( pf, m_d->precisionCol );
        emit dataChanged(index, index);
        emit precisionChanged( pf, newVal );
        return true;
    }
    return false;
}

bool PriceFieldModel::applyFormula(int pf) {
    if( (pf >= 0) && (pf < m_d->fieldsList.size()) ){
        return m_d->fieldsList.at(pf)->applyFormula;
    }
    return false;
}

bool PriceFieldModel::setApplyFormula(int pf, bool newVal) {
    if( pf < 0 || !(pf < m_d->fieldsList.size() )){
        return false;
    }
    if( m_d->fieldsList.at(pf)->applyFormula != newVal ){
        m_d->fieldsList.at(pf)->applyFormula = newVal;
        QModelIndex index = createIndex( pf, m_d->formulaCol );
        emit dataChanged(index, index);
        emit applyFormulaChanged( pf, newVal );
        emit formulaChanged( pf );
        return true;
    }
    return false;
}

QString PriceFieldModel::formula(int pf) {
    if( (pf >= 0) && (pf < m_d->fieldsList.size()) ){
        return m_d->fieldsList.at(pf)->formula;
    }
    return QString();
}

bool PriceFieldModel::setFormula(int pf, const QString &newVal) {
    if( !m_d->isIndexValid(pf) ){
        return false;
    }
    if( m_d->fieldsList.at(pf)->formula != newVal ){
        m_d->fieldsList.at(pf)->formula = newVal;
        m_d->updateIsFormulaValid(pf);
        QModelIndex index = createIndex( pf, m_d->formulaCol );
        emit dataChanged(index, index);
        emit formulaChanged( pf, newVal );
        emit formulaChanged( pf );
        return true;
    }
    return false;
}

PriceFieldModel::FieldType PriceFieldModel::fieldType(int pf ) {
    if( (pf >= 0) && (pf < m_d->fieldsList.size()) ){
        return m_d->fieldsList.at(pf)->fieldType;
    }
    return None;
}

bool PriceFieldModel::setFieldType(int pf, PriceFieldModel::FieldType newVal, bool resetField) {
    if( pf < 0 || !(pf < m_d->fieldsList.size() )){
        return false;
    }
    if( m_d->fieldsList.at(pf)->fieldType != newVal ){
        m_d->fieldsList.at(pf)->fieldType = newVal;
        QModelIndex index = createIndex( pf, m_d->fieldTypeCol );
        emit dataChanged(index, index);
        emit fieldTypeChanged( pf, newVal );

        if( resetField ){
            if( newVal == PriceTotal ){
                setPriceName( pf, trUtf8("Costo Unitario") );
                setAmountName( pf, trUtf8("Importo") );
                setUnitMeasure( pf, trUtf8("€"));
                setPrecision( pf, 2 );
                setApplyFormula( pf, false );
                setFormula( pf, QString() );
            }
            if( newVal == PriceHuman){
                setPriceName( pf, trUtf8("Costo Unitario M.O.") );
                setAmountName( pf, trUtf8("Importo M.O.") );
                setUnitMeasure( pf, trUtf8("€"));
                setPrecision( pf, 2 );
                setApplyFormula( pf, false );
                setFormula( pf, QString() );
            }
            if( newVal == PriceHumanNet ){
                setPriceName( pf, trUtf8("Costo Unitario Netto M.O.") );
                setAmountName( pf, trUtf8("Importo Netto M.O.") );
                setUnitMeasure( pf, trUtf8("€"));
                setPrecision( pf, 2 );
                setApplyFormula( pf, false );
                setFormula( pf, QString() );
            }
            if( newVal == PriceNoHumanNet ){
                setPriceName( pf, trUtf8("Costo Unitario sog. rib.") );
                setAmountName( pf, trUtf8("Importo sogg. ribasso") );
                setUnitMeasure( pf, trUtf8("€"));
                setPrecision( pf, 2 );
                setApplyFormula( pf, false );
                setFormula( pf, QString() );
            }
        }
        return true;
    }
    return false;
}

PriceFieldModel &PriceFieldModel::operator =(const PriceFieldModel &cp) {
    if( &cp != this ){
        if( cp.m_d->fieldsList.size() == 0 ){
            clear();
        } else {
            int D = m_d->fieldsList.size() - cp.m_d->fieldsList.size();
            if( D > 0 ){
                removeRows( cp.m_d->fieldsList.size(), D );
            }
            if( D < 0 ){
                insertRows( m_d->fieldsList.size(), -D );
            }
            // ora le due liste hanno la stessa dimensione
            // procediamo a copiare le cell.
            for( int i=0; i < m_d->fieldsList.size(); ++i ){
                setPriceName( i, cp.m_d->fieldsList.at(i)->priceName );
                setAmountName( i, cp.m_d->fieldsList.at(i)->amountName);
                setUnitMeasure( i, cp.m_d->fieldsList.at(i)->unitMeasure );
                setPrecision( i, cp.m_d->fieldsList.at(i)->precision);
                setApplyFormula( i, cp.m_d->fieldsList.at(i)->applyFormula);
                setFormula( i, cp.m_d->fieldsList.at(i)->formula);
                setFieldType( i, cp.m_d->fieldsList.at(i)->fieldType);
            }
            for( int i=0; i < m_d->fieldsList.size(); ++i ){
                m_d->updateIsFormulaValid(i);
            }
        }
    }
    return *this;
}

QList<QString> PriceFieldModel::fieldNames() {
    QList<QString> ret;
    for( QList<PriceFieldData *>::iterator i = m_d->fieldsList.begin(); i != m_d->fieldsList.end(); ++i ){
        ret.append( (*i)->priceName );
    }
    return ret;
}

void PriceFieldModel::createHumanNetPriceFields() {
    if( rowCount() > 1 ){
        removeRows( 1, rowCount() - 1 );
    }

    insertRows( 1, 3 );

    setFieldType( 0, PriceTotal );
    setFieldType( 1, PriceHuman );
    setFieldType( 2, PriceHumanNet );
    setApplyFormula( 2, true );
    setFormula( 2, QString("$2$ / ((%1 + $SG$) * (%1 + $UI$))").arg( m_d->toString(1.0)) );
    setFieldType( 3, PriceNoHumanNet );
    setApplyFormula( 3, true );
    setFormula( 3, "$1$ - $3$" );
}

int PriceFieldModel::fieldCount() {
    return m_d->fieldsList.size();
}

QVariant PriceFieldModel::data(const QModelIndex &index, int role) const {
    if( !index.isValid() || !(index.row() < m_d->fieldsList.size()) ){
        return QVariant();
    }
    if( role == Qt::CheckStateRole ){
        if( index.column() == 4 ){
            if( m_d->fieldsList.at(index.row())->applyFormula ){
                return QVariant( Qt::Checked );
            } else {
                return QVariant( Qt::Unchecked );
            }
        }
    }

    if( role == Qt::ForegroundRole ){
        if( index.column() == m_d->formulaCol ){
            if( !m_d->fieldsList.at(index.row())->applyFormula ){
                return QColor( Qt::gray );
            } else if( !m_d->fieldsList.at(index.row())->isFormulaValid ){
                return QColor( Qt::red );
            }
        }
    }

    if( (role == Qt::DisplayRole) || (role == Qt::EditRole) ){
        if( index.column() == m_d->priceNameCol ){
            return QVariant( m_d->fieldsList.at(index.row())->priceName );
        }
        if( index.column() == m_d->amountNameCol ){
            return QVariant( m_d->fieldsList.at(index.row())->amountName );
        }
        if( index.column() == m_d->unitMeasureCol ){
            return QVariant( m_d->fieldsList.at(index.row())->unitMeasure );
        }
        if( index.column() == m_d->precisionCol ){
            return QVariant( m_d->fieldsList.at(index.row())->precision );
        }
        if( index.column() == m_d->formulaCol ){
            return QVariant( m_d->fieldsList.at(index.row())->formula );
        }
        if( index.column() == m_d->fieldTypeCol ){
            return QVariant( m_d->fieldsList.at(index.row())->fieldType );
        }
    }
    return QVariant();
}

QVariant PriceFieldModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal ) {
        if( section == m_d->priceNameCol ) {
            return trUtf8("Denom. Quant. Unit.");
        }
        if( section == m_d->amountNameCol ) {
            return trUtf8("Denom. Quantità");
        }
        if( section == m_d->unitMeasureCol ) {
            return trUtf8("UdM");
        }
        if( section == m_d->precisionCol ) {
            return trUtf8("Precisione");
        }
        if( section == m_d->formulaCol ) {
            return trUtf8("Formula");
        }
        if( section == m_d->fieldTypeCol ) {
            return trUtf8("Tipo");
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}

int PriceFieldModel::rowCount(const QModelIndex &) const {
    return m_d->fieldsList.size();
}

int PriceFieldModel::columnCount(const QModelIndex &) const {
    return 6;
}

Qt::ItemFlags PriceFieldModel::flags(const QModelIndex &index) const {
    if( !index.isValid() || !(index.row() < m_d->fieldsList.size()) ){
        return QAbstractTableModel::flags(index);
    }

    if( index.column() == m_d->formulaCol ){
        if( m_d->fieldsList.at(index.row())->applyFormula ){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
        } else {
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        }
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool PriceFieldModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && index.row() < m_d->fieldsList.size() ) {
        if( role == Qt::EditRole ){
            if( index.column() == m_d->priceNameCol ){
                setPriceName( index.row(), value.toString() );
                return true;
            }
            if( index.column() == m_d->amountNameCol ){
                setAmountName( index.row(), value.toString() );
                return true;
            }
            if( index.column() == m_d->unitMeasureCol ){
                setUnitMeasure( index.row(), value.toString() );
                return true;
            }
            if( index.column() == m_d->precisionCol ){
                setPrecision( index.row(), value.toInt() );
                return true;
            }
            if( index.column() == m_d->formulaCol ){
                setFormula( index.row(), value.toString() );
                return true;
            }
            if( index.column() == m_d->fieldTypeCol ){
                setFieldType( index.row(), (FieldType)( value.toInt()) );
                return true;
            }
        }
        if( role == Qt::CheckStateRole ){
            if( index.column() == m_d->formulaCol ){
                setApplyFormula( index.row(), (value.toInt() == Qt::Checked) );
                return true;
            }
        }
    }
    return false;
}

bool PriceFieldModel::insertRows(int row, int count, const QModelIndex &) {
    if( count < 1 ){
        return false;
    }
    if( row < 0 ){
        row = 0;
    }
    if( row > m_d->fieldsList.size() ){
        row = m_d->fieldsList.size();
    }

    beginInsertRows(QModelIndex(), row, row+count-1 );
    emit beginInsertPriceField( row, row+count-1 );
    for(int i=0; i < count; ++i){
        PriceFieldData * data = new PriceFieldData();
        m_d->fieldsList.insert( row, data );
    }
    endInsertRows();
    emit endInsertPriceField( row, row+count-1 );

    for( int i=0; i < m_d->fieldsList.size(); ++i){
        bool oldVal = m_d->fieldsList.at(i)->isFormulaValid;
        m_d->updateIsFormulaValid(i);
        bool newVal = m_d->fieldsList.at(i)->isFormulaValid;
        if( oldVal != newVal ){
            QModelIndex index = createIndex( i, m_d->formulaCol );
            emit dataChanged(index, index);
        }
    }

    return true;
}

bool PriceFieldModel::appendRow() {
    return insertRows( m_d->fieldsList.size(), 1 );
}

bool PriceFieldModel::removeRows(int row, int count, const QModelIndex &) {
    if( row == 0 ){
        row = 1;
        count--;
    }

    if( count < 1 || row < 0 || row > m_d->fieldsList.size() ){
        return false;
    }

    if( (row+count) > m_d->fieldsList.size() ){
        count = m_d->fieldsList.size() - row;
    }

    beginRemoveRows(QModelIndex(), row, row+count-1);
    emit beginRemovePriceField(row, row+count-1);
    for(int i=0; i < count; ++i){
        delete m_d->fieldsList.takeAt( row );
    }
    endRemoveRows();
    emit endRemovePriceField(row, row+count-1);

    for( int i=0; i < m_d->fieldsList.size(); ++i){
        bool oldVal = m_d->fieldsList.at(i)->isFormulaValid;
        m_d->updateIsFormulaValid(i);
        bool newVal = m_d->fieldsList.at(i)->isFormulaValid;
        if( oldVal != newVal ){
            QModelIndex index = createIndex( i, m_d->formulaCol );
            emit dataChanged(index, index);
        }
    }

    return true;
}

bool PriceFieldModel::clear() {
    if( m_d->fieldsList.size() > 0 ){
        return removeRows( 1, m_d->fieldsList.size()-1 );
    }
    return true;
}

bool PriceFieldModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow) {
    if( sourceRow < 0 || !(sourceRow < m_d->fieldsList.size()) || destinationRow < 0 || !(destinationRow < m_d->fieldsList.size()) ){
        return false;
    }
    if( !(sourceRow+count < m_d->fieldsList.size()) ){
        count = m_d->fieldsList.size() - sourceRow - 1;
    }
    if( sourceParent == destinationParent ){
        if( destinationRow > sourceRow ){
            beginMoveRows( sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationRow );
            for(int row = sourceRow + count - 1; row >= sourceRow; row++){
                m_d->fieldsList.move( row, destinationRow );
            }
            endMoveRows();
            return true;
        }
        if( destinationRow < sourceRow ){
            beginMoveRows( sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationRow );
            for(int row = sourceRow; row < sourceRow+count; row++){
                m_d->fieldsList.move( row, destinationRow );
            }
            endMoveRows();
            return true;
        }
    }

    for( int i=0; i < m_d->fieldsList.size(); ++i){
        bool oldVal = m_d->fieldsList.at(i)->isFormulaValid;
        m_d->updateIsFormulaValid(i);
        bool newVal = m_d->fieldsList.at(i)->isFormulaValid;
        if( oldVal != newVal ){
            QModelIndex index = createIndex( i, m_d->formulaCol );
            emit dataChanged(index, index);
        }
    }

    return false;
}

void PriceFieldModel::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "PriceFieldModel" );
    for( QList<PriceFieldData *>::iterator i = m_d->fieldsList.begin(); i != m_d->fieldsList.end(); ++i ){
        (*i)->writeXml( writer, m_d->parser );
    }
    writer->writeEndElement();
}

void PriceFieldModel::readXml(QXmlStreamReader *reader) {
    bool firstField = true;
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "PRICEFIELDMODEL") ){
        reader->readNext();
        if( (reader->name().toString().toUpper() == "PRICEFIELDDATA") &&
                reader->isStartElement() ) {
            if( firstField ) {
                loadFromXml( m_d->fieldsList.size() - 1, reader->attributes() );
                firstField = false;
            } else if(appendRow()){
                loadFromXml( m_d->fieldsList.size() - 1, reader->attributes() );
            }
        }
    }
}

void PriceFieldModel::loadFromXml(int pf, const QXmlStreamAttributes &attrs) {
    if( attrs.hasAttribute( "priceName" ) ){
        setPriceName( pf, attrs.value( "priceName" ).toString() );
    }
    if( attrs.hasAttribute( "amountName" ) ){
        setAmountName( pf, attrs.value( "amountName" ).toString() );
    }
    if( attrs.hasAttribute( "unitMeasure" ) ){
        setUnitMeasure( pf, attrs.value( "unitMeasure" ).toString() );
    }
    if( attrs.hasAttribute( "precision" ) ){
        setPrecision( pf, attrs.value( "precision" ).toInt() );
    }
    if( attrs.hasAttribute( "applyFormula" ) ){
        setApplyFormula( pf, PriceFieldData::fromQStringToBool( attrs.value( "applyFormula" ).toString() ) );
    }
    if( attrs.hasAttribute( "formula" ) ){
        QString f = attrs.value( "formula" ).toString();
        if( m_d->parser != NULL ){
            f.replace( ".", m_d->parser->decimalSeparator() );
        }
        setFormula( pf, f );
    }
    if( attrs.hasAttribute( "fieldType" ) ){
        setFieldType( pf, PriceFieldData::fromQStringToFieldType( attrs.value( "fieldType" ).toString() ), false );
    }
}

double PriceFieldModel::calcFormula( bool * ok, int field, QList<double> fieldValues, double overheads, double profits ) {
    if( m_d->isIndexValid(field)){
        if( m_d->fieldsList.at(field)->isFormulaValid ){
            QString valStr = formula( field );
            valStr.replace( QString("$SG$"), m_d->toString( overheads, 'g' ) );
            valStr.replace( QString("$UI$"), m_d->toString( profits, 'g' ) );
            for(int i=0; i < fieldValues.size(); ++i ){
                if( i < m_d->fieldsList.size() ){
                    valStr.replace( QString("$%1$").arg(i+1), m_d->toString(fieldValues.at(i), 'f', m_d->fieldsList.at(i)->precision ) );
                }
            }
            *ok = true;
            double val = m_d->parser->evaluate( valStr );
            val = UnitMeasure::applyPrecision( val, m_d->fieldsList.at(field)->precision );
            return val;
        }
    }
    *ok = false;
    return 0.0;
}
