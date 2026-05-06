# ♟️ Chess Game in C++ with Raylib

A fully playable chess game written in modern C++ using the graphics library Raylib.

This project includes:

- Full chess movement logic
- Check / Checkmate / Stalemate detection
- Castling
- Pawn promotion
- Move history
- Undo / Redo system
- Save & Load chess logs
- Rotating board view
- Captured pieces display
- Graphical user interface with textures

---

# 📸 Features

## ✅ Implemented

- ♔ Full chess rules
- ♜ Castling (short + long)
- ♟ Pawn promotion
- ⚠ Check detection
- 🏁 Checkmate detection
- 🤝 Stalemate detection
- 🔄 Undo / Redo moves
- 💾 Save game logs to `.txt`
- 📂 Load logs from files
- 🖱 Click-based movement system
- 🔃 Automatic board flipping
- 📜 Scrollable move history
- 🎨 Piece rendering using spritesheets

---

# 🛠 Technologies

- C++23
- Raylib
- tinyfiledialogs
- STL (`vector`, `array`, `stack`, `list`, `filesystem`, `chrono`, etc.)

---

# 📂 Project Structure

```txt
project/
│
├── images/
│   ├── chesspieces.png
│   └── chess_icon.png
│
├── Logs/
│
├── external/
│   └── tinyfiledialogs/
│       ├── tinyfiledialogs.c
│       └── tinyfiledialogs.h
│
├── main.cpp
├── CMakeLists.txt
└── README.md
```

---

# ▶️ Build & Run

## Requirements

Install:

- C++23 compiler
- Raylib
- CMake

---

## Example CMake

```cmake
cmake_minimum_required(VERSION 3.20)
project(Chess)

set(CMAKE_CXX_STANDARD 23)

find_package(raylib REQUIRED)

add_executable(Chess
    main.cpp
    external/tinyfiledialogs/tinyfiledialogs.c
)

target_include_directories(Chess PRIVATE
    external/tinyfiledialogs
)

target_link_libraries(Chess raylib)
```

---

# 🎮 Controls

| Action | Input |
|---|---|
| Select piece | Left mouse click |
| Move piece | Left mouse click |
| Scroll move history | Mouse wheel |
| Restart game | Restart button |
| Save game log | Save Log button |
| Load game log | Load Log button |
| Undo move | `<-` button |
| Redo move | `->` button |

---

# 💾 Save System

The game can export move histories into text files:

```txt
W_PAWN -> E4
B_PAWN -> E5
W_KNIGHT -> F3
```

Saved logs include:
- timestamps
- game result
- captures
- castling

Example:

```txt
logs_2026-05-06_15-20-30_WHITE_WON.txt
```

---

# ♟ Chess Rules Supported

| Rule | Supported |
|---|---|
| Normal movement | ✅ |
| Capturing | ✅ |
| Castling | ✅ |
| Pawn promotion | ✅ |
| Check | ✅ |
| Checkmate | ✅ |
| Stalemate | ✅ |

---

# 📷 Assets

The project uses:
- a chess spritesheet (`chesspieces.png`)
- a custom application icon (`chess_icon.png`)

---

# 🚀 Future Ideas

- En passant
- AI opponent
- Online multiplayer
- Sound effects
- Timers / chess clock
- PGN export
- Better animations

---

# 📄 License

This project is open source and free to use.

---

# 👨‍💻 Author

Created by a C++ chess enthusiast using Raylib.
