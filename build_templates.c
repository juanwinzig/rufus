#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>

#define TARGET_LENGTH 16000

#define MAX_FRAMES 100
#define N_MFCC 13
#define FINAL_COEFFS 12

// ------------------------------------------------
// PROTOTIPOS
// ------------------------------------------------

// mfcc.c
int compute_mfcc(
    float *audio,
    int length,
    float mfcc_out[MAX_FRAMES][N_MFCC]
);

// wav
int read_wav(
    const char *filename,
    float *audio,
    int max_samples
);

// procesamiento
void remove_c0(
    float mfcc[MAX_FRAMES][N_MFCC],
    int frames,
    float out[MAX_FRAMES][FINAL_COEFFS]
);

void normalize(
    float data[MAX_FRAMES][FINAL_COEFFS],
    int frames
);

void compress(
    float data[MAX_FRAMES][FINAL_COEFFS],
    int frames,
    float out[FINAL_COEFFS]
);

// generar vector + guardar csv
void classify(
    float mfcc[MAX_FRAMES][N_MFCC],
    int frames,
    const char *output_path
);

// ------------------------------------------------
// MAIN
// ------------------------------------------------

int main() {

    const char *input_folder =
        "C:\\Users\\Juanw\\OneDrive\\Documentos\\AudiosWAV";

    const char *output_folder =
        "C:\\Users\\Juanw\\OneDrive\\Documentos\\ProyectMicro\\Resource";

    DIR *dir;
    struct dirent *entry;

    dir = opendir(input_folder);

    if (dir == NULL) {
        printf("No se pudo abrir carpeta\n");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {

        // solo .wav
        if (strstr(entry->d_name, ".wav") == NULL) {
            continue;
        }

        // ----------------------------
        // path entrada
        // ----------------------------

        char input_path[512];

        snprintf(
            input_path,
            sizeof(input_path),
            "%s\\%s",
            input_folder,
            entry->d_name
        );

        printf("Procesando: %s\n", input_path);

        // ----------------------------
        // leer audio
        // ----------------------------

        float audio[TARGET_LENGTH];

        int samples = read_wav(
            input_path,
            audio,
            TARGET_LENGTH
        );

        if (samples == 0) {
            printf("Error leyendo WAV\n");
            continue;
        }

        // ----------------------------
        // MFCC
        // ----------------------------

        float mfcc[MAX_FRAMES][N_MFCC];

        int frames = compute_mfcc(
            audio,
            samples,
            mfcc
        );

        // ----------------------------
        // nombre salida
        // ----------------------------

        char base_name[256];

        strcpy(base_name, entry->d_name);

        char *dot = strrchr(base_name, '.');

        if (dot != NULL) {
            *dot = '\0';
        }

        char output_path[512];

        snprintf(
            output_path,
            sizeof(output_path),
            "%s\\%s.csv",
            output_folder,
            base_name
        );

        // ----------------------------
        // generar vector y guardar
        // ----------------------------

        classify(
            mfcc,
            frames,
            output_path
        );

        printf("Guardado: %s\n", output_path);
    }

    closedir(dir);

    printf("Finalizado\n");

    return 0;
}

// ------------------------------------------------
// WAV
// ------------------------------------------------

int read_wav(
    const char *filename,
    float *audio,
    int max_samples
) {

    FILE *f = fopen(filename, "rb");

    if (!f) {
        return 0;
    }

    fseek(f, 44, SEEK_SET);

    short sample;

    int i = 0;

    while (
        fread(&sample, sizeof(short), 1, f)
        && i < max_samples
    ) {

        audio[i++] = sample / 32768.0f;
    }

    fclose(f);

    return i;
}

// ------------------------------------------------
// UTILIDADES
// ------------------------------------------------

void remove_c0(
    float mfcc[MAX_FRAMES][N_MFCC],
    int frames,
    float out[MAX_FRAMES][FINAL_COEFFS]
) {

    for (int i = 0; i < frames; i++) {

        for (int j = 1; j < N_MFCC; j++) {

            out[i][j - 1] = mfcc[i][j];
        }
    }
}

void normalize(
    float data[MAX_FRAMES][FINAL_COEFFS],
    int frames
) {

    float sum = 0;
    float sum_sq = 0;

    int total = frames * FINAL_COEFFS;

    for (int i = 0; i < frames; i++) {

        for (int j = 0; j < FINAL_COEFFS; j++) {

            float v = data[i][j];

            sum += v;
            sum_sq += v * v;
        }
    }

    float mean = sum / total;

    float var =
        (sum_sq / total) - (mean * mean);

    float std =
        sqrtf(var) + 1e-8f;

    for (int i = 0; i < frames; i++) {

        for (int j = 0; j < FINAL_COEFFS; j++) {

            data[i][j] =
                (data[i][j] - mean) / std;
        }
    }
}

void compress(
    float data[MAX_FRAMES][FINAL_COEFFS],
    int frames,
    float out[FINAL_COEFFS]
) {

    for (int j = 0; j < FINAL_COEFFS; j++) {

        float sum = 0;

        for (int i = 0; i < frames; i++) {

            sum += data[i][j];
        }

        out[j] = sum / frames;
    }
}

// ------------------------------------------------
// GENERAR VECTOR
// ------------------------------------------------

void classify(
    float mfcc[MAX_FRAMES][N_MFCC],
    int frames,
    const char *output_path
) {

    // quitar c0

    float mfcc_no_c0[MAX_FRAMES][FINAL_COEFFS];

    remove_c0(
        mfcc,
        frames,
        mfcc_no_c0
    );

    // normalizar

    normalize(
        mfcc_no_c0,
        frames
    );

    // comprimir

    float vec[FINAL_COEFFS];

    compress(
        mfcc_no_c0,
        frames,
        vec
    );

    // guardar csv

    FILE *f = fopen(output_path, "w");

    if (!f) {
        printf("Error guardando CSV\n");
        return;
    }

    for (int i = 0; i < FINAL_COEFFS; i++) {

        fprintf(f, "%f", vec[i]);

        if (i < FINAL_COEFFS - 1) {
            fprintf(f, ",");
        }
    }

    fprintf(f, "\n");

    fclose(f);
}
