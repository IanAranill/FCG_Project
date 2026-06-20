# Relazione di Progetto: Simulazione Specchio 3D
## Fondamenti di Computer Grafica

### 1. Obiettivo del Progetto
Il progetto si propone di realizzare un'applicazione grafica 3D in tempo reale utilizzando C++17 e le librerie OpenGL (4.1), SFML e GLM. L'obiettivo primario è l'implementazione di un effetto specchio planare realistico situato all'interno di un ambiente chiuso (una stanza). L'effetto di riflessione dinamica sarà ottenuto manipolando lo **Stencil Buffer** di OpenGL.

Per permettere all'utente di osservare i cambiamenti prospettici del riflesso da diverse angolazioni, la navigazione dell'ambiente avverrà tramite l'implementazione di una telecamera in prima persona esplorabile con mouse e tastiera. Inoltre, il progetto integra la libreria **ImGui** per fornire un'interfaccia grafica intuitiva (GUI) utile alla gestione e al debugging in tempo reale dei parametri di scena.

---

### 2. Sviluppo a Tappe
Lo sviluppo del progetto segue un flusso di lavoro incrementale e modulare ("Stages"), gestito tramite tag di Git e uno script Bash per la compilazione simultanea delle varie tappe storiche.

#### Stage 01: Template e Inizializzazione Base
L'obiettivo di questa prima fase è stato predisporre un ambiente di sviluppo solido, prendendo spunto dalle fondamenta architetturali di progetti precedenti.

* **Cosa aggiunge:** È stato configurato un template di base che predispone l'infrastruttura del progetto. Al momento viene visualizzata esclusivamente una finestra con uno **sfondo grigio neutrale**, senza alcuna geometria (nessun pavimento) o pannello GUI renderizzato. L'obiettivo è confermare la corretta compilazione e l'inizializzazione del contesto OpenGL.


![Finestra iniziale](resources/screenshots/stage01.png)

---

### 3. Problemi Riscontrati e Soluzioni Tecniche



---

### 4. Codice Esterno e Risorse Utilizzate
Per l'avvio del progetto è stato utilizzato il seguente supporto esterno:

* [cite_start]**Script del Docente:** Lo script `stages.sh` fornito dal professore è utilizzato invariato per il tracciamento dei commit e la compilazione automatizzata multi-tappa[cite: 4, 19, 22].