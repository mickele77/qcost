#ifndef VAR_H
#define VAR_H

#include "qcost_export.h"

class AccountingBillItem;
class BillItem;
class MathParser;
class QTextStream;
class QXmlStreamWriter;
class QXmlStreamAttributes;

#include <QObject>

class VarPrivate;

class EXPORT_QCOST_LIB_OPT Var : public QObject {
    Q_OBJECT
public:
    explicit Var( BillItem * bItem = NULL, MathParser *p = NULL );
    explicit Var( AccountingBillItem * accBItem = NULL, MathParser *p = NULL, UnitVar * ump = NULL );
    ~Var();

    Var &operator =(const Var &cp);

    QString comment() const;
    QString formula() const;
    double quantity() const;
    QString quantityStr() const;

    /** Imposta il valore della formula
     * @param connItemFromId se vero i valori tra parentesi [] sono gli id degli item connessi,
              altrimenti sono i progressiveCode */
    void setFormula(const QString &nf, bool connItemFromId = false );
    void setComment(const QString &nc);
    void setUnitVar( UnitVar * ump );

    void writeXml( QXmlStreamWriter * writer );
    void loadFromXmlTmp();
    void loadXmlTmp(const QXmlStreamAttributes &attrs);

    QList<BillItem *> connectedBillItems();
    QList<AccountingBillItem *> connectedAccBillItems();

signals:
    void quantityChanged( double );

private:
    VarPrivate * m_d;
    void updateQuantity();
};

#endif // VAR_H
