# Definisce esplicitamente il target predefinito (attivato lanciando solo 'make')
.DEFAULT_GOAL := all

# Dichiarazione di target fittizi (phony) per evitare conflitti con file omonimi
.PHONY: all format clean configure build run package all-win configure-win build-win run-win package-win all-mac configure-mac build-mac package-mac

# ==========================================
# TARGET PRINCIPALI
# ==========================================

# Pipeline di compilazione ed esecuzione nativa
all: format configure build run

# Pipeline di cross-compilazione (Windows)
all-win: format configure-win build-win run-win

# Pipeline di cross-compilazione (macOS via osxcross o toolchain Apple)
all-mac: format configure-mac build-mac

# ==========================================
# STRUMENTI CONDIVISI
# ==========================================

# Formattazione del codice sorgente basata sullo standard LLVM/C++
format:
	clang-format -i src/*.cc src/*/*.hh resources/shaders/*

# ==========================================
# AMBIENTE NATIVO
# ==========================================

# Configurazione del progetto tramite CMake in modalità Release
configure:
	cmake -B build -DCMAKE_BUILD_TYPE=Release

# Compilazione parallela dei sorgenti
build:
	cmake --build build --parallel --config Release

# Esecuzione dell'applicazione OpenGL principale
run:
	./build/main

# ==========================================
# AMBIENTE WINDOWS (CROSS-COMPILAZIONE)
# ==========================================

# Configurazione per Windows usando la toolchain MinGW-w64
configure-win:
	cmake -B build-win --toolchain windows-toolchain.cmake -DCMAKE_BUILD_TYPE=Release

# Compilazione dell'eseguibile .exe per Windows
build-win:
	cmake --build build-win --parallel --config Release

# Esecuzione dell'eseguibile Windows (Nota: su Linux/Mac richiede Wine, su MSYS/WSL potrebbe essere eseguito direttamente)
run-win:
	./build-win/main.exe

# ==========================================
# AMBIENTE MACOS (CROSS-COMPILAZIONE)
# ==========================================

# Configurazione per macOS (Universal Binary per ARM64/x86_64) tramite toolchain Darwin
configure-mac:
	cmake -B build-mac --toolchain macos-toolchain.cmake -DCMAKE_BUILD_TYPE=Release

# Compilazione dell'app per ambiente Apple
build-mac:
	cmake --build build-mac --parallel --config Release

# ==========================================
# PULIZIA
# ==========================================

# Rimuove le directory di build native e cross-compilate per garantire uno stato pulito della working directory
clean:
	rm -rf build build-win build-mac