#ifndef PRICELISTDBMODEL_H
#define PRICELISTDBMODEL_H

class MathParser;

#include <QAbstractItemModel>

class PriceListDBModelPrivate;

class PriceListDBModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum PriceColType{
        nullptrCol,
        codeCol,
        shortDescCol,
        longDescCol,
        unitMeasureCol,
        priceTotalCol,
        priceHumanCol,
        priceEquipmentCol,
        priceMaterialCol,
        overehadsCol,
        profitsCol
    };
    explicit PriceListDBModel(MathParser *p, const QString & connectionName = QLatin1String( "defaultConnection" ), QObject *parent = 0);
    ~PriceListDBModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QList< QPair<QString, int> > unitMeasureModel();

    QString code( const QModelIndex &index, bool inheritFromParent=false );
    QString shortDescription( const QModelIndex &index, bool inheritFromParent=false );
    QString longDescription( const QModelIndex &index, bool inheritFromParent=false );
    QString unitMeasure( const QModelIndex &index );
    double priceTotal( const QModelIndex &index );
    double priceHuman( const QModelIndex &index );
    double priceEquipment(const QModelIndex &index);
    double priceMaterial(const QModelIndex &index);
    double overheads(const QModelIndex &index);
    QString overheadsStr(const QModelIndex &index);
    double profits(const QModelIndex &index);
    QString profitsStr(const QModelIndex &index);

    QString currentFile();
    bool setCurrentFile(const QString &newFile );

    void applyFilter( const QString & filter );

private:
    PriceListDBModelPrivate * m_d;

    void createPriceListTable();
    void addAllParents();
    void addAllChildren();
};

#endif // PRICELISTDBMODEL_H
