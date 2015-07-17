Per creare l'eseguibile devi avere installato le librerie Qt, almeno la versione 5.3.
Scompatta il file e lancia i comandi
  $ qmake
  $ make
Se la compilazione va a buon fine dovresti trovare l'eseguibile con le librerie di cui ha bisogno nella cartella bin.
Su linux puoi installare l'eseguibile con il comando
  # make install
Di default l'eseguibile sarà installato in /usr/local/bin. Se vuoi cambiare tale opzione edita la riga
  target.path = /usr/local/bin/
nel file qcostgui/qcostgui.pro prima di compilare.

In caso di problemi lascia un messaggio sul forum di QCost all'indirizzo http://ingegnerialibera.altervista.org/forum/viewforum.php?f=3.

Per maggiori informazioni il sito di riferimento è http://ingegnerialibera.altervista.org/wiki/doku.php/qcost:indice.

---

To build binaries you need library Qt>=5.3.
Uncompress the file and launch
  $ qmake
  $ make
You should find binaries in folder bin/.
On linux you can install binery using
  # make install
By default binary is installed in /usr/local/bin. You can change this destination editing line
  target.path = /usr/local/bin/
in qcostgui/qcostgui.pro before launching qmake.

If you encounter problems post a message on http://ingegnerialibera.altervista.org/forum/viewforum.php?f=3

Reference site is http://ingegnerialibera.altervista.org/wiki/doku.php/qcost:indice.
