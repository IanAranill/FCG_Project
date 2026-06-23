# FCG Project: Simulazione Specchio 3D

Questo è il progetto finale per il corso di **Fondamenti di Computer Grafica**. L'obiettivo è realizzare uno specchio funzionante in tempo reale, sfruttando lo **Stencil Buffer**, all'interno di un ambiente 3D chiuso ed esplorabile con una telecamera in prima persona (FPS).

Il motore di rendering è scritto in **C++17** e si appoggia a **SFML**, **OpenGL 4.1** e **GLM**.

Lo sviluppo procede per tappe incrementali (Stages), tracciate tramite i tag di Git e gestite da uno script automatizzato per la compilazione simultanea (stages.sh).

## Compilazione (Build)

### Prerequisiti

Per compilare il progetto sono necessari **CMake** (v3.5 o superiore) e un compilatore che supporti lo standard **C++17** (come GCC, Clang o MSVC).

### Modalità di Sviluppo (Standalone)

Per compilare ed eseguire il codice esegui questi comandi dalla cartella radice del progetto:

#### Configurazione del Progetto

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

#### Compilazione del Progetto

```bash
cmake --build build
```

Una volta completata la compilazione con successo, l'eseguibile e le eventuali risorse verranno generati all'interno della cartella di output `build/`.

#### Esecuzione dell'Applicazione

```bash
./build/main
```

*(Su sistemi Windows utilizzare `.\build\main.exe`)*

## Gestione e Compilazione delle Tappe (Stages Workflow)

Questo repository tiene traccia delle tappe evolutive del progetto utilizzando i tag di Git (`Stage_01`, `Stage_02`, ecc.). Uno script di automazione centralizzato, `stages.sh`, gestisce l'estrazione delle tappe storiche e la loro compilazione all'interno di un'unica cartella isolata dal repository principale.

### 1. Esportazione delle Tappe

Per estrarre tutte le tappe taggate del progetto in sottocartelle modulari e separate (es. `FCG_Stages/Stage_01`) senza intaccare il tuo branch di sviluppo attuale, esegui:

```bash
./stages.sh x
# oppure: ./stages.sh export
```

### 2. Compilazione Centralizzata

Una volta esportate, puoi compilare tutte le tappe evolutive in sequenza con un unico comando automatizzato:

```bash
./stages.sh c
# oppure: ./stages.sh compile
```

In alternativa, puoi concatenare i processi di esportazione e compilazione in un singolo step:

```bash
./stages.sh a
# oppure: ./stages.sh all
```

### 3. Avvio di un Eseguibile Indipendente (Tappa specifica)

Dopo aver completato la compilazione multi-tappa, gli eseguibili indipendenti per ogni pietra miliare si troveranno all'interno delle rispettive directory di build nella cartella delle tappe:

```bash
# Per avviare lo Stage 01
cd FCG_Stages/Stage_01
./build/main
```
Gli stages seguenti si avviano nello stesso modo, nelle relative cartelle

## Comandi e Interfaccia Utente

### Stage 01 (Template e Inizializzazione Base)

In questa fase iniziale, l'infrastruttura di base è stata predisposta. Il motore grafico inizializza il contesto OpenGL, carica le librerie necessarie ed esegue il game loop di base con una finestra pulita (sfondo grigio).

#### Controlli Tastiera Stage 01

* **`ESC`**: Chiude l'applicazione in modo sicuro e arresta immediatamente il loop di rendering.

#### Controlli Tastiera Stage 02

* **`ESC`**: Chiude l'applicazione in modo sicuro e arresta immediatamente il loop di rendering.
* Comandi **`WASD`** per il movimento nelle quattro direzioni, non essendoci mesh da vedere il movimento non è verificabile a occhio nudo.

#### Controlli Mouse Stage 02

* Tenendo premuto il tasto sinistro del mouse e muovendolo si controlla la rotazione della telecamera.

#### Controlli Tastiera Stage 03

* **`ESC`**: Chiude l'applicazione in modo sicuro e arresta immediatamente il loop di rendering.
* Comandi **`WASD`** per il movimento nelle quattro direzioni, la telecamera può muoversi verso l'alto staccandosi dal suolo.

#### Controlli Mouse Stage 03

* Tenendo premuto il tasto sinistro del mouse e muovendolo si controlla la rotazione della telecamera.

#### Controlli Stage 04

* I controlli rimangono invariati

#### Controlli Stage 05

* I controlli rimangono invariati

#### Controlli Stage 06

* **Mouse (Movimento):** Ruota la visuale / Guarda attorno (Il cursore viene catturato automaticamente all'avvio della finestra).
* **Movimento Lineare:** Usa i tasti <kbd>W</kbd>, <kbd>A</kbd>, <kbd>S</kbd>, <kbd>D</kbd> per spostare la telecamera nello spazio (Avanti, Sinistra, Indietro, Destra).

#### Controlli Stage 07

* **`Right Shift` (Shift Destro):** Tasto rapido per attivare o disattivare la Modalità Interfaccia.
  * *Quando l'interfaccia è attiva:* la telecamera viene bloccata e il cursore del mouse compare sullo schermo, permettendo di interagire con i menu.
  * *Quando l'interfaccia è disattiva:* il mouse scompare (*grabbed* e *not visible*) e riprende il controllo della visuale in prima persona della telecamera.

#### Funzionalità del Pannello "Impostazioni"
* **Luce:** Consente di regolare le coordinate spaziali (X, Y, Z) della sorgente luminosa e di utilizzare un selettore visivo per modificare i colori della luce diretta e ambientale (tramite slider e campi scrivibili con doppio click).
* **Modelli (Bunny e Stanza):**
  * **Materiali:** Permette di bilanciare i coefficienti *Ambient*, *Diffuse* e *Specular* di ogni modello visibile. È possibile modificare il parametro "Shininess" per rendere la superficie più opaca o più lucida.
  * **Trasformazioni:** Consente di modificare le coordinate di "Posizione" per traslare l'oggetto nello spazio, o di utilizzare lo slider "Scala" per ingrandire e rimpicciolire il modello in modo uniforme e proporzionale su tutti gli assi(tramite slider e campi scrivibili con doppio click).
