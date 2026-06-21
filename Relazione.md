# Relazione di Progetto: Simulazione Specchio 3D
## Fondamenti di Computer Grafica

### Obiettivo del Progetto
Il progetto si propone di realizzare un'applicazione grafica 3D in tempo reale utilizzando C++17 e le librerie OpenGL (4.1), SFML e GLM. L'obiettivo primario è l'implementazione di un effetto specchio planare realistico situato all'interno di un ambiente chiuso (una stanza). L'effetto di riflessione dinamica sarà ottenuto manipolando lo **Stencil Buffer** di OpenGL.

Per permettere all'utente di osservare i cambiamenti prospettici del riflesso da diverse angolazioni, la navigazione dell'ambiente avverrà tramite l'implementazione di una telecamera in prima persona esplorabile con mouse e tastiera. Inoltre, il progetto integra la libreria **ImGui** per fornire un'interfaccia grafica intuitiva (GUI) utile alla gestione e al debugging in tempo reale dei parametri di scena.

---

### Sviluppo a Tappe
Lo sviluppo del progetto segue un flusso di lavoro incrementale e modulare ("Stages"), gestito tramite tag di Git e uno script Bash per la compilazione simultanea delle varie tappe storiche.

#### Stage 01: Template e Inizializzazione Base
L'obiettivo di questa prima fase è stato predisporre un ambiente di sviluppo solido, prendendo spunto dalle fondamenta architetturali di progetti precedenti.

* **Cosa aggiunge:** È stato configurato un template di base che predispone l'infrastruttura del progetto. Al momento viene visualizzata esclusivamente una finestra con uno **sfondo grigio neutrale**, senza alcuna geometria (nessun pavimento) o pannello GUI renderizzato. L'obiettivo è confermare la corretta compilazione e l'inizializzazione del contesto OpenGL.

![Finestra iniziale](resources/screenshots/stage01.png)

#### Stage 02: Modularizzazione e Telecamera FPS
In questa fase il monolite iniziale è stato scomposto in moduli indipendenti e scalabili (Camera, Scene, Lights, Materials e Shaders). 

* **Cosa aggiunge:** È stata progettata e integrata una telecamera in prima persona (FPS) fluida, basata sul *delta time*, esplorabile tramite tastiera (WASD) e rotazione drag-and-drop del mouse, garantendo così una perfetta compatibilità e reattività anche su ambiente macOS. Il `main` è diventato un "direttore d'orchestra" pulito, strutturato per ospitare e renderizzare le geometrie esterne in modo ordinato.

#### Stage 03: Geometrie OFF, Illuminazione e Problemi di Scala
In questa fase l'infrastruttura è stata estesa per supportare il caricamento di modelli 3D complessi da file esterni e il calcolo realistico dell'illuminazione diffusa e speculare.

* **Cosa aggiunge:** È stata implementata la classe `Mesh` per il parsing dei file testuali `.off`, integrando l'algoritmo di calcolo delle normali per vertice (*Smooth Shading*) ereditato dalla logica originale del docente per rendere le superfici morbide alla luce. L'architettura è stata ottimizzata unificando la gestione di luci e materiali nella classe `Lighting` e delegando la matematica complessa e la gestione degli input hardware a funzioni helper esterne al `main`.
* **Stato attuale e Problematiche Spaziali:** Dal punto di vista del codice, il sistema compila ed esegue correttamente tutti i passaggi di rendering e shading. Tuttavia, l'applicazione grezza delle matrici di trasformazione (`glm::scale` e `glm::translate`) evidenzia notevoli problemi di scala e di posizionamento spaziale dovuti alle proporzioni intrinseche dei file caricati. Il modello della stanza (`corner.off`) presenta coordinate enormi e risulta eccessivamente fuori scala rispetto alla telecamera, impedendo di inquadrarlo interamente. Al contempo, il modello del coniglio (`bunny.off`) e la sorgente luminosa risentono di disallineamenti spaziali nel posizionamento manuale, finendo bloccati e compenetrati all'interno della geometria stessa delle pareti della stanza.

![Problema di scala - Stanza fuori misura](resources/screenshots/stage03.png)

![Problema di posizionamento - Coniglio compenetrato nel muro](resources/screenshots/stage03_bunny.png)

---

### Problemi Riscontrati e Soluzioni Tecniche



---

### Codice Esterno e Risorse Utilizzate
Per l'avvio del progetto è stato utilizzato il seguente supporto esterno:

* **Script del Docente:** Lo script `stages.sh` fornito dal professore è utilizzato quasi invariato per il tracciamento dei commit e la compilazione automatizzata multi-tappa (l'unica modifica effettuata è sul controllo del file .gitignore, dove ora è anche accettato `FCG_Stages/` come nome nell'ignore.
