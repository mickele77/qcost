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
#include "unitmeasuremodel.h"

#include "unitmeasure.h"
#include "mathparser.h"

#include <QXmlStreamReader>
#include <QTextStream>
#include <QList>

class UnitMeasureModelPrivate{
public:
    UnitMeasureModelPrivate( MathParser * prs):
        parser(prs){
    };
    void insert( int row, UnitMeasure * ump ){
        unitMeasureModel.insert( row, ump );
    };
    void removeAndDel( int row ){
        delete unitMeasureModel.at( row );
        unitMeasureModel.removeAt( row );
    };
    UnitMeasure * unitMeasure( unsigned int id ){
        for( QList<UnitMeasure *>::iterator i = unitMeasureModel.begin(); i != unitMeasureModel.end(); ++i ){
            if( (*i)->id() == id ){
                return *i;
            }
        }
        return NULL;
    };
    MathParser * parser;
    QList<UnitMeasure *> unitMeasureModel;
};

UnitMeasureModel::UnitMeasureModel(MathParser * prs, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new UnitMeasureModelPrivate(prs) ) {
    connect( this, &UnitMeasureModel::rowsInserted, this, &UnitMeasureModel::modelChanged );
    connect( this, &UnitMeasureModel::rowsMoved, this, &UnitMeasureModel::modelChanged );
    connect( this, &UnitMeasureModel::rowsRemoved, this, &UnitMeasureModel::modelChanged );
    connect( this, &UnitMeasureModel::modelReset, this, &UnitMeasureModel::modelChanged );
    connect( this, &UnitMeasureModel::dataChanged, this, &UnitMeasureModel::modelChanged );
}

UnitMeasureModel::~UnitMeasureModel(){
    delete m_d;
}

void UnitMeasureModel::insertStandardUnits(){
    insert( 0, 4);

    setData( createIndex(0,0), QVariant( trUtf8("Cadauno")) );
    setData( createIndex(0,1), QVariant( trUtf8("cad")) );
    if( m_d->parser ){
        setData( createIndex(0,2), QVariant( m_d->parser->toString( 2 ) ) );
    } else {
        setData( createIndex(0,2), QVariant( 1 ) );
    }

    setData( createIndex(1,0), QVariant( trUtf8("Metro")) );
    setData( createIndex(1,1), QVariant( trUtf8("m")) );
    if( m_d->parser ){
        setData( createIndex(1,2), QVariant( m_d->parser->toString( 2 ) ) );
    } else {
        setData( createIndex(1,2), QVariant( 2 ) );
    }

    setData( createIndex(2,0), QVariant( trUtf8("Metri quadrati")) );
    setData( createIndex(2,1), QVariant( trUtf8("m²")) );
    if( m_d->parser ){
        setData( createIndex(2,2), QVariant( m_d->parser->toString( 2 ) ) );
    } else {
        setData( createIndex(2,2), QVariant( 2 ) );
    }

    setData( createIndex(3,0), QVariant( trUtf8("Metri cubi")) );
    setData( createIndex(3,1), QVariant( trUtf8("m³")) );
    if( m_d->parser ){
        setData( createIndex(3,2), QVariant( m_d->parser->toString( 3 ) ) );
    } else {
        setData( createIndex(3,2), QVariant( 3 ) );
    }
}

int UnitMeasureModel::size() {
    return m_d->unitMeasureModel.size();
}

int UnitMeasureModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    return m_d->unitMeasureModel.size();
}

int UnitMeasureModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    return 3;
}

Qt::ItemFlags UnitMeasureModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant UnitMeasureModel::data(const QModelIndex &index, int role) const {
    if( !index.isValid() || !(index.row() < m_d->unitMeasureModel.size()) ){
        return QVariant();
    }
    switch( index.column() ){
    case 0: {
        if( role == Qt::DisplayRole || role == Qt::EditRole ){
            return QVariant( m_d->unitMeasureModel.at(index.row())->description() );
        } else if( role == Qt::TextAlignmentRole ){
            return Qt::AlignLeft + Qt::AlignVCenter;
        }
        break;}
    case 1: {
        if( role == Qt::DisplayRole || role == Qt::EditRole ){
            return QVariant( m_d->unitMeasureModel.at(index.row())->tag() );
        } else if( role == Qt::TextAlignmentRole ){
            return Qt::AlignLeft + Qt::AlignVCenter;
        }
        break;}
    case 2: {
        if( role == Qt::DisplayRole || role == Qt::EditRole ){
            if( m_d->parser ){
                return QVariant( m_d->parser->toString( m_d->unitMeasureModel.at(index.row())->precision() ) );
            } else {
                return QVariant( m_d->unitMeasureModel.at(index.row())->precision() );
            }
        } else if( role == Qt::TextAlignmentRole ){
            return Qt::AlignRight + Qt::AlignVCenter;
        }
        break;}
    }
    return QVariant();
}

