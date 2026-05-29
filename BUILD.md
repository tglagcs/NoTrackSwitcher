# Building NoTrack Switcher

**Requirements:** Windows 10/11 x64, CMake 3.20+, Ninja, and one of the toolchains below.

---

## Option A — MSVC (Visual Studio)

1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) (Community is free)  
   In the installer, select the **"Desktop development with C++"** workload (includes CMake and Ninja).

2. Open **x64 Native Tools Command Prompt for VS 2022** and run:

```bat
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Output: `build\bin\NoTrackSwitcher.exe`

---

## Option B — MinGW (GCC, no Visual Studio needed)

### Install tools via winget

```powershell
winget install Kitware.CMake
winget install Ninja-build.Ninja
winget install GnuWin32.Make   # optional
# GCC — pick one:
winget install MSYS2.MSYS2     # then see MSYS2 section below
# or scoop:
scoop install gcc
```

### Build with scoop GCC

```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_CXX_COMPILER="C:/Users/<you>/scoop/apps/gcc/current/bin/g++.exe" `
  -DCMAKE_RC_COMPILER="C:/Users/<you>/scoop/apps/gcc/current/bin/windres.exe"
cmake --build build
```

Replace `<you>` with your Windows username.

### Build with MSYS2

Open **MSYS2 UCRT64** shell, install the toolchain once:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
```

Then build:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Output: `build/bin/NoTrackSwitcher.exe`

> **Note on paths with spaces:** if your project folder contains spaces (e.g. `My Projects\app`),
> the MinGW resource compiler (`windres`) may fail. Move the repo to a path without spaces,
> or use MSVC instead.

---

## Running

```
build\bin\NoTrackSwitcher.exe
```

The app starts silently in the system tray. Right-click the tray icon to access settings or exit.

**Default hotkey:**
- `Ctrl+Shift+F` — fix selected text (via clipboard)
- Config file: `notrack.ini` next to the EXE (created on first run)

