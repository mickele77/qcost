#ifndef PRICELISTDBVIEWER_H
#define PRICELISTDBVIEWER_H

class MathParser;
class PriceListDBViewerPrivate;

#include <QDialog>

class PriceListDBViewer : public QDialog {
public:
    explicit PriceListDBViewer(MathParser * p, QWidget *parent = 0);
    ~PriceListDBViewer();

private:
    PriceListDBViewerPrivate * m_d;
};

#endif // PRICELISTDBVIEWER_H
