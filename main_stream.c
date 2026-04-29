#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portaudio.h>


#define SAMPLE_RATE 16000
#define HOP_SIZE 160
#define TARGET_LENGTH 16000
#define N_MFCC 13
#define MAX_FRAMES 100

int compute_mfcc(float *audio, int length, float mfcc[MAX_FRAMES][N_MFCC]);

const char* classify(float mfcc[MAX_FRAMES][N_MFCC], int frames);

float audio_buffer[TARGET_LENGTH];
int write_index = 0;

static int audio_callback(
    const void *inputBuffer,
    void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
) {
    const float *in = (const float*)inputBuffer;

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        audio_buffer[write_index] = in[i];
        write_index = (write_index + 1) % TARGET_LENGTH;
    }

    return paContinue;
}
//filtro pasa altas.
float highpass(float x) {
    static float prev_y = 0;
    static float prev_x = 0;

    float alpha = 0.95f;  // ajustable

    float y = alpha * (prev_y + x - prev_x);

    prev_y = y;
    prev_x = x;

    return y;
}
//filtro pasa bajas
float lowpass(float x) {
    static float y = 0;

    float alpha = 0.1f;  // ajustable

    y = y + alpha * (x - y);

    return y;
}
//pasa banda.
float bandpass(float x) {
    float hp = highpass(x);
    float bp = lowpass(hp);
    return bp;
}
// ---------- EXTRAER VENTANA ----------
void get_current_window(float *window) {
    int idx = write_index;

    for (int i = 0; i < TARGET_LENGTH; i++) {
        float sample = audio_buffer[(idx + i) % TARGET_LENGTH];
        window[i] = sample;
    }
    //printf("Aqui")
}
//para comprobar si hay o no una voz 
float compute_energy(float *audio, int length) {
    float sum = 0;
    for (int i = 0; i < length; i++) {
        sum += audio[i] * audio[i];
    }

    return sum / length;
}

int main() {
    printf("Inicio...\n");
    PaStream *stream;
    PaError err;

    float window[TARGET_LENGTH];
    float mfcc[MAX_FRAMES][N_MFCC];

    memset(audio_buffer, 0, sizeof(audio_buffer));

    // Init PortAudio
    err = Pa_Initialize();
    if (err != paNoError){
        printf("Error Pa_Initialize : %s\n", Pa_GetErrorText(err));
        return -1;
    }

    err = Pa_OpenDefaultStream(
        &stream,
        1,             
        0,              
        paFloat32,
        SAMPLE_RATE,
        HOP_SIZE,
        audio_callback,
        NULL
    );

    if (err != paNoError) {
        printf("Error Pa_OpenDefaultStream: %s\n", Pa_GetErrorText(err));
        return -1;
    }


    err = Pa_StartStream(stream);
    if (err != paNoError){
        printf("Error Pa_StartSteram: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    printf("Escuchando...\n");

    // ---------- LOOP ----------
    while (1) {
        Pa_Sleep(200);

        // 1. Obtener ventana de 1s
        get_current_window(window);
       
        float energy = compute_energy(window, TARGET_LENGTH); //distinguir entre voz y silencio
        if (energy < 0.0001f) {
            printf("Silencio\n");
            continue;
        }

        //printf("Ha llegado")
        // 2. Calcular MFCC
        int frames = compute_mfcc(window, TARGET_LENGTH, mfcc);

        // 3. Clasificar usando MFCC
        const char* word = classify(mfcc, frames);

        // Agregar acciones pos deteccion de la palabra clave.
        //
        // 4. Mostrar resultado
        printf("Detectado: %s\n", word);

    }

    Pa_StopStream(stream);
    Pa_Terminate();

    return 0;
}