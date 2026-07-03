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
repeats=100

for c in "${c_values[@]}"; do
    for c_real in "${c_real_values[@]}"; do
        output="results_c${c}_real${c_real}.txt"
        echo "Обработка c=$c, c_real=$c_real, сохраняю в $output"

        > "$output"

        for ((run=1; run<=repeats; run++)); do
            echo "=== Run $run ===" >> "$output"
            {
                echo "$c"
                echo "$c_real"
                for f in "${files[@]}"; do
                    echo "$f"
                done
            } | ./a.out >> "$output" 2>&1
            echo "" >> "$output"
        done

        echo "Закончил $output"
    done
done

echo "Все эксперименты завершены."