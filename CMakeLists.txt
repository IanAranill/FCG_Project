# ==============================================================================
# 1. IMPOSTAZIONI GLOBALI
# ==============================================================================
cmake_minimum_required(VERSION 3.5...4.0)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum policy version for all subprojects")

project(ImGUI_SFML_OpenGL LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ==============================================================================
# 2. GESTIONE DIPENDENZE ESTERNE (FetchContent)
# ==============================================================================
include(FetchContent)

# --- GLM ---
FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         1.0.3
    GIT_SHALLOW     ON
    SYSTEM          
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(glm)

# --- SFML ---
FetchContent_Declare(
    SFML
    GIT_REPOSITORY  https://github.com/SFML/SFML.git
    GIT_TAG         3.0.2
    GIT_SHALLOW     ON
    SYSTEM          
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(SFML)

# --- ImGui (Interfaccia Utente) ---
FetchContent_Declare(
    ImGui
    GIT_REPOSITORY  https://github.com/ocornut/imgui
    GIT_TAG         v1.91.9
    GIT_SHALLOW     ON
    SYSTEM          
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ImGui)
FetchContent_GetProperties(ImGui SOURCE_DIR IMGUI_DIR)

# --- ImGui-SFML (Binding tra ImGui e SFML) ---
set(IMGUI_SFML_FIND_SFML OFF)
FetchContent_Declare(
    ImGui-SFML
    GIT_REPOSITORY  https://github.com/SFML/imgui-sfml
    GIT_TAG         v3.0
    GIT_SHALLOW     ON
    SYSTEM          
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ImGui-SFML)
set(IMGUI_DIR ${imgui_SOURCE_DIR})

# ==============================================================================
# 3. IMPOSTAZIONI COMPILATORE E SISTEMA
# ==============================================================================

add_library(common INTERFACE)
target_compile_features(common INTERFACE cxx_std_17)
target_compile_options(common INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wall -DGL_SILENCE_DEPRECATION>
    $<$<CXX_COMPILER_ID:MSVC>: /W4>
)

find_package(OpenGL REQUIRED)

# ==============================================================================
# 4. CREAZIONE DELL'ESEGUIBILE E LINKING
# ==============================================================================
set(m "main")

add_executable(${m} 
    src/${m}.cc 
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp 
)

target_link_libraries(${m} PRIVATE 
    SFML::Window 
    glm::glm-header-only 
    ImGui-SFML::ImGui-SFML 
    OpenGL::GL 
    common
)

target_include_directories(${m} PRIVATE 
    ${imgui_SOURCE_DIR}  
    ${imgui_SOURCE_DIR}/backends 
)

# ==============================================================================
# 5. COPIA DELLE RISORSE (MESHES, SHADERS)
# ==============================================================================

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources")
        add_custom_command(
            TARGET ${m} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/resources"
                "$<TARGET_FILE_DIR:${m}>/resources"
            COMMENT "Copying assets and resources to build directory..."
        )
endif()
