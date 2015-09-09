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
#include "priceitemdatasetviewmodel.h"

#include "priceitemdatasetmodel.h"
#include "mathparser.h"

#include <QFont>

class PriceItemDataSetViewModelPrivate{
public:
    PriceItemDataSetViewModelPrivate( PriceItemDataSetModel * m, int curPriceDataSet, MathParser * prs ):
        model(m),
        currentPriceDataSet(curPriceDataSet),
        parser(prs){
    }
    ~PriceItemDataSetViewModelPrivate(){
    }

    int fromModelToViewModel( int row ){
        if( model != NULL ){
            if( row <= model->lastValueRow() ){
                return 2*row;
            } else {
                return (2*model->lastValueRow()+1)+(row - model->lastValueRow());
            }
        }
        return -1;
    }

    int fromViewModelToModel( int row ){
        if( model != NULL ){
            int retRow = row / 2;
            if( retRow <= model->lastValueRow() ){
                if( row % 2 == 0 ){
                    return retRow;
                }
            } else {
                retRow = model->lastValueRow() + (row - (2*model->lastValueRow()+1));
                return retRow;
            }
        }
        return -1;
    }

    QString	toString(double i, char f = 'g', int prec = 6) const{
        if( parser == NULL ){
            return QString::number( i, f, prec );
        } else {
            return parser->toString( i, f, prec );
        }
    }

    PriceItemDataSetModel * model;
    int currentPriceDataSet;
    MathParser * parser;
};

PriceItemDataSetViewModel::PriceItemDataSetViewModel( PriceItemDataSetModel * m, int curPriceDataSet, MathParser * prs, QObject * parent ):
    QAbstractTableModel( parent ),
    m_d( new PriceItemDataSetViewModelPrivate( NULL, 0, prs )){
    setModel(m);
    setCurrentPriceDataSet( curPriceDataSet );
}

PriceItemDataSetViewModel::~PriceItemDataSetViewModel(){
    delete m_d;
}

void PriceItemDataSetViewModel::setModel(PriceItemDataSetModel * m ){
    if( m_d->model != m ){
        beginResetModel();
        if( m_d->model != NULL ){
            disconnect( m_d->model, &PriceItemDataSetModel::beginInsertPriceDataSets, this, &PriceItemDataSetViewModel::beginInsertPriceDataSets );
            disconnect( m_d->model, &PriceItemDataSetModel::endInsertPriceDataSets, this, &PriceItemDataSetViewModel::endInsertPriceDataSets );
            disconnect( m_d->model, &PriceItemDataSetModel::beginRemovePriceDataSets, this, &PriceItemDataSetViewModel::beginRemovePriceDataSets );
            disconnect( m_d->model, &PriceItemDataSetModel::endRemovePriceDataSets, this, &PriceItemDataSetViewModel::endRemovePriceDataSets );
            disconnect( m_d->model, &PriceItemDataSetModel::dataChanged, this, &PriceItemDataSetViewModel::dataHaveChanged );
        }
        m_d->model = m;
        if( m_d->model != NULL ){
            connect( m_d->model, &PriceItemDataSetModel::beginInsertPriceDataSets, this, &PriceItemDataSetViewModel::beginInsertPriceDataSets );
            connect( m_d->model, &PriceItemDataSetModel::endInsertPriceDataSets, this, &PriceItemDataSetViewModel::endInsertPriceDataSets );
            connect( m_d->model, &PriceItemDataSetModel::beginRemovePriceDataSets, this, &PriceItemDataSetViewModel::beginRemovePriceDataSets );
            connect( m_d->model, &PriceItemDataSetModel::endRemovePriceDataSets, this, &PriceItemDataSetViewModel::endRemovePriceDataSets );
            connect( m_d->model, &PriceItemDataSetModel::dataChanged, this, &PriceItemDataSetViewModel::dataHaveChanged );
        }
        endResetModel();
    }
}

