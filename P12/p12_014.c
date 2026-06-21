#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int rank, size;
    long long N = 0;
    int expected_procs = 0;

    // Inisialisasi environment MPI
    MPI_Init(&argc, &argv);
    
    // Mendapatkan rank (ID) proses saat ini
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    // Mendapatkan jumlah total proses yang berjalan
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3) {
        if (rank == 0) {
            printf("Cara penggunaan: %s <batas_akhir> <jumlah_proses>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    N = atoll(argv[1]);
    expected_procs = atoi(argv[2]);

    if (rank == 0 && size != expected_procs) {
        printf("Peringatan: Jumlah pemroses mpiexec (-np %d) berbeda dengan argumen Anda (%d).\n", size, expected_procs);
    }

    // Splitting Job: Pembagian beban kerja per pekerja
    long long chunk_size = N / size;
    long long start_val = rank * chunk_size + 1;
    long long end_val = (rank == size - 1) ? N : (rank + 1) * chunk_size;

    // Masing-masing pekerja menghitung bagiannya
    long long local_sum = 0;
    for (long long i = start_val; i <= end_val; i++) {
        local_sum += i;
    }

    // Cetak dan paksa buang ke buffer console (agar teks tidak tumpang tindih/garbled)
    printf("Pekerja (Rank %d) menghitung dari %lld sampai %lld dengan hasil lokal: %lld\n", rank, start_val, end_val, local_sum);
    fflush(stdout); 

    // Sinkronisasi: Tunggu semua pekerja selesai mencetak prosesnya sebelum Root mencetak hasil akhir
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank != 0) {
        // Pekerja mengirimkan hasil sum lokal ke Root (Rank 0)
        MPI_Send(&local_sum, 1, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD);
    } else {
        // Root mengumpulkan seluruh hasil
        long long total_sum = local_sum;
        long long recv_val;
        
        for (int i = 1; i < size; i++) {
            MPI_Recv(&recv_val, 1, MPI_LONG_LONG, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_sum += recv_val;
        }
        
        printf("=========================================\n");
        printf("Hasil akhir perhitungan total: %lld\n", total_sum);
        printf("=========================================\n");
        fflush(stdout);
    }

    // Membersihkan state MPI
    MPI_Finalize();
    return 0;
}