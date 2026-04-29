#include <stdio.h>
#include <stdlib.h>

#define TARGET_LENGTH 16000
#define MAX_FRAMES 100
#define N_MFCC 13

// ------------------------------------------------
// codigo utilizado para probar el comportamietno de 
// la clasificacion y la MFCC.
// ------------------------------------------------

// mfcc.c
int compute_mfcc(
    float *audio,
    int length,
    float mfcc_out[MAX_FRAMES][N_MFCC]
);

// classifier.c
const char* classify(
    float mfcc[MAX_FRAMES][N_MFCC],
    int frames
);

// wav
int read_wav(
    const char *filename,
    float *audio,
    int max_samples
);

// ------------------------------------------------
// MAIN
// ------------------------------------------------

int main() {

    const char *path =
        "C:\\Users\\Juanw\\OneDrive\\Documentos\\Audacity\\rufus.wav";

    float audio[TARGET_LENGTH];

    // ------------------------------------------------
    // leer wav
    // ------------------------------------------------

    int samples = read_wav(
        path,
        audio,
        TARGET_LENGTH
    );

    if (samples == 0) {

        printf("Error leyendo WAV\n");
        return 1;
    }

    printf("Audio cargado\n");
    printf("Samples: %d\n", samples);

    // ------------------------------------------------
    // MFCC
    // ------------------------------------------------

    float mfcc[MAX_FRAMES][N_MFCC];

    int frames = compute_mfcc(
        audio,
        samples,
        mfcc
    );

    printf("Frames: %d\n", frames);

    // ------------------------------------------------
    // clasificar
    // ------------------------------------------------

    const char *word = classify(
        mfcc,
        frames
    );

    printf("Detectado: %s\n", word);

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

    // saltar header WAV
    fseek(f, 44, SEEK_SET);

    short sample;

    int i = 0;

    while (
        fread(&sample, sizeof(short), 1, f)
        && i < max_samples
    ) {

        audio[i++] =
            sample / 32768.0f;
    }

    fclose(f);

    return i;
}