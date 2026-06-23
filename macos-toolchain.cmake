# Sistema operativo di destinazione
set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.3" CACHE STRING "Minimum macOS version" FORCE)

# Generazione Universal Binary (Apple Silicon ARM64 + Intel x86_64)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS" FORCE)

# Cross-compilatori Apple Darwin (es. via osxcross)
set(CMAKE_C_COMPILER x86_64-apple-darwin25.1-clang)
set(CMAKE_CXX_COMPILER x86_64-apple-darwin25.1-clang++)
set(CMAKE_AR x86_64-apple-darwin25.1-ar)
set(CMAKE_RANLIB x86_64-apple-darwin25.1-ranlib)

# NOTA ARCHITETTURALE: Se ottieni errori "Header non trovato" con la policy ONLY,
# decommenta la riga seguente puntando alla root del tuo SDK di macOS estrapolato.
# set(CMAKE_FIND_ROOT_PATH /path/to/your/osxcross/SDK/MacOSX13.3.sdk)

# Regole di ricerca per programmi, librerie, header e pacchetti
# NEVER per i programmi: forza CMake a usare gli eseguibili del sistema host nativo
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# ONLY per librerie e header: PREVIENE rigorosamente il linking accidentale con OpenGL dell'host
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)