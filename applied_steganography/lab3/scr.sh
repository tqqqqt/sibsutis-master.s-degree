#!/bin/bash

for i in {1..20}; do
      printf "%s\n" "../../bmp/$i.bmp" | ./a.out
      #printf "%s\n%s\n" "../../bmp/$i.bmp" "new_file.bmp" | ./psnr
      cp decode.txt decode1-$i.txt
done

echo "========="
echo "========="

for i in {1..9}; do
       printf "%s\n" "../../data/medical/00$i.bmp" | ./a.out
       #printf "%s\n%s\n" "../../data/medical/00$i.bmp" "new_file.bmp" | ./psnr
       cp decode.txt decode2-$i.txt
done
for i in {10..20}; do
       printf "%s\n" "../../data/medical/0$i.bmp" | ./a.out
       #printf "%s\n%s\n" "../../data/medical/0$i.bmp" "new_file.bmp" | ./psnr
       cp decode.txt decode2-$i.txt
done

echo "========="
echo "========="

for i in {1..9}; do
       printf "%s\n" "../../data/textures/00$i.bmp" | ./a.out
       #printf "%s\n%s\n" "../../data/textures/00$i.bmp" "new_file.bmp" | ./psnr
       cp decode.txt decode1-$i.txt
done
for i in {10..20}; do
       printf "%s\n" "../../data/textures/0$i.bmp" | ./a.out
       #printf "%s\n%s\n" "../../data/textures/0$i.bmp" "new_file.bmp" | ./psnr
       cp decode.txt decode1-$i.txt
done
