import csv
import random
import os
import re

def get_last_index():
    max_index = 0
    # Pola regex untuk membaca file dengan format matrix_X.csv atau matrix_X_N.csv
    pattern = re.compile(r'^matrix_(\d+)(?:_\d+)?\.csv$')
    
    # Memeriksa semua file di direktori saat ini
    for filename in os.listdir('.'):
        match = pattern.match(filename)
        if match:
            # Mengambil grup pertama (X) yang merupakan indeks file
            index = int(match.group(1))
            if index > max_index:
                max_index = index
                
    return max_index

def generate_csv_matrices():
    print("--- Generator Matriks CSV ---")
    try:
        n = int(input("Masukkan ukuran matriks (N x N): "))
        num_files = int(input("Masukkan jumlah file matriks yang dibuat: "))
    except ValueError:
        print("Error: Harap masukkan angka yang valid.")
        return

    # Mendapatkan indeks terakhir dan memulai penomoran dari selanjutnya
    start_index = get_last_index() + 1

    for i in range(num_files):
        current_index = start_index + i
        # Format baru: matrix_[indeks]_[ukuran].csv
        filename = f"matrix_{current_index}_{n}.csv"
        
        with open(filename, mode='w', newline='') as file:
            writer = csv.writer(file)
            for _ in range(n):
                row = [random.randint(1, 1000) for _ in range(n)]
                writer.writerow(row)
        print(f"[*] File {filename} berhasil dibuat.")

if __name__ == "__main__":
    generate_csv_matrices()