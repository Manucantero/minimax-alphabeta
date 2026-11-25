#include "AgenteEstudiante.hpp"
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

AgenteEstudiante::AgenteEstudiante(int id, int profundidadMax, double tiempoMax, int numHeuristica, ModoJuego modo) 
    : id(id), profundidadMax(profundidadMax), tiempoMaxSegundos(tiempoMax), numHeuristica(numHeuristica), modo(modo), abortarBanda(false) {
    nodosVisitados = 0;
}

bool AgenteEstudiante::tieneLimiteDeTiempo() const {
    return modo != ModoJuego::STATUS;
}

std::pair<int, int> AgenteEstudiante::think(const Tablero& tablero) {
    std::pair<int, int> mejor;
    nodosVisitados = 0;
    abortarBanda = false;
    inicioBusqueda = std::chrono::steady_clock::now();

    switch (modo)
    {
    case ModoJuego::ALEATORIO:
        return JuegaAleatorio(tablero);
        break;
    
    case ModoJuego::STATUS:
        Status(tablero, mejor);
        return mejor;
        break;    

    case ModoJuego::MINIMAX:
        minimax(tablero, 0, profundidadMax, mejor);
        return mejor;
        break; 

    case ModoJuego::INTELIGENTE:
        return JuegaInteligente(tablero);   
        break;
    }
        
    return {-1, -1};
}


/**
 * @brief Compara dos tableros para identificar cuál ha sido el movimiento realizado.
 * @param padre Estado inicial del tablero.
 * @param hijo Estado resultante tras un movimiento.
 * @return Un par (fila, columna) con la posición de la nueva pieza.
 */
std::pair<int, int> SacarMovimiento(const Tablero& padre, const Tablero &hijo){
    for(int f=0; f<padre.getFilas(); ++f)
        for(int c=0; c<padre.getColumnas(); ++c)
            if (padre.getCelda(f,c) == 0 && hijo.getCelda(f,c) != 0) 
                return {f, c};
    return {-1, -1};
}

/**
 * @brief Implementa un agente que juega de forma totalmente aleatoria.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida al azar.
 */
std::pair<int, int> AgenteEstudiante::JuegaAleatorio(const Tablero& tablero) {

    // Calculo los tableros descendientes de tablero
    auto sucesores = tablero.getSucesores();

    // Si no tiene descendientes, paso el turno
    if (sucesores.empty()) return {-1, -1};

    // Elijo aleatoriamente uno de los descendientes
    int elegido = rand() % sucesores.size();

    // Saco el movimiento realizado comparando el tablero original con el elegido.
    std::pair<int,int> Mov = SacarMovimiento(tablero, sucesores[elegido]);

    return Mov;
}


/**
 * @brief Algoritmo de resolución completa para estados de final de juego.
 * Determina si una posición está matemáticamente ganada, perdida o empatada.
 * @param tablero Estado a evaluar.
 * @param Mov [Salida] La jugada óptima encontrada.
 * @return Resultado del análisis (VICTORIA, DERROTA o EMPATE).
 */
AgenteEstudiante::Resultado AgenteEstudiante::Status(const Tablero &tablero, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    /* ============== Empieza a partir de aquí tu implementación  =============== */

   // 1. CONDICIONES DE PARADA (Casos base)
    int ganador = tablero.comprobarGanador();
    if (ganador == id) return Resultado::VICTORIA;
    if (ganador != 0 && ganador != -1) return Resultado::DERROTA; // El ganador es el oponente
    if (ganador == -1) return Resultado::EMPATE; // Empate técnico detectado por el motor

    // 2. GENERACIÓN DE HIJOS
    auto sucesores = tablero.getSucesoresConMovimientos();

    // 3. RESOLUCIÓN DE ESTANCAMIENTOS (Tie-break)
    // Si no quedan movimientos legales, aplicamos la regla de recuento por puntos
    if (sucesores.empty()) {
        int ganadorDesempate = tablero.getGanadorDesempate();
        if (ganadorDesempate == id) return Resultado::VICTORIA;
        if (ganadorDesempate != 0 && ganadorDesempate != -1) return Resultado::DERROTA;
        return Resultado::EMPATE;
    }

    // 4. RECURSIÓN MINIMAX PARA STATUS
    bool esMax = (tablero.getJugadorTurno() == id);

    // --- TURNO DE NUESTRO AGENTE (NODO MAX) ---
    // Buscamos maximizar el resultado (VICTORIA > EMPATE > DERROTA)
    if (esMax) {
        Resultado mejor_res = Resultado::DERROTA; 
        Mov = sucesores[0].second; // Movimiento por defecto de seguridad

        for (const auto& sucesor : sucesores) {
            std::pair<int, int> dummyMov; // Variable crucial para aislar la memoria en la recursión
            Resultado res = Status(sucesor.first, dummyMov);

            // Si encontramos un camino ganador, es la jugada óptima. Podamos y retornamos.
            if (res == Resultado::VICTORIA) {
                mejor_res = Resultado::VICTORIA;
                Mov = sucesor.second;
                break; 
            }
            // Si empatamos y antes perdíamos, guardamos este movimiento por si no hay victorias.
            else if (res == Resultado::EMPATE && mejor_res == Resultado::DERROTA) {
                mejor_res = Resultado::EMPATE;
                Mov = sucesor.second;
            }
        }
        return mejor_res;
    } 
    // --- TURNO DEL OPONENTE (NODO MIN) ---
    // El rival busca minimizar nuestro resultado (DERROTA > EMPATE > VICTORIA)
    else {
        Resultado peor_res = Resultado::VICTORIA; 
        Mov = sucesores[0].second; // Movimiento por defecto para el adversario

        for (const auto& sucesor : sucesores) {
            std::pair<int, int> dummyMov;
            Resultado res = Status(sucesor.first, dummyMov);

            // Si el rival puede forzar nuestra derrota lo hará (Estoy cooked). Podamos.
            if (res == Resultado::DERROTA) {
                peor_res = Resultado::DERROTA;
                Mov = sucesor.second;
                break;
            }
            // Si el rival puede forzar un empate, lo preferirá a dejarnos ganar.
            else if (res == Resultado::EMPATE && peor_res == Resultado::VICTORIA) {
                peor_res = Resultado::EMPATE;
                Mov = sucesor.second;
            }
        }
        return peor_res;
    }
}


/**
 * @brief Implementación del algoritmo Minimax clásico.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado.
 */
double AgenteEstudiante::minimax(const Tablero &tablero, int profundidad, int prof_Max, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    if (abortarBanda) return 0;
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
        abortarBanda = true; return 0;
    }
    /* ============== Empieza a partir de aquí tu implementación  =============== */

    // 1. CONDICIONES DE PARADA ESTRICTAS (Nodos Terminales)
    // Primero comprobamos si el juego ya ha terminado de forma natural.
    // Usamos las constantes GANAR (1e12) y PERDER (-1e12) definidas en el .hpp
    int ganador = tablero.comprobarGanador();
    if (ganador == id) return GANAR;
    if (ganador != 0 && ganador != -1) return PERDER;
    if (ganador == -1) return 0; // Empate vale 0

    auto sucesores = tablero.getSucesoresConMovimientos();
    if (sucesores.empty()) {
        int ganadorDesempate = tablero.getGanadorDesempate();
        if (ganadorDesempate == id) return GANAR;
        if (ganadorDesempate != 0 && ganadorDesempate != -1) return PERDER;
        return 0; // Empate técnico
    }

    // Límite de profundidad superado -> Evaluamos con la heurística
    if (profundidad == prof_Max) {
        return heuristica(tablero);
    }

    // 2. LÓGICA MINIMAX
    bool esMax = (tablero.getJugadorTurno() == id);
    

    // --- TURNO DE NUESTRO AGENTE (NODO MAX) ---
    
    if (esMax) {
        double maxEval = MenosInfinito; // Partimos del peor valor posible 
        Mov = sucesores[0].second;      // Movimiento por defecto 

        for (const auto& sucesor : sucesores) {
            std::pair<int, int> dummyMov; // Aislamos la variable de movimiento para la recursión
            // Llamada recursiva incrementando la profundidad
            double eval = minimax(sucesor.first, profundidad + 1, prof_Max, dummyMov);
            
            // Buscamos maximizar la puntuación
            if (eval > maxEval) {
                maxEval = eval;
                Mov = sucesor.second; // Actualizamos la mejor jugada en esta profundidad
            }
        }
        return maxEval;
    } 
    // --- TURNO DEL OPONENTE (NODO MIN) ---
    else {
        double minEval = MasInfinito; // Partimos del valor más alto posible (1e15)
        Mov = sucesores[0].second;

        for (const auto& sucesor : sucesores) {
            std::pair<int, int> dummyMov;
            double eval = minimax(sucesor.first, profundidad + 1, prof_Max, dummyMov);
            
            // El oponente busca minimizar nuestra puntuación
            if (eval < minEval) {
                minEval = eval;
                Mov = sucesor.second;
            }
        }
        return minEval;
    }
}


/**
 * @brief Punto de entrada para el juego inteligente.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida por el algoritmo de búsqueda.
 */
std::pair<int, int> AgenteEstudiante::JuegaInteligente(const Tablero& tablero) {
    std::pair<int,int> Mov;

    double valor = alfaBeta(tablero, 0, profundidadMax, MenosInfinito, MasInfinito, Mov);
    std::cout << "Valor Minimax: " << valor << "\tJugada: (" << Mov.first << ", " << Mov.second << ")\n";
    return Mov;
}




/**
 * @brief Implementación del algoritmo Minimax con Poda Alfa-Beta.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param alfa Valor mínimo garantizado para el jugador MAX.
 * @param beta Valor máximo garantizado para el jugador MIN.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado tras la poda.
 */
double AgenteEstudiante::alfaBeta(const Tablero &tablero, int profundidad, int prof_Max, double alfa, double beta, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    if (abortarBanda) return 0;
    
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
        abortarBanda = true;
        return 0;
    }
// 1. CONDICIONES DE PARADA ESTRICTAS (Nodos Terminales)
    int ganador = tablero.comprobarGanador();
    if (ganador == id) return GANAR;
    if (ganador != 0 && ganador != -1) return PERDER;
    if (ganador == -1) return 0; // Empate

    auto sucesores = tablero.getSucesoresConMovimientos();
    if (sucesores.empty()) {
        int ganadorDesempate = tablero.getGanadorDesempate();
        if (ganadorDesempate == id) return GANAR;
        if (ganadorDesempate != 0 && ganadorDesempate != -1) return PERDER;
        return 0;
    }

    // 2. CONDICIÓN DE PARADA POR PROFUNDIDAD
    if (profundidad == prof_Max) {
        return heuristica(tablero);
    }

    // 3. LÓGICA ALFA-BETA
    bool esMax = (tablero.getJugadorTurno() == id);

    // --- TURNO DE NUESTRO AGENTE (NODO MAX) ---
    if (esMax) {
        double maxEval = MenosInfinito;
        Mov = sucesores[0].second;

        for (const auto& sucesor : sucesores) {
            std::pair<int, int> dummyMov;
            // Llamada recursiva pasando alfa y beta
            double eval = alfaBeta(sucesor.first, profundidad + 1, prof_Max, alfa, beta, dummyMov);
            
            if (eval > maxEval) {
                maxEval = eval;
                Mov = sucesor.second;
            }
            
            // Actualizamos alfa (la mejor opción garantizada para MAX hasta ahora)
            alfa = std::max(alfa, eval);
            
            // CONDICIÓN DE PODA
            if (beta <= alfa) {
                break; // El oponente (MIN) evitará esta rama, no seguimos explorando
            }
        }
        return maxEval;
    } 
    // --- TURNO DEL OPONENTE (NODO MIN) ---
    else {
        double minEval = MasInfinito;
        Mov = sucesores[0].second;

        for (const auto& sucesor : sucesores) {
            std::pair<int, int> dummyMov;
            double eval = alfaBeta(sucesor.first, profundidad + 1, prof_Max, alfa, beta, dummyMov);
            
            if (eval < minEval) {
                minEval = eval;
                Mov = sucesor.second;
            }
            
            // Actualizamos Beta (la mejor opción garantizada para MIN hasta ahora)
            beta = std::min(beta, eval);
            
            // CONDICIÓN DE PODA
            if (beta <= alfa) {
                break; // Nosotros (MAX) evitaremos esta rama, no seguimos explorando
            }
        }
        return minEval;
    }
}

/**
 * @brief Función heurística para evaluar la calidad de un tablero.
 * @param tablero Estado a evaluar.
 * @return Puntuación numérica (positiva para ventaja de J1, negativa para J2).
 */
double AgenteEstudiante::heuristica(const Tablero& tablero) {
    switch(numHeuristica) {
        case 0: return heuristicaPrueba(tablero);
                break;
        case 1: return heuristica1(tablero);
                break;
        case 2: return heuristica2(tablero);
                break;
        default: return heuristica1(tablero);
    }
}

double AgenteEstudiante::heuristicaPrueba(const Tablero& tablero) {
    // n es el número de fichas en línea para ganar.
    int n = tablero.getNParaGanar();
    int oponente = (id == 1) ? 2 : 1;
    double score_positivo = 0;

    double score_negativo = 0;

    for (int f=0; f< tablero.getFilas(); f++ ){
        for (int c = 0; c< tablero.getColumnas(); c++){
            if (tablero.getCelda(f,c) != 0 ){
                int valor = tablero.getFilas()-abs(f-(tablero.getFilas()/2)) + tablero.getColumnas()-abs(c-(tablero.getColumnas()/2)); 
                if (tablero.getCelda(f,c) == id){
                  score_positivo += valor;
                 }
                else {
                  score_negativo += valor;
                }
            }
        }
    }

   
    return score_positivo - score_negativo;
}

/**
 * @brief HEURÍSTICA 1: Control del centro + Conectividad básica
 * Mi idea aquí es mejorar la heurística de prueba que nos dieron.
 * Además de valorar más las casillas del centro (que dan más juego), añado un bonus 
 * si mis fichas están juntas. En el N en raya las fichas sueltas no ganan, así que 
 * si una ficha tiene una compañera al lado o abajo, le doy más valor para fomentar 
 * que el algoritmo agrupe fichas y cree líneas de ataque.
 */
double AgenteEstudiante::heuristica1(const Tablero& tablero) {
    int oponente = (id == 1) ? 2 : 1;
    double puntuacion = 0;
    
    // Tamaño del tablero
    int filas = tablero.getFilas();
    int columnas = tablero.getColumnas();
    
    // Posicion del centro
    int centroF = filas / 2;
    int centroC = columnas / 2;

    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < columnas; c++) {
            int jugador = tablero.getCelda(f, c);
            
            // Si la casilla no está vacía, la evaluamos
            if (jugador != 0) {
                
                // 1. Calculo la cercanía al centro (fórmula de la heurística de prueba)
                double valorCasilla = (filas - abs(f - centroF)) + (columnas - abs(c - centroC));
                double bonusConectividad = 0;
                
                // 2. Conectividad Vertical (Miro si hay una ficha igual abajo)
                if (f + 1 < filas && tablero.getCelda(f + 1, c) == jugador) {
                    bonusConectividad += 4.0;
                }
                // 3. Conectividad Horizontal (Miro si hay una ficha igual a la derecha)
                if (c + 1 < columnas && tablero.getCelda(f, c + 1) == jugador) {
                    bonusConectividad += 4.0;
                }
                // 4. Diagonal Principal (Miro abajo-derecha)
                if (f + 1 < filas && c + 1 < columnas && tablero.getCelda(f + 1, c + 1) == jugador) {
                    bonusConectividad += 5.0; // Las diagonales valen un poco más por ser más difíciles de ver
                }
                // 5. Diagonal Secundaria (Miro abajo-izquierda)
                if (f + 1 < filas && c - 1 >= 0 && tablero.getCelda(f + 1, c - 1) == jugador) {
                    bonusConectividad += 5.0;
                }

                // Si la ficha es mía sumo los puntos, si es del rival los resto para contrarrestarlo
                if (jugador == id) {
                    puntuacion += (valorCasilla + bonusConectividad);
                } else {
                    puntuacion -= (valorCasilla + bonusConectividad);
                }
            }
        }
    }
    return puntuacion;
}

/**
 * @brief HEURÍSTICA 2: Movilidad y potencial de expansión (Huecos libres)
 * Para esta segunda estrategia paso completamente del centro del tablero y me enfoco 
 * en la "libertad" de las fichas. Una ficha que está completamente rodeada o bloqueada 
 * contra una pared no sirve para hacer un N en raya. Por lo tanto, por cada ficha del 
 * tablero, cuento cuántos de sus 4 vecinos directos (arriba, abajo, izquierda, derecha) 
 * están vacíos (valen 0). Cuantos más huecos libres tenga alrededor, más potencial tiene 
 * esa ficha para expandirse en turnos futuros y más puntos le otorgo.
 */
double AgenteEstudiante::heuristica2(const Tablero& tablero) {
    int oponente = (id == 1) ? 2 : 1;
    double puntuacion = 0;
    
    int filas = tablero.getFilas();
    int columnas = tablero.getColumnas();

    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < columnas; c++) {
            int jugador = tablero.getCelda(f, c);
            
            if (jugador != 0) {
                // Contador para ver cuántos huecos vacíos rodean a la ficha actual
                int espaciosLibres = 0;
                
                // Comprobaciones de seguridad para no salirme de los límites de la matriz
                if (f > 0 && tablero.getCelda(f - 1, c) == 0) espaciosLibres++;              // Vecino Arriba
                if (f < filas - 1 && tablero.getCelda(f + 1, c) == 0) espaciosLibres++;      // Vecino Abajo
                if (c > 0 && tablero.getCelda(f, c - 1) == 0) espaciosLibres++;              // Vecino Izquierda
                if (c < columnas - 1 && tablero.getCelda(f, c + 1) == 0) espaciosLibres++;  // Vecino Derecha

                // Una ficha base vale 1 punto. Cada casilla vacía a su alrededor le suma 2 puntos más.
                double valorFicha = 1.0 + (espaciosLibres * 2.0);

                // Sumo si es mía, resto si es del oponente
                if (jugador == id) {
                    puntuacion += valorFicha;
                } else {
                    puntuacion -= valorFicha;
                }
            }
        }
    }
    return puntuacion;
}

