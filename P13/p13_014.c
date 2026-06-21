#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Fungsi untuk membaca matriks dari file CSV
void read_matrix_from_csv(const char* filename, double* matrix, int N) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Tidak dapat membuka file %s\n", filename);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    for (int i = 0; i < N * N; i++) {
        if (fscanf(file, "%lf", &matrix[i]) != 1) {
            printf("Error: Format CSV tidak sesuai di %s\n", filename);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fgetc(file); // Melewati karakter koma (,) atau newline (\n)
    }
    fclose(file);
}

// Fungsi untuk menyimpan hasil matriks ke file CSV
void write_matrix_to_csv(const char* filename, double* matrix, int N) {
    FILE *file = fopen(filename, "w");
    if (!file) return;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(file, "%.2f", matrix[i * N + j]);
            if (j < N - 1) fprintf(file, ",");
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    int numtasks, taskid, numworkers, source, dest, mtype, rows, averow, extra, offset;
    double start_time, end_time;
    
    // Argumen N tetap diperlukan untuk alokasi memori sebelum file dibaca
    int N = 100; 
    if (argc > 1) {
        N = atoi(argv[1]);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    
    if (numtasks < 2) {
        if (taskid == 0) printf("Butuh minimal 2 proses MPI!\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(1);
    }
    numworkers = numtasks - 1;

    double *A = (double *)malloc(N * N * sizeof(double));
    double *B = (double *)malloc(N * N * sizeof(double));
    double *C = (double *)malloc(N * N * sizeof(double));

    // ================= MASTER TASK =================
    if (taskid == 0) {
        printf("MPI Matrix Multiplication (%d Node)\n", numtasks);
        char *file_A = (argc > 2) ? argv[2] : "matrix_1.csv";
        char *file_B = (argc > 3) ? argv[3] : "matrix_2.csv";
        char *file_C = (argc > 4) ? argv[4] : "result_matrix.csv";

        printf("Membaca file %s dan %s ukuran %dx%d...\n", file_A, file_B, N, N);
        
        read_matrix_from_csv(file_A, A, N);
        read_matrix_from_csv(file_B, B, N);
        start_time = MPI_Wtime();

        averow = N / numworkers;
        extra = N % numworkers;
        offset = 0;
        mtype = 1;

        // Distribusi ke worker
        for (dest = 1; dest <= numworkers; dest++) {
            rows = (dest <= extra) ? averow + 1 : averow;   
            MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
            MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
            MPI_Send(&A[offset * N], rows * N, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
            MPI_Send(B, N * N, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
            offset = offset + rows;
        }

        // Kumpulkan hasil
        mtype = 2;
        for (source = 1; source <= numworkers; source++) {
            MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&C[offset * N], rows * N, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        end_time = MPI_Wtime();
        printf("Waktu komputasi MPI: %f detik\n", end_time - start_time);
        
        printf("Menyimpan hasil ke result_matrix.csv...\n");
        write_matrix_to_csv(file_C, C, N);
        printf("Selesai!\n");
    }

    // ================= WORKER TASK =================
    if (taskid > 0) {
        mtype = 1;
        MPI_Recv(&offset, 1, MPI_INT, 0, mtype, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&rows, 1, MPI_INT, 0, mtype, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(A, rows * N, MPI_DOUBLE, 0, mtype, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(B, N * N, MPI_DOUBLE, 0, mtype, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int k = 0; k < N; k++) {
            for (int i = 0; i < rows; i++) {
                C[i * N + k] = 0.0;
                for (int j = 0; j < N; j++) {
                    C[i * N + k] += A[i * N + j] * B[j * N + k];
                }
            }
        }

        mtype = 2;
        MPI_Send(&offset, 1, MPI_INT, 0, mtype, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, 0, mtype, MPI_COMM_WORLD);
        MPI_Send(C, rows * N, MPI_DOUBLE, 0, mtype, MPI_COMM_WORLD);
    }

    free(A); free(B); free(C);
    MPI_Finalize();
    return 0;
}