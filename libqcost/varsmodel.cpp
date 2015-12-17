#include "varsmodel.h"

#include "var.h"
#include "mathparser.h"

#include <QXmlStreamWriter>
#include <QVariant>
#include <QList>
#include <QLocale>

class VarsModelPrivate{
public:
    VarsModelPrivate( MathParser * prs ):
        valueCol(2),
        nameCol(0),
        commentCol(1),
        parser(prs){
    }
    ~VarsModelPrivate(){
    }

    QList<Var *> linesContainer;
    int valueCol;
    int nameCol;
    int commentCol;
    MathParser * parser;
};

VarsModel::VarsModel(MathParser * prs) :
    QAbstractTableModel(),
    m_d(new VarsModelPrivate(prs)){
    insertRows(0);
}

VarsModel::~VarsModel(){
    delete m_d;
}

VarsModel &VarsModel::operator=(const VarsModel &cp) {
    if( &cp != this ){
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
                if( index.column() == m_d->nameCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->name());
                }
                if( index.column() == m_d->valueCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->value() );
                }
                if( index.column() == m_d->commentCol ){
                    return QVariant( m_d->linesContainer.at(index.row())->comment());
                }
            }
            if( role == Qt::TextAlignmentRole ){
                if( index.column() == m_d->nameCol ){
                    return Qt::AlignLeft + Qt::AlignVCenter;
                }
                if( index.column() == m_d->commentCol ){
                    return Qt::AlignLeft + Qt::AlignVCenter;
                }
                if( index.column() == m_d->valueCol ){
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
                if( index.column() == m_d->commentCol ){
                    m_d->linesContainer.at(index.row())->setComment( value.toString() );
                    emit dataChanged( index, index );
                    emit modelChanged();
                    return true;
                }
                if( index.column() == m_d->nameCol ){
                    m_d->linesContainer.at(index.row())->setName( value.toString() );
                    emit dataChanged( index, index );
                    emit modelChanged();
                    return true;
                }
                if( index.column() == m_d->valueCol ){
                    m_d->linesContainer.at(index.row())->setValue( value.toString() );
                    emit dataChanged( index, index );
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
        if( section == m_d->commentCol ) {
            return trUtf8("Commento");
        }
        if( section == m_d->nameCol ) {
            return trUtf8("Variabile");
        }
        if( section == m_d->valueCol ) {
            return trUtf8("Valore");
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
        Var * itemLine = new Var( m_d->parser );
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
    emit modelChanged();
    return true;
}

bool VarsModel::append( int count ){
    return insertRows( m_d->linesContainer.size(), count );
}

void VarsModel::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "VarsModel" );
    for( QList<Var *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->writeXml( writer );
    }
    writer->writeEndElement();
}

void VarsModel::readXml(QXmlStreamReader *reader) {
    bool firstLine = true;
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "VARSMODEL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "VAR" && reader->isStartElement()) {
            if( firstLine ){
                m_d->linesContainer.last()->loadXml( reader->attributes() );
                firstLine = false;
            } else {
                if(append()){
                    m_d->linesContainer.last()->loadXml( reader->attributes() );
                }
            }
        }
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

QString VarsModel::replaceValue( const QString & expr ) {
    QString ret;
    for( QList<Var *>::iterator i = m_d->linesContainer.begin(); i != m_d->linesContainer.end(); ++i ){
        (*i)->replaceValue( &ret );
    }
    return ret;
}