void PriceItemDataSetViewModel::beginInsertPriceDataSets( int firstCol, int lastCol ){
    beginInsertColumns( QModelIndex(), firstCol, lastCol );
}

void PriceItemDataSetViewModel::endInsertPriceDataSets( int firstCol, int lastCol ){
    Q_UNUSED(firstCol);
    Q_UNUSED(lastCol);
    endInsertColumns();
}

void PriceItemDataSetViewModel::beginRemovePriceDataSets( int firstCol, int lastCol ){
    beginRemoveColumns( QModelIndex(), firstCol, lastCol );
}

void PriceItemDataSetViewModel::endRemovePriceDataSets( int firstCol, int lastCol ){
    Q_UNUSED(firstCol);
    Q_UNUSED(lastCol);
    endRemoveColumns();
}

void PriceItemDataSetViewModel::beginInsertPriceField( int firstRow, int lastRow ){
    beginInsertRows( QModelIndex(), m_d->fromModelToViewModel(firstRow), m_d->fromModelToViewModel(lastRow) );
}

void PriceItemDataSetViewModel::endInsertPriceField( int firstRow, int lastRow ){
    Q_UNUSED(firstRow);
    Q_UNUSED(lastRow);
    endInsertRows();
}

void PriceItemDataSetViewModel::beginRemovePriceField( int firstRow, int lastRow ){
    beginRemoveRows( QModelIndex(), m_d->fromModelToViewModel(firstRow), m_d->fromModelToViewModel(lastRow) );
}

void PriceItemDataSetViewModel::endRemovePriceField( int firstRow, int lastRow ){
    Q_UNUSED(firstRow);
    Q_UNUSED(lastRow);
    endRemoveRows();
}

void PriceItemDataSetViewModel::dataHaveChanged( const QModelIndex & topLeft, const QModelIndex & bottomRight ){
    if( m_d->model != NULL ){
        for( int modelRow = topLeft.row(); modelRow < bottomRight.row(); ++ modelRow ){
            int viewRow = m_d->fromModelToViewModel( modelRow );
            QModelIndex viewTopLeft = createIndex( viewRow, viewTopLeft.column() );
            QModelIndex viewBottomRight;
            if( modelRow <= m_d->model->lastValueRow() ){
                viewBottomRight = createIndex( viewRow+1, bottomRight.column() );
            } else {
                viewBottomRight = createIndex( viewRow, bottomRight.column() );
            }
            emit dataChanged( viewTopLeft, viewBottomRight );

            if( modelRow <= m_d->model->lastValueRow() &&
                    (m_d->currentPriceDataSet >= topLeft.column() && m_d->currentPriceDataSet <= bottomRight.column() ) ){
                viewTopLeft = createIndex( viewRow+1, 0 );
                viewBottomRight = createIndex( viewRow+1, m_d->model->priceDataSetCount()-1 );
                emit dataChanged( viewTopLeft, viewBottomRight );
            }
        }
    }
}

int PriceItemDataSetViewModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    if( m_d->model != NULL ){
        return m_d->model->lastValueRow() * 2 + 1 + ( m_d->model->rowCount(parent) - m_d->model->lastValueRow() );
    }
    return 0;
}

int PriceItemDataSetViewModel::columnCount(const QModelIndex &parent) const {
    if( m_d->model != NULL ){
        return m_d->model->columnCount( parent );
    }
    return 0;
}

Qt::ItemFlags PriceItemDataSetViewModel::flags(const QModelIndex &index) const {
    if( m_d->model != NULL ){
        int row = index.row() / 2;
        if( row <= m_d->model->lastValueRow() ){
            if( index.row() % 2 == 0 ){
                return m_d->model->flags( createIndex( row, index.column() ) );
            } else {
                return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            }
        } else {
            row = m_d->model->lastValueRow() + (index.row() - (2 * m_d->model->lastValueRow() + 1));
            return m_d->model->flags( createIndex( row, index.column() ) );
        }
    }
    return QAbstractTableModel::flags( index );
}

QVariant PriceItemDataSetViewModel::data(const QModelIndex &index, int role) const {
    if( !index.isValid() ){
        return QVariant();
    }

    if( role == Qt::FontRole){
        if( index.column() == m_d->currentPriceDataSet ){
            QFont font;
            font.setUnderline( true );
            return font;
        }
    }

    if( role == Qt::TextAlignmentRole ){
        return Qt::AlignRight + Qt::AlignVCenter ;
    }

    if( m_d->model != NULL ){
        int row = index.row() / 2;
        if( row <= m_d->model->lastValueRow() ){
            if( index.row() % 2 == 0 ){
                return m_d->model->data( createIndex( row, index.column() ), role );
            } else {
                if( role == Qt::DisplayRole ){
                    double ret = 0.0;
                    if( m_d->model->value(row, m_d->currentPriceDataSet ) != 0.0){
                        ret = (m_d->model->value(row, index.column() ) - m_d->model->value(row, m_d->currentPriceDataSet ) ) / m_d->model->value(row, m_d->currentPriceDataSet );
                    }
                    ret *= 100.0;
                    return QVariant( QString("%1 %").arg(m_d->toString(ret, 'f', 4)) );
                }
            }
        } else {
            row = m_d->model->lastValueRow() + (index.row() - (2 * m_d->model->lastValueRow() + 1));
            return m_d->model->data( createIndex( row, index.column() ), role );
        }
    }

    return QVariant();
}

bool PriceItemDataSetViewModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if( ! index.isValid() ){
        return false;
    }

    if( m_d->model != NULL ){
        int row = index.row() / 2;
        if( row <= m_d->model->lastValueRow() ){
            if( index.row() % 2 == 0 ){
                return m_d->model->setData( createIndex( row, index.column() ), value, role );
            }
        } else {
            row = m_d->model->lastValueRow() + (index.row() - (2 * m_d->model->lastValueRow() + 1));
            return m_d->model->setData( createIndex( row, index.column() ), value, role );
        }
    }

    return false;
}

QVariant PriceItemDataSetViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if( m_d->model != NULL ){
        if (orientation == Qt::Horizontal ){
            return m_d->model->headerData( section, orientation, role );
        } else {
            int row = section / 2;
            if( row <= m_d->model->lastValueRow() ){
                if( section % 2 == 0 ){
                    return m_d->model->headerData( row, orientation, role );
                } else {
#ifdef BUILD_MSVC
#define UTF8_QSTRING(str) QString::fromWCharArray(L##str)
#else
#define UTF8_QSTRING(str) QString::fromUtf8(str)
#endif
                    return QVariant( UTF8_QSTRING("Δ") + " [%]" );
               }
            } else {
                row = m_d->model->lastValueRow() + (section - (2 * m_d->model->lastValueRow() + 1));
                return m_d->model->headerData( row, orientation, role );
            }
        }
    }
    return QVariant();
}

int PriceItemDataSetViewModel::currentPriceDataSet() const {
    return m_d->currentPriceDataSet;
}

void PriceItemDataSetViewModel::setCurrentPriceDataSet(int newDataSet) {
    if( m_d->currentPriceDataSet != newDataSet ){
        m_d->currentPriceDataSet = newDataSet;
        if( m_d->model != NULL ){
            for( int i=0; i <= m_d->model->lastValueRow() ; ++i ){
                int viewRow = m_d->fromModelToViewModel( i ) + 1;
                QModelIndex topLeft = createIndex( viewRow, 0 );
                QModelIndex bottomRight = createIndex( viewRow, m_d->model->priceDataSetCount()-1 );
                emit dataChanged( topLeft, bottomRight );
            }
        }
    }
}

int PriceItemDataSetViewModel::associatedAPRow() {
    if( m_d->model != NULL ){
        int r = m_d->fromModelToViewModel( m_d->model->associatedAPRow() );
        return r;
    }
    return -1;
}
