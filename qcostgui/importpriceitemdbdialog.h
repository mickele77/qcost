#ifndef IMPORTPRICEITEMDBDIALOG_H
#define IMPORTPRICEITEMDBDIALOG_H

#include "pricelistdbwidget.h"

class ImportPriceItemDBDialogPrivate;

class UnitMeasureModel;
class PriceItem;
class QString;
class MathParser;

#include <QVariant>
#include <QDialog>

class ImportPriceItemDBDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImportPriceItemDBDialog( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions,
                                      QString *EPAFileName,
                                      PriceItem * importPriceItem,
                                      int priceDataSet,
                                      const QString & connectionName,
                                      MathParser *prs,
                                      UnitMeasureModel *uml,
                                      QWidget *parent = 0);
    explicit ImportPriceItemDBDialog( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions,
                                      QString *EPAFileName,
                                      const QString &connectionName,
                                      MathParser *prs,
                                      UnitMeasureModel *uml,
                                      QWidget *parent = 0);
    ~ImportPriceItemDBDialog();

signals:
    void importMultiPriceItemDB( const QList< QList< QPair<QString, QVariant> > > & itemDataList, const QList<int> & hierarchy );
private slots:
    void importSinglePriceItemDB();
    void importMultiPriceItemDB();
private:
    ImportPriceItemDBDialogPrivate * m_d;
};

#endif // IMPORTPRICEITEMDBDIALOG_H
