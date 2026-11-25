#!/bin/bash

# Crear carpeta para almacenar los logs de Minimax
mkdir -p resultados_minimax
RESUMEN="resultados_minimax/resumen_minimax_d4.txt"

echo "=========================================================" > $RESUMEN
echo "   RESUMEN COMPLETO DE PARTIDAS (MINIMAX PROFUNDIDAD 4)  " >> $RESUMEN
echo "   Fecha: $(date)" >> $RESUMEN
echo "=========================================================" >> $RESUMEN
echo "" >> $RESUMEN

NINJAS=("ninja1" "ninja2" "ninja3" "ninja4")
HEURISTICAS=(1 2)

# Función interna para extraer el último tablero y el resultado
extraer_informacion() {
    local log_file=$1
    local titulo=$2
    
    echo "---------------------------------------------------------" >> $RESUMEN
    echo " CONFIGURACIÓN: $titulo" >> $RESUMEN
    echo "---------------------------------------------------------" >> $RESUMEN
    
    # Extraer el resultado final
    local resultado=$(grep -E "PARTIDA FINALIZADA|Gana|Empate" "$log_file")
    if [ -z "$resultado" ]; then
        resultado="Resultado: No finalizado correctamente o error."
    fi
    echo "$resultado" >> $RESUMEN
    echo "" >> $RESUMEN
    echo "Tablero en el estado final:" >> $RESUMEN
    
    # Extraer las últimas líneas que tengan formato de tablero (9 filas)
    grep -E '^([.XO] )+([.XO])?$' "$log_file" | tail -n 9 >> $RESUMEN
    echo "" >> $RESUMEN
}

for h in "${HEURISTICAS[@]}"; do
    echo "=================================================="
    echo " Ejecutando Heurística $h con MINIMAX (d=4)..."
    echo "=================================================="
    
    for ninja in "${NINJAS[@]}"; do
        
        # 1. Tu IA juega como JUGADOR 1 (X)
        LOG_J1="resultados_minimax/H${h}_J1_vs_${ninja}.txt"
        echo " -> Ejecutando Minimax: H${h} (J1) vs ${ninja} (J2)..."
        
        # Añadido -nogui para desactivar la interfaz gráfica
        ./n_en_raya -p1 inteligente -id1 $h -p2 $ninja -d 4 -f 9 -c 9 -n 5 -nogui > "$LOG_J1" 2>&1
        extraer_informacion "$LOG_J1" "Heurística $h como J1 (X) vs $ninja (O) [Minimax d=4]"
        
        # 2. Tu IA juega como JUGADOR 2 (O)
        LOG_J2="resultados_minimax/H${h}_J2_vs_${ninja}.txt"
        echo " -> Ejecutando Minimax: ${ninja} (J1) vs H${h} (J2)..."
        
        # Añadido -nogui para desactivar la interfaz gráfica
        ./n_en_raya -p1 $ninja -p2 inteligente -id2 $h -d 4 -f 9 -c 9 -n 5 -nogui > "$LOG_J2" 2>&1
        extraer_informacion "$LOG_J2" "$ninja (X) vs Heurística $h como J2 (O) [Minimax d=4]"
        
        echo "--------------------------------------------------"
    done
done

echo ""
echo "¡Todo listo! Las simulaciones de Minimax han terminado."
echo "He generado las tablas y los resultados en: $RESUMEN"
