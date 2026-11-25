# N-in-a-Row Adversarial AI — Minimax & Alpha-Beta Pruning

Game-playing artificial intelligence agent developed in C++17 for the generalized **N-in-a-Row (`n_en_raya`)** board game (configured on a 9×9 grid with a 5-in-a-row win condition), competing against automated benchmark bots (**Ninja agents**) using adversarial search algorithms and custom heuristic evaluation functions.

## Repository Structure & My Contributions

- **`/Comportamiento_Agente` (`AgenteEstudiante.cpp` / `.h` — Implemented by me):**
  - Adversarial search decision-making algorithms: **Minimax** and **Alpha-Beta pruning**.
  - Two custom domain-specific heuristic evaluation functions (`id 1` and `id 2`) weighing alignment threats, board control, and defensive blocking.
- **`/src` & `/include`:** Base game engine (`Tablero`, `ControladorJuego`), OpenGL/GLUT graphical interface (`GUI`), and opponent framework (`AgenteNinja`).
- **`/Memoria`:** Technical report analyzing heuristic design, search-tree pruning efficiency, depth-vs-time trade-offs, and experimental results.
- **Automated Benchmark Suite (Implemented by me):**
  - `ejecutar_pruebas.sh`: Runs 16 headless tournament matches using **Alpha-Beta pruning (depth 7)** for both heuristics as Player 1 and Player 2 against `ninja1`–`ninja4`, parsing final board states and win rates into `resultados_competicion/resumen_partidas.txt`.
  - `ejecutar_pruebas_minimax.sh`: Runs the equivalent 16-match benchmark suite using **Minimax (depth 4)** into `resultados_minimax/resumen_minimax_d4.txt`.
- **Build & Environment Files:** `CMakeLists.txt`, `install.sh`, `ninja_bridge.py`, and `tablero_inicial.txt`.

## Tech Stack

- **Languages:** C++17 (`-O3 -march=native`), Bash, Python 3 (Ninja evaluation bridge)
- **Libraries & Build System:** CMake (3.10+), OpenGL, GLUT (`freeglut`), POSIX Threads (`pthread`)
- **Core Concepts:** Adversarial Search, Game Trees, Minimax, Alpha-Beta Pruning, Heuristic Evaluation

## Prerequisites (Linux / Ubuntu)

Install the compiler, CMake, and OpenGL/GLUT development libraries (or run `./install.sh` if configured for your distribution):

```bash
sudo apt update
sudo apt install build-essential cmake libgl1-mesa-dev freeglut3-dev
```

## How to Build & Run

1. Clone the repository:
   ```bash
   git clone [https://github.com/Manucantero/minimax-alphabeta-solver.git](https://github.com/Manucantero/minimax-alphabeta-solver.git)
   cd minimax-alphabeta-solver
   ```

2. Build both targets (multithreaded `n_en_raya` and single-threaded `n_en_rayaSH`):
   ```bash
   mkdir -p build && cd build
   cmake ..
   make
   cp n_en_raya n_en_rayaSH ..
   cd ..
   ```

3. Run a single match or execute the automated benchmark suites:
   ```bash
   # Single GUI match: Heuristic 1 vs ninja1 (Alpha-Beta depth 7, 9x9 board, 5-in-a-row)
   ./n_en_raya -p1 inteligente -id1 1 -p2 ninja1 -d 7 -f 9 -c 9 -n 5

   # Run all 16 automated Alpha-Beta (d=7) benchmark matches
   chmod +x ejecutar_pruebas.sh
   ./ejecutar_pruebas.sh

   # Run all 16 automated Minimax (d=4) benchmark matches
   chmod +x ejecutar_pruebas_minimax.sh
   ./ejecutar_pruebas_minimax.sh
   ```
