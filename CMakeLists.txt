cmake_minimum_required(VERSION 3.20)
project(NoTrackSwitcher VERSION 1.0.0 LANGUAGES CXX)

# ---- Language / platform --------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
    message(FATAL_ERROR "NoTrackSwitcher is Windows-only")
endif()

# ---- Source files ---------------------------------------------------------
set(SOURCES
    src/main.cpp
    src/hook/KeyboardHook.cpp
    src/hook/MouseHook.cpp
    src/core/TextBuffer.cpp
    src/core/LayoutManager.cpp
    src/core/Converter.cpp
    src/core/InputSender.cpp
    src/core/ClipManager.cpp
    src/core/Worker.cpp
    src/config/Config.cpp
    src/ui/TrayIcon.cpp
    src/ui/SettingsDlg.cpp
    src/resources/resource.rc
)

# ---- Executable -----------------------------------------------------------
add_executable(NoTrackSwitcher WIN32 ${SOURCES})

target_include_directories(NoTrackSwitcher PRIVATE src src/resources)

target_compile_definitions(NoTrackSwitcher PRIVATE
    UNICODE _UNICODE
    WIN32_LEAN_AND_MEAN NOMINMAX
    _WIN32_WINNT=0x0601   # Windows 7+
)

target_link_libraries(NoTrackSwitcher PRIVATE
    user32 kernel32 shell32 gdi32 advapi32 comctl32 psapi dwmapi uxtheme
)

# ---- Compiler-specific settings -------------------------------------------
if(MSVC)
    # /MT — static CRT (no VCRUNTIME DLL dependency, truly portable EXE)
    # /O2 — optimize for speed
    # /GS- — no security cookie overhead (safe for a trusted single-user app)
    target_compile_options(NoTrackSwitcher PRIVATE
        /W3 /WX-
        $<$<CONFIG:Release>:/O1 /Os /Oi /GL /MD>
        $<$<CONFIG:Debug>:/Od /MDd>
    )
    target_link_options(NoTrackSwitcher PRIVATE
        /SUBSYSTEM:WINDOWS
        $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>
    )
    # Manifest: require Windows 7+ and disable UAC elevation
    set_target_properties(NoTrackSwitcher PROPERTIES
        VS_USER_PROPS ""
    )
elseif(MINGW)
    # -municode: startup code calls wWinMain (Unicode entry point)
    target_compile_options(NoTrackSwitcher PRIVATE
        -Wall -O2
    )
    target_link_options(NoTrackSwitcher PRIVATE
        -municode -mwindows
        -static -static-libgcc -static-libstdc++
    )

    # windres's internal preprocessor (cc1.exe) cannot handle -I paths with
    # spaces. resource.rc uses only numeric literals and no #include, so we
    # skip <INCLUDES> and <DEFINES> entirely.
    set(CMAKE_RC_COMPILE_OBJECT
        "<CMAKE_RC_COMPILER> -O coff -o <OBJECT> <SOURCE>")
endif()

# ---- Install / output -----------------------------------------------------
set_target_properties(NoTrackSwitcher PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

