#ifndef VAR_H
#define VAR_H

#include "qcost_export.h"

class MathParser;
class QTextStream;
class QXmlStreamWriter;
class QXmlStreamAttributes;

#include <QObject>

class VarPrivate;

class EXPORT_QCOST_LIB_OPT Var : public QObject {
    Q_OBJECT
public:
    explicit Var(MathParser *prs);
    ~Var();

    Var &operator =(const Var &cp);

    QString comment() const;
    QString name() const;
    QString value() const;

    /** Imposta il valore della formula
     * @param connItemFromId se vero i valori tra parentesi [] sono gli id degli item connessi,
              altrimenti sono i progressiveCode */
    void setName(const QString &nf);
    void setComment(const QString &nc);
    void setValue( const QString & newVal );

    void writeXml( QXmlStreamWriter * writer );
    void loadXml(const QXmlStreamAttributes &attrs);

    void replaceVar(QString *expr);
signals:
    void quantityChanged( double );

private:
    VarPrivate * m_d;
};

#endif // VAR_H
