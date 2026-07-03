#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

error_exit() {
    echo -e "${RED}Ошибка: $1${NC}" >&2
    exit 1
}

warn() {
    echo -e "${RED}Предупреждение: $1${NC}" >&2
}

process_file() {
    local algo=$1
    local file=$2
    local first=$3

    if [[ "$algo" =~ ^[12]$ ]] && [[ "$file" =~ [[:space:]] ]]; then
        warn "Путь '$file' содержит пробелы. Без кавычек это может вызвать ошибку."
    fi

    case $algo in
        1|2)
            local outfile="p${algo}.txt"
            if [ "$first" -eq 1 ]; then
                ./a.out $algo $file > "$outfile" 2>&1
            else
                ./a.out $algo $file >> "$outfile" 2>&1
            fi
            ;;
        3)
            local outfile="p3.txt"
            if [ ! -f "main.m" ]; then
                error_exit "Файл main.m не найден в текущей директории"
            fi
            cp main.m main.m.bak
            escaped_file=$(echo "$file" | sed "s/'/'\\\\''/g")
            sed -i "1i X=imread('$escaped_file');" main.m
            if [ "$first" -eq 1 ]; then
                octave-cli main.m > "$outfile" 2>&1
            else
                octave-cli main.m >> "$outfile" 2>&1
            fi
            mv main.m.bak main.m
            ;;
        *)
            error_exit "Неизвестный алгоритм: $algo"
            ;;
    esac

    cat "$outfile"
}

echo "Какой алгоритм запустить (1, 2 или 3)?"
read algorithm
if [[ ! "$algorithm" =~ ^[123]$ ]]; then
    error_exit "Алгоритм должен быть 1, 2 или 3"
fi

echo "Укажите путь к файлу или папке с .bmp файлами:"
read path

if [ ! -e "$path" ]; then
    error_exit "Путь '$path' не существует"
fi

files=()
if [ -d "$path" ]; then
    shopt -s nullglob
    for f in "$path"/*.bmp; do
        files+=("$f")
    done
    shopt -u nullglob
    if [ ${#files[@]} -eq 0 ]; then
        error_exit "В папке '$path' нет .bmp файлов"
    fi
else
    files=("$path")
fi

first=1
for file in "${files[@]}"; do
    echo "Обработка файла: $file"
    process_file "$algorithm" "$file" "$first"
    first=0
done

echo -e "${GREEN}Готово. Результаты сохранены в p${algorithm}.txt${NC}"