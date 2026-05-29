[🇷🇺 Русский](#русский) | [🇬🇧 English](#english)

---

## Русский

# NoTrack Switcher

Утилита для Windows исправляющая текст, набранный не в той раскладке.

### Как использовать

1. Выдели неправильно набранный текст
2. Нажми `Ctrl+Shift+F`
3. Текст будет перконвертирован и вставлен в нужной раскладке

### Установка

Скачай `NoTrackSwitcher.exe` из [Releases](../../releases) и запусти. Установка не нужна — приложение работает из любой папки.

### Настройки

Двойной клик по иконке в трее открывает настройки:

- **Start with Windows** — автозапуск при входе в систему
- **Fix selected text** — горячая клавиша (по умолчанию `Ctrl+Shift+F`)

Правый клик по иконке позволяет временно отключить приложение или выйти.

### Сборка из исходников

Требования: Windows 10/11 x64, CMake 3.20+, Ninja, Visual Studio 2022.

```bat
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Результат: `build\bin\NoTrackSwitcher.exe`

Подробнее — в [BUILD.md](BUILD.md).

### Сравнение с Punto Switcher (одна из популярнейших утилит для исправления раскладки)

| | NoTrack Switcher | Punto Switcher |
|---|---|---|
| Установка | Не нужна | Установщик |
| Телеметрия | Нет | Да (Яндекс) |
| Открытый код | Да | Нет |

NoTrack Switcher делает одно — исправляет выделенный текст по горячей клавише. Никаких фоновых процессов, никакой телеметрии.

---

## English

# NoTrack Switcher

A Windows utility that fixes text typed in the wrong keyboard layout.

### How to use

1. Select the mistyped text
2. Press `Ctrl+Shift+F`
3. The text will be converted and pasted in the correct layout

### Installation

Download `NoTrackSwitcher.exe` from [Releases](../../releases) and run it. No installation needed — works from any folder.

### Settings

Double-click the tray icon to open settings:

- **Start with Windows** — launch automatically on login
- **Fix selected text** — hotkey (default: `Ctrl+Shift+F`)

Right-click the tray icon to temporarily disable the app or exit.

### Building from source

Requirements: Windows 10/11 x64, CMake 3.20+, Ninja, Visual Studio 2022.

```bat
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Output: `build\bin\NoTrackSwitcher.exe`

See [BUILD.md](BUILD.md) for more options (MinGW, MSYS2).

### Comparison with Punto Switcher (one of the most popular layout-fix utilities)

| | NoTrack Switcher | Punto Switcher |
|---|---|---|
| Installation | Not required | Installer |
| Telemetry | None | Yes (Yandex) |
| Open source | Yes | No |

NoTrack Switcher does one thing — fixes selected text with a hotkey. No background processes, no telemetry.
