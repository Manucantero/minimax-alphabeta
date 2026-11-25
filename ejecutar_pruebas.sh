#!/bin/bash

# Crear carpeta para almacenar los logs de las partidas
mkdir -p resultados_competicion
RESUMEN="resultados_competicion/resumen_partidas.txt"

echo "=========================================================" > $RESUMEN
echo "   RESUMEN COMPLETO DE PARTIDAS (ALFABETA PROFUNDIDAD 7) " >> $RESUMEN
echo "   Fecha: $(date)" >> $RESUMEN
echo "=========================================================" >> $RESUMEN
echo "" >> $RESUMEN

NINJAS=("ninja1" "ninja2" "ninja3" "ninja4")
# Solo tus dos heurísticas competitivas para evitar bloqueos
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
    
    # Extraer las líneas del tablero (9 filas)
    grep -E '^([.XO] )+([.XO])?$' "$log_file" | tail -n 9 >> $RESUMEN
    echo "" >> $RESUMEN
}

for h in "${HEURISTICAS[@]}"; do
    echo "=================================================="
    echo " Ejecutando y analizando Heurística $h..."
    echo "=================================================="
    
    for ninja in "${NINJAS[@]}"; do
        
        # 1. Tu IA juega como JUGADOR 1 (X)
        LOG_J1="resultados_competicion/H${h}_J1_vs_${ninja}.txt"
        echo " -> H${h} (J1) vs ${ninja} (J2)..."
        
        ./n_en_raya -p1 inteligente -id1 $h -p2 $ninja -d 7 -f 9 -c 9 -n 5 -nogui > "$LOG_J1" 2>&1
        extraer_informacion "$LOG_J1" "Heurística $h como J1 (X) vs $ninja (O)"
        
        # 2. Tu IA juega como JUGADOR 2 (O)
        LOG_J2="resultados_competicion/H${h}_J2_vs_${ninja}.txt"
        echo " -> ${ninja} (J1) vs H${h} (J2)..."
        
        ./n_en_raya -p1 $ninja -p2 inteligente -id2 $h -d 7 -f 9 -c 9 -n 5 -nogui > "$LOG_J2" 2>&1
        extraer_informacion "$LOG_J2" "$ninja (X) vs Heurística $h como J2 (O)"
        
        echo "--------------------------------------------------"
    done
done

echo ""
echo "¡Todo listo! Las simulaciones han terminado (16 partidas en total)."
echo "He generado las tablas y los resultados en: $RESUMEN"
