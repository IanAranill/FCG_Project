FCG Project: Simulazione Specchio 3D

Questo è il progetto finale per il corso di Fondamenti di Computer Grafica. L'obiettivo è realizzare uno specchio funzionante in tempo reale, sfruttando lo Stencil Buffer, all'interno di un ambiente 3D chiuso ed esplorabile con una telecamera in prima persona (FPS).

Il motore di rendering è scritto in C++17 e si appoggia a SFML, OpenGL 4.1 e GLM.

Lo sviluppo procede per tappe incrementali (Stages), tracciate tramite i tag di Git e gestite da uno script automatizzato per la compilazione simultanea.

Compilazione

Prerequisiti

Per compilare il progetto sono necessari CMake (v3.5 o superiore) e un compilatore che supporti lo standard C++17 (come GCC, Clang o MSVC).

Modalità di Sviluppo

Per compilare ed eseguire il codice eseguire questi comandi dalla cartella radice del progetto:

Configurazione del Progetto

cmake -B build -DCMAKE_BUILD_TYPE=Release


Compilazione del Progetto

cmake --build build


Una volta completata la compilazione con successo, l'eseguibile e le eventuali risorse verranno generati all'interno della cartella di output build/.

Esecuzione dell'Applicazione

./build/main


(Su sistemi Windows utilizzare .\build\main.exe)

Gestione e Compilazione delle Tappe (Stages Workflow)

Questo repository tiene traccia delle tappe evolutive del progetto utilizzando i tag di Git (Stage_01, Stage_02, ecc.). Uno script di automazione centralizzato, stages.sh, gestisce l'estrazione delle tappe storiche e la loro compilazione all'interno di un'unica cartella isolata dal repository principale.

1. Esportazione delle Tappe

Per estrarre tutte le tappe taggate del progetto in sottocartelle modulari e separate (es. FCG_Stages/Stage_01) senza intaccare il tuo branch di sviluppo attuale, esegui:

./stages.sh x
# oppure: ./stages.sh export


2. Compilazione Centralizzata

Una volta esportate, puoi compilare tutte le tappe evolutive in sequenza con un unico comando automatizzato:

./stages.sh c
# oppure: ./stages.sh compile


In alternativa, puoi concatenare i processi di esportazione e compilazione in un singolo step:

./stages.sh a
# oppure: ./stages.sh all


3. Avvio di un Eseguibile Indipendente (Tappa specifica)

Dopo aver completato la compilazione multi-tappa, gli eseguibili indipendenti per ogni pietra miliare si troveranno all'interno delle rispettive directory di build nella cartella delle tappe:

# Per avviare lo Stage 01
cd FCG_Stages/Stage_01
./build/main


Comandi e Interfaccia Utente

Stage 01 (Template e Inizializzazione Base)

In questa fase iniziale, l'infrastruttura di base è stata predisposta. Il motore grafico inizializza il contesto OpenGL, carica le librerie necessarie ed esegue il game loop di base con una finestra pulita (sfondo grigio).

Controlli Tastiera Stage 01

ESC: Chiude l'applicazione in modo sicuro e arresta immediatamente il loop di rendering.
