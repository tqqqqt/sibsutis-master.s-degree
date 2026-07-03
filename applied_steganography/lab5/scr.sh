#!/bin/bash

files=(
    "../../bmp/1.bmp"
    "../../bmp/2.bmp"
    "../../bmp/3.bmp"
    "../../bmp/4.bmp"
    "../../bmp/5.bmp"
    "../../bmp/6.bmp"
    "../../bmp/7.bmp"
    "../../bmp/8.bmp"
    "../../bmp/9.bmp"
    "../../bmp/10.bmp"
)

c_values=(2 3 4 5)
c_real_values=(2 3 4 5)

for c in "${c_values[@]}"; do
    #for c_real in "${c_values[@]}"; do
        echo "Запуск для c=$c, c_real=$c"
        
        output="output_c${c}_real${c}.txt"

        {
            echo "$c"
            echo "$c"
            for file in "${files[@]}"; do
                echo "$file"
            done
        } | ./a.out > "$output" 2>&1
    #done
done

echo "Все эксперименты завершены."