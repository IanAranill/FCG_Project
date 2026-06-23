# Sistema operativo di destinazione
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Cross-compilatori MinGW-w64
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Root dell'ambiente cross-compilato
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# Regole di ricerca per programmi, librerie, header e pacchetti
# NEVER per i programmi: forza CMake a usare gli eseguibili del sistema host nativo
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# ONLY per librerie e header: PREVIENE rigorosamente il linking accidentale con OpenGL/SFML dell'host
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)