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
#include "qcostdbwritergui.h"

#include "pricelistdbmodel.h"
#include "pricelistdbwidget.h"
#include "loadfromtxtdialog.h"

#include <QFileDialog>
#include <QTextStream>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QLocale>

class QCostDBWriterGUIPrivate{
public:
    QCostDBWriterGUIPrivate(QWidget *p):
        locale( new QLocale(QLocale::system()) ),
        model( new PriceListDBModel(locale, p) ),
        pldbWidget(new PriceListDBWidget( model, p )){
    };
    QLocale * locale;
    PriceListDBModel * model;

    // *** GUI ***

    PriceListDBWidget * pldbWidget;

    // Elenco file recenti
    QStringList recentFiles;

    // Menu
    QMenu * fileMenu;
    QMenu * helpMenu;
    // Toolbar
    QToolBar * fileToolBar;
    // Azioni
    QAction * newFileAct;
    QAction * openFileAct;
    QAction * importFromDBFileAct;
    QAction * importFromTXTFileAct;
    QAction * saveAct;
    QAction * saveAsAct;
    QAction *separatorAction;
    QAction * exitAct;
    QAction * aboutQtAct;
    QAction * aboutAct;
    QAction *recentFileActions[QCostDBWriterGUI::MaxRecentFiles];
};

QCostDBWriterGUI::QCostDBWriterGUI(QWidget *parent) :
    QMainWindow(parent),
    m_d( new QCostDBWriterGUIPrivate( this ) ){

    createActions();
    createMenus();
    updateRecentFileActions();
    createToolBars();

    setCentralWidget( m_d->pldbWidget );

    setWindowTitle(tr("%1[*] - %2").arg(trUtf8("Senza titolo"))
                   .arg(trUtf8("QCostDBWriter")));
    connect( m_d->model, SIGNAL(modelChanged(bool)), this, SLOT(setModified(bool)) );
    setModified( false );
}

void QCostDBWriterGUI::createActions(){
    m_d->newFileAct = new QAction(tr("&Nuovo"), this);
    m_d->newFileAct->setShortcut(tr("Ctrl+N"));
    m_d->newFileAct->setStatusTip(tr("Crea un nuovo progetto"));
    m_d->newFileAct->setIcon( QIcon(":/icons/document-new.svg"));
    connect(m_d->newFileAct, SIGNAL(triggered()), this, SLOT(newFile()) );

    m_d->openFileAct = new QAction(tr("&Apri"), this);
    m_d->openFileAct->setShortcut(tr("Ctrl+A"));
    m_d->openFileAct->setStatusTip(tr("Apre un file esistente"));
    m_d->openFileAct->setIcon( QIcon(":/icons/document-open.svg"));
    connect(m_d->openFileAct, SIGNAL(triggered()), this, SLOT(openFile()));

    m_d->importFromDBFileAct = new QAction(tr("Importa da EP&A"), this);
    m_d->importFromDBFileAct->setShortcut(tr("Ctrl+A"));
    m_d->importFromDBFileAct->setStatusTip(tr("Importa un Elenco Prezzi Archivio"));
    m_d->importFromDBFileAct->setIcon( QIcon(":/icons/document-import-db.svg"));
    connect(m_d->importFromDBFileAct, SIGNAL(triggered()), this, SLOT(importFromDB()));

    m_d->importFromTXTFileAct = new QAction(tr("Importa da &TXT"), this);
    m_d->importFromTXTFileAct->setShortcut(tr("Ctrl+T"));
    m_d->importFromTXTFileAct->setStatusTip(tr("Importa un file TXT"));
    m_d->importFromTXTFileAct->setIcon( QIcon(":/icons/document-import-txt.svg"));
    connect(m_d->importFromTXTFileAct, SIGNAL(triggered()), this, SLOT(importFromTXT()));

    m_d->saveAct = new QAction(tr("&Salva"), this);
    m_d->saveAct->setShortcut(tr("Ctrl+S"));
    m_d->saveAct->setStatusTip(tr("Salva il progetto corrente"));
    m_d->saveAct->setIcon( QIcon(":/icons/document-save.svg"));
    connect(m_d->saveAct, SIGNAL(triggered()), this, SLOT(save()));

    m_d->saveAsAct = new QAction(tr("Salva con &nome..."), this);
    m_d->saveAsAct->setShortcut(tr("Ctrl+Shift+S"));
    m_d->saveAsAct->setStatusTip(tr("Salva il progetto corrente specificando il file di destinazione"));
    m_d->saveAsAct->setIcon( QIcon(":/icons/document-save-as.svg"));
    connect(m_d->saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        m_d->recentFileActions[i] = new QAction(this);
        m_d->recentFileActions[i]->setVisible(false);
        connect(m_d->recentFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    m_d->exitAct = new QAction(tr("&Esci"), this);
    m_d->exitAct->setShortcut(tr("Ctrl+Q"));
    m_d->exitAct->setStatusTip(tr("Esci dall'applicazione"));
    m_d->exitAct->setIcon( QIcon(":/icons/system-log-out.svg"));
    connect(m_d->exitAct, SIGNAL(triggered()), this, SLOT(close()));

    m_d->aboutQtAct = new QAction(tr("Informazioni sulle Qt"), this);
    m_d->aboutQtAct->setStatusTip(tr("Mostra informazioni sulle librerie Qt"));
    connect(m_d->aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    m_d->aboutAct = new QAction(tr("&Informazioni su QCost"), this);
    m_d->aboutAct->setStatusTip(tr("Mostra informazioni su QCost"));
    connect(m_d->aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void QCostDBWriterGUI::createMenus(){
    m_d->fileMenu = menuBar()->addMenu(trUtf8("&File"));
    m_d->fileMenu->addAction( m_d->newFileAct );
    m_d->fileMenu->addAction( m_d->openFileAct );
    m_d->fileMenu->addAction( m_d->saveAct );
    m_d->fileMenu->addAction( m_d->saveAsAct );
    m_d->fileMenu->addAction( m_d->importFromDBFileAct );
    m_d->fileMenu->addAction( m_d->importFromTXTFileAct );
    m_d->separatorAction = m_d->fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i){
        m_d->fileMenu->addAction(m_d->recentFileActions[i]);
    }
    m_d->fileMenu->addSeparator();
    m_d->fileMenu->addAction( m_d->exitAct );

    m_d->helpMenu = menuBar()->addMenu(trUtf8("&Aiuto"));
    m_d->helpMenu->addAction( m_d->aboutQtAct );
    m_d->helpMenu->addAction( m_d->aboutAct );
}

void QCostDBWriterGUI::createToolBars(){
    m_d->fileToolBar = new QToolBar(trUtf8("File"), this );
    m_d->fileToolBar->addAction( m_d->newFileAct );
    m_d->fileToolBar->addAction( m_d->openFileAct );
    m_d->fileToolBar->addAction( m_d->saveAct );
    m_d->fileToolBar->addAction( m_d->saveAsAct );
    m_d->fileToolBar->addAction( m_d->importFromDBFileAct );
    m_d->fileToolBar->addAction( m_d->importFromTXTFileAct );
    m_d->fileToolBar->addAction( m_d->exitAct );
    addToolBar( Qt::TopToolBarArea, m_d->fileToolBar );
}

bool QCostDBWriterGUI::setCurrentFile(const QString &fileName, bool openExistent ) {
    if( fileName.isEmpty() ){
        QString shownName = trUtf8("Senza titolo");
        setWindowTitle(tr("%1[*] - %2").arg(shownName)
                       .arg(tr("QCostDB")));
        setModified( false );

        return true;
    } else {
        if( (QFileInfo(fileName).fileName() != QFileInfo(m_d->model->currentFile()).fileName()) ){
            // imposta il nuovo file
            m_d->model->setCurrentFile( fileName, openExistent );

            // aggiorna il titolo della finestra
            setWindowModified(false);
            QString shownName = QFileInfo(fileName).fileName();
            m_d->recentFiles.removeAll(fileName);
            m_d->recentFiles.prepend(fileName);
            setWindowTitle(tr("%1[*] - %2").arg(shownName)
                           .arg(tr("QCostDB")));
            setModified( false );
            return true;
        }
    }
    return false;
}

void QCostDBWriterGUI::setModified(bool v) {
    m_d->saveAct->setEnabled( v );
    setWindowModified( v );
}

void QCostDBWriterGUI::closeEvent(QCloseEvent *event) {
    if (okToContinue()) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool QCostDBWriterGUI::okToContinue() {
    if (isWindowModified()) {
        int r = QMessageBox::warning(this, tr("QCostDB"),
                                     trUtf8("L'elenco prezzi di archivio è stato modificato.\n"
                                            "Vuoi salvare i cambiamenti?"),
                                     QMessageBox::Yes | QMessageBox::No
                                     | QMessageBox::Cancel);
        if (r == QMessageBox::Yes) {
            return save();
        } else if (r == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void QCostDBWriterGUI::newFile() {
    if (okToContinue()) {
        m_d->model->clear();
        setModified( false );
        setCurrentFile( QString() );
        m_d->pldbWidget->clear();
    }
}

void QCostDBWriterGUI::openFile() {
    if (okToContinue()) {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        trUtf8("Apri elenco prezzi di archivio"), ".",
                                                        trUtf8("EP Archivio QCost(*.qdb)"));
        setCurrentFile( fileName, true );
        m_d->pldbWidget->clear();
    }
}

void QCostDBWriterGUI::importFromDB() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Apri File EPA..."),
                                                    QString(), tr("File EPA (*.qdb);;File generico (*)"));

    if( !fileName.isEmpty() ){
        m_d->model->importFromDB( fileName );
    }
}

void QCostDBWriterGUI::importFromTXT(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Apri File testo..."),
                                                    QString(), tr("File CSV (*.csv);;File TXT (*.txt);;File generico (*)"));

    if( !fileName.isEmpty() ){
        QFile file (fileName);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in(&file);
            in.setCodec("UTF-8");
            QList<PriceListDBModel::PriceColType> pCols;
            QString decSep = ",";
            QString thousandSep = ".";
            double overheads = 0.13;
            double profits = 0.10;
            bool setShortDescFromLong = true;
            LoadFromTXTDialog dlg( m_d->model, &pCols, &in, &decSep, &thousandSep, &setShortDescFromLong, &overheads, &profits, m_d->locale, this );
            if( dlg.exec() ){
                if( pCols.size() > 0 ){
                    m_d->model->importFromTXT( decSep, thousandSep, overheads, profits, setShortDescFromLong, &pCols, & in );
                }
            }
            file.close();
        }
    }
}

void QCostDBWriterGUI::openRecentFile() {
    if (okToContinue()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action){
            setCurrentFile( action->data().toString(), true );
        }
    }
}

bool QCostDBWriterGUI::save(){
    if ( m_d->model->currentFile().isEmpty() ) {
        return saveAs();
    } else {
        return m_d->model->saveCurrentFile();
    }
}

bool QCostDBWriterGUI::saveAs(){
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    trUtf8("Salva  elenco prezzi di archivio"), ".",
                                                    trUtf8("EP Archivio QCost(*.qdb)"));

    if (fileName.isEmpty()){
        return false;
    } else {
        setCurrentFile( fileName );
        return m_d->model->saveCurrentFile();
    }
}

void QCostDBWriterGUI::updateRecentFileActions() {
    QMutableStringListIterator i(m_d->recentFiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }

    for (int j = 0; j < MaxRecentFiles; ++j) {
        if (j < m_d->recentFiles.count()) {
            QString text = tr("&%1 %2")
                    .arg(j + 1)
                    .arg(QFileInfo( m_d->recentFiles[j]).fileName() );
            m_d->recentFileActions[j]->setText(text);
            m_d->recentFileActions[j]->setData(m_d->recentFiles[j]);
            m_d->recentFileActions[j]->setVisible(true);
        } else {
            m_d->recentFileActions[j]->setVisible(false);
        }
    }
    m_d->separatorAction->setVisible(!m_d->recentFiles.isEmpty());
}

void QCostDBWriterGUI::about() {
    QMessageBox::about(this, trUtf8("Informazioni su QCost"),
                       trUtf8("<h1>QCostDB</h1>"
                              "<h2>v0.6.1</h2>"
                              "<p>Copyright &copy; 2013-2014 Michele Mocciola."
                              "<p>QCostDB fa parte di QCost, un software per la redazione di computi metrici estimativi "
                              "rilasciato sotto licenza GPL v 3. Questo  programma deve  essere  distribuito assieme "
                              "ad una  copia della Licenza Pubblica Generica (GPL) GNU versione 3;  in caso contrario, "
                              "se ne può ottenere una contattando la Free  Software Foundation,  Inc., 59 "
                              "Temple Place, Suite 330, Boston, MA 02111-1307 USA."
                              "<p>IN PARTICOLARE SI SOTTOLINEA CHE:"
                              "<p>IL PROGRAMMA È CONCESSO IN USO GRATUITAMENTE. "
                              "IL DETENTORE DEL COPYRIGHT FORNISCE IL PROGRAMMA \"COSÌ COM'È\", SENZA ALCUN "
                              "TIPO  DI GARANZIA,  NÉ ESPLICITA  NÉ IMPLICITA;  CIÒ  COMPRENDE, SENZA "
                              "LIMITARSI  A  QUESTO,  LA  GARANZIA  IMPLICITA  DI  COMMERCIABILITÀ  E "
                              "UTILIZZABILITÀ PER UN PARTICOLARE SCOPO.  L'INTERO RISCHIO CONCERNENTE "
                              "LA QUALITÀ  E LE PRESTAZIONI  DEL PROGRAMMA È DELL'UTILIZZATORE. "
                              "SE IL PROGRAMMA DOVESSE RIVELARSI DIFETTOSO, L'UTILIZZATORE SI ASSUMERÀ IL COSTO "
                              "DI OGNI MANUTENZIONE, RIPARAZIONE O CORREZIONE NECESSARIA."
                              "<p>Trovate ulteriore informazioni sul programma all'indirizzo "
                              "<a href=\"http://ingegnerialibera.altervista.org/wiki/doku.php/qcost:indice\">ingegnerialibera.altervista.org/wiki/doku.php/qcost:indice</a>."
                              "<p>Per segnalazione di bug o per proposte di miglioramento è a disposizione "
                              "il <a href=\"http://ingegnerialibera.altervista.org/forum/viewforum.php?f=3\">forum di QCost </a> ."));
}