bool UnitMeasureModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole && index.row() < m_d->unitMeasureModel.size() ) {
        switch( index.column() ){
        case 0: {
            m_d->unitMeasureModel.at(index.row())->setDescription( value.toString() );
            emit(dataChanged(index, index));
            return true;
            break;}
        case 1: {
            m_d->unitMeasureModel.at(index.row())->setTag( value.toString() );
            emit(dataChanged(index, index));
            return true;
            break;}
        case 2: {
            double v = 0.0;
            if( m_d->parser ){
                v = m_d->parser->evaluate(value.toString());
            } else {
                v = value.toDouble();
            }
            if( v >= 0.0 ){
                m_d->unitMeasureModel.at(index.row())->setPrecision( v );
                emit(dataChanged(index, index));
                return true;
            }
            break;}
        }
    }

    return false;
}

QVariant UnitMeasureModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return trUtf8("Descrizione");
        case 1:
            return trUtf8("Simbolo");
        case 2:
            return trUtf8("Precisione");
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}

bool UnitMeasureModel::insert(int row, int count){
    if( count < 1 ){
        return false;
    }
    if( row < 0 ){
        row = m_d->unitMeasureModel.size();
    }
    if( row > m_d->unitMeasureModel.size() ){
        row = m_d->unitMeasureModel.size();
    }
    beginInsertRows(QModelIndex(), row, row+count-1 );
    for(int i=0; i < count; ++i){
        UnitMeasure * ump = new UnitMeasure();
        while( unitMeasureId(ump->id()) != NULL ){
            ump->nextId();
        }
        m_d->insert( row, ump );
    }
    endInsertRows();
    return true;
}

bool UnitMeasureModel::append() {
    return insert( m_d->unitMeasureModel.size() );
}

int UnitMeasureModel::append(const QString & tag) {
    if( insert( m_d->unitMeasureModel.size() ) ){
        int row = m_d->unitMeasureModel.size() - 1;
        setData( createIndex(row, 1), QVariant(tag) );
        return row;
    }
    return -1;
}

void UnitMeasureModel::removeUnitMeasure(int row, int count) {
    emit removeSignal( row, count );
}

void UnitMeasureModel::clear() {
    removeUnitMeasure( 0, m_d->unitMeasureModel.size() );
}

bool UnitMeasureModel::removeUnitMeasurePrivate(int row, int count){
    if( count < 1 || row < 0 || row > m_d->unitMeasureModel.size() ){
        return false;
    }

    if( (row+count) > m_d->unitMeasureModel.size() ){
        count = m_d->unitMeasureModel.size() - row;
    }

    beginRemoveRows(QModelIndex(), row, row+count-1);
    for(int i=0; i < count; ++i){
        m_d->removeAndDel( row );
    }
    endRemoveRows();
    return true;
}

void UnitMeasureModel::clearPrivate() {
    removeUnitMeasurePrivate( 0, m_d->unitMeasureModel.size() );
}

UnitMeasure *UnitMeasureModel::unitMeasure(int i) {
    if( i >= 0 && i < m_d->unitMeasureModel.size()){
        return m_d->unitMeasureModel.value(i);
    }
    return NULL;
}

UnitMeasure *UnitMeasureModel::unitMeasureId(unsigned int id) {
    for( QList<UnitMeasure *>::iterator i = m_d->unitMeasureModel.begin(); i != m_d->unitMeasureModel.end(); ++i ){
        if( (*i)->id() == id ){
            return (*i);
        }
    }
    return NULL;
}

UnitMeasure *UnitMeasureModel::unitMeasureTag(const QString &tag) {
    for( QList<UnitMeasure *>::iterator i = m_d->unitMeasureModel.begin(); i != m_d->unitMeasureModel.end(); ++i ){
        if( (*i)->tag().toUpper() == tag.toUpper() ){
            return *i;
        }
    }
    return NULL;
}

int UnitMeasureModel::findTag(const QString & tag) {
    int i = 0;
    QList<UnitMeasure *>::iterator iter = m_d->unitMeasureModel.begin();
    while( iter != m_d->unitMeasureModel.end() ){
        if( (*iter)->tag().toUpper() == tag.toUpper() ){
            return i;
        }
        ++iter;
        ++i;
    }
    return -1;
}

void UnitMeasureModel::writeXml(QXmlStreamWriter *writer, const QString & vers ) const {
    if( (vers == "1.0") || (vers == "0.3")){
        writeXml10( writer );
    } else {
        writeXml10( writer );
    }
}

void UnitMeasureModel::writeXml10(QXmlStreamWriter *writer) const {
    writer->writeStartElement( "UnitMeasureModel" );
    for( QList<UnitMeasure *>::iterator i = m_d->unitMeasureModel.begin(); i != m_d->unitMeasureModel.end(); ++i ){
        (*i)->writeXml10( writer );
    }
    writer->writeEndElement();
}

void UnitMeasureModel::readXml(QXmlStreamReader *reader) {
    bool thereIsSomething = false;
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "UNITMEASUREMODEL") ){
        if( ! thereIsSomething ){
            thereIsSomething = true;
        }
        reader->readNext();
        if( reader->name().toString().toUpper() == "UNITMEASURE" && reader->isStartElement()) {
            if(append()){
                m_d->unitMeasureModel.last()->loadFromXml( reader->attributes() );
            }
        }
    }
    if( thereIsSomething ){
        emit modelChanged();
    }
}
