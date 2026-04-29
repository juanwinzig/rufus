#include <stdio.h>
#include <math.h>
#include <string.h>

#define MAX_FRAMES 100
#define N_MFCC 13
#define FINAL_COEFFS 12
#define NUM_TEMPLATES 6
#define THRESHOLD 0.15f


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

float mse(float *a, float *b, int n) {

    float sum = 0;

    for (int i = 0; i < n; i++) {

        float d = a[i] - b[i];

        sum += d * d;
    }

    return sum / n;
}

// ------------------------------------------------
// TEMPLATE
// ------------------------------------------------

int load_template(
    const char *filename,
    float *vec
) {

    FILE *f = fopen(filename, "r");

    if (!f) {
        printf("No se pudo abrir: %s\n", filename);
        return 0;
    }

    for (int i = 0; i < FINAL_COEFFS; i++) {

        fscanf(f, "%f,", &vec[i]);
    }

    fclose(f);

    return 1;
}

// ------------------------------------------------
// TEMPLATES EN MEMORIA
// ------------------------------------------------

static float t_encender[NUM_TEMPLATES][FINAL_COEFFS];
static float t_apagar[NUM_TEMPLATES][FINAL_COEFFS];
static float t_luz[NUM_TEMPLATES][FINAL_COEFFS];
static float t_rufus[NUM_TEMPLATES][FINAL_COEFFS];
static float t_clima[NUM_TEMPLATES][FINAL_COEFFS];

static int templates_loaded = 0;

// ------------------------------------------------
// CARGAR SOLO UNA VEZ
// ------------------------------------------------

void load_templates_once() {

    if (templates_loaded) {
        return;
    }

    char path[256];

    for (int i = 0; i < NUM_TEMPLATES; i++) {

        // Encender
        sprintf(
            path,
            "Resource/Encender%d.csv",
            i + 1
        );

        load_template(
            path,
            t_encender[i]
        );

        // Apagar
        sprintf(
            path,
            "Resource/Apagar%d.csv",
            i + 1
        );

        load_template(
            path,
            t_apagar[i]
        );

        // Luz
        sprintf(
            path,
            "Resource/Luz%d.csv",
            i + 1
        );

        load_template(
            path,
            t_luz[i]
        );

        // Rufus
        sprintf(
            path,
            "Resource/Rufus%d.csv",
            i + 1
        );

        load_template(
            path,
            t_rufus[i]
        );

        // Clima
        sprintf(
            path,
            "Resource/Clima%d.csv",
            i + 1
        );

        load_template(
            path,
            t_clima[i]
        );
    }

    templates_loaded = 1;
}

// ------------------------------------------------
// BUSCAR MEJOR MATCH
// ------------------------------------------------

float best_distance(
    float input[FINAL_COEFFS],
    float templates[NUM_TEMPLATES][FINAL_COEFFS]
) {

    float min = mse(
        input,
        templates[0],
        FINAL_COEFFS
    );

    for (int i = 1; i < NUM_TEMPLATES; i++) {

        float d = mse(
            input,
            templates[i],
            FINAL_COEFFS
        );

        if (d < min) {
            min = d;
            //printf("Lejos")
        }
    }

    return min;
}

// ------------------------------------------------
// CLASIFICADOR
// ------------------------------------------------

const char* classify(
    float mfcc[MAX_FRAMES][N_MFCC],
    int frames
) {

    load_templates_once();

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

    float input_vec[FINAL_COEFFS];

    compress(
        mfcc_no_c0,
        frames,
        input_vec
    );

    // ------------------------------------------------
    // comparar contra grupos
    // ------------------------------------------------

    float d_encender =
        best_distance(
            input_vec,
            t_encender
        );

    float d_apagar =
        best_distance(
            input_vec,
            t_apagar
        );

    float d_luz =
        best_distance(
            input_vec,
            t_luz
        );

    float d_rufus =
        best_distance(
            input_vec,
            t_rufus
        );

    float d_clima =
        best_distance(
            input_vec,
            t_clima
        );

    // ------------------------------------------------
    // decisión
    // ------------------------------------------------

    float min = d_encender;
    const char *word = "encender";

    if (d_apagar < min) {
        min = d_apagar;
        word = "apagar";
    }

    if (d_luz < min) {
        min = d_luz;
        word = "luz";
    }

    if (d_rufus < min) {
        min = d_rufus;
        word = "rufus";
    }

    if (d_clima < min) {
        min = d_clima;
        word = "clima";
    }

    // filtro desconocido

    if (min > THRESHOLD) {
        return "desconocido";
    }
    printf("E:%f A:%f L:%f R:%f C:%f\n",
    d_encender,
    d_apagar,
    d_luz,
    d_rufus,
    d_clima);
    return word;
}
