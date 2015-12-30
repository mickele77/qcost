#ifndef VARSMODEL_H
#define VARSMODEL_H

#include "qcost_export.h"

class QXmlStreamWriter;
class QXmlStreamReader;
class Var;
class MathParser;

#include <QAbstractTableModel>

class VarsModelPrivate;

class EXPORT_QCOST_LIB_OPT VarsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit VarsModel(MathParser *prs);

    ~VarsModel();

    VarsModel &operator =(const VarsModel &cp);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex())  const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool insertRows(int row, int count = 1, const QModelIndex &parent = QModelIndex() );
    bool append( int count = 1 );
    bool removeRows(int row, int count = 1, const QModelIndex &parent = QModelIndex() );
    bool clear();

    double quantity();

    void writeXml( QXmlStreamWriter * writer );
    void readXml(QXmlStreamReader *reader);

    int varsCount();
    Var * var( int i );

    QString replaceValue(const QString &expr);
signals:
    void modelChanged();

private:
    VarsModelPrivate * m_d;
};

#endif // VARSMODEL_H
