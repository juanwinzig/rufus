#include <math.h>

#define SAMPLE_RATE 16000
#define FRAME_SIZE 400
#define HOP_SIZE 160
#define N_MFCC 13
#define N_MELS 26
#define PI 3.141592653589793
#define MAX_FRAMES 100

void hann(float *x) {
    for (int i = 0; i < FRAME_SIZE; i++) {
        x[i] *= 0.5f * (1 - cos(2 * PI * i / (FRAME_SIZE - 1)));
    }
}

// FFT
void fft(float *real, float *imag) {
    float out_r[FRAME_SIZE], out_i[FRAME_SIZE];

    for (int k = 0; k < FRAME_SIZE; k++) {
        float sr = 0, si = 0;
        for (int t = 0; t < FRAME_SIZE; t++) {
            float angle = -2 * PI * k * t / FRAME_SIZE;
            sr += real[t] * cos(angle);
            si += real[t] * sin(angle);
        }
        out_r[k] = sr;
        out_i[k] = si;
    }

    for (int i = 0; i < FRAME_SIZE; i++) {
        real[i] = out_r[i];
        imag[i] = out_i[i];
    }
}

void power_spectrum(float *real, float *imag, float *mag) {
    for (int i = 0; i < FRAME_SIZE; i++) {
        mag[i] = real[i]*real[i] + imag[i]*imag[i];
    }
}

float hz_to_mel(float hz) {
    return 2595.0f * log10f(1 + hz / 700.0f);
}

float mel_to_hz(float mel) {
    return 700.0f * (powf(10, mel / 2595.0f) - 1);
}

// filtro mel
void mel_filter(float *mag, float *mel_out) {
    int n_bins = FRAME_SIZE / 2 + 1;

    float mel_min = hz_to_mel(0);
    float mel_max = hz_to_mel(SAMPLE_RATE / 2);

    float mel_pts[N_MELS + 2];
    float hz_pts[N_MELS + 2];
    int bin[N_MELS + 2];

    for (int i = 0; i < N_MELS + 2; i++) {
        mel_pts[i] = mel_min + (mel_max - mel_min) * i / (N_MELS + 1);
        hz_pts[i] = mel_to_hz(mel_pts[i]);
        bin[i] = (int)((FRAME_SIZE + 1) * hz_pts[i] / SAMPLE_RATE);
    }

    for (int m = 0; m < N_MELS; m++) {
        mel_out[m] = 0;

        for (int k = bin[m]; k < bin[m+1] && k < n_bins; k++) {
            float w = (k - bin[m]) / (float)(bin[m+1] - bin[m]);
            mel_out[m] += mag[k] * w;
        }

        for (int k = bin[m+1]; k < bin[m+2] && k < n_bins; k++) {
            float w = (bin[m+2] - k) / (float)(bin[m+2] - bin[m+1]);
            mel_out[m] += mag[k] * w;
        }

        if (mel_out[m] < 1e-10f) mel_out[m] = 1e-10f;

        mel_out[m] = logf(mel_out[m]);
    }
}

//
void dct(float *in, float *out) {
    for (int k = 0; k < N_MFCC; k++) {
        out[k] = 0;
        for (int n = 0; n < N_MELS; n++) {
            out[k] += in[n] * cos(PI * k * (2*n + 1) / (2 * N_MELS));
        }
    }
}

//funcio para calcular la MFCC
int compute_mfcc(float *audio, int length, float mfcc_out[MAX_FRAMES][N_MFCC]) {

    int num_frames = 1 + (length - FRAME_SIZE) / HOP_SIZE;

    if (num_frames > MAX_FRAMES) num_frames = MAX_FRAMES;

    float real[FRAME_SIZE], imag[FRAME_SIZE];
    float mag[FRAME_SIZE];
    float mel[N_MELS];

    for (int i = 0; i < num_frames; i++) {
        int start = i * HOP_SIZE;

        for (int j = 0; j < FRAME_SIZE; j++) {
            real[j] = audio[start + j];
            imag[j] = 0;
        }

        hann(real);
        fft(real, imag);
        power_spectrum(real, imag, mag);
        mel_filter(mag, mel);
        dct(mel, mfcc_out[i]); 
    }

    return num_frames;
}