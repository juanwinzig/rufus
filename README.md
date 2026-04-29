# Rufus

Sistema de automatización de laboratorio mediante reconocimiento de comandos de voz en tiempo real.

---

## Descripción

Rufus es un proyecto experimental de reconocimiento de voz orientado a sistemas embebidos y automatización de laboratorio. El objetivo es detectar comandos de voz específicos y ejecutar acciones sobre dispositivos físicos como iluminación o aire acondicionado.

El sistema está diseñado con restricciones similares a hardware embebido, evitando modelos pesados de inteligencia artificial y utilizando técnicas clásicas de procesamiento digital de señales.

Actualmente el reconocimiento se basa en:

- Captura de audio en tiempo real
- Extracción de características mediante MFCC (Mel-Frequency Cepstral Coefficients)
- Clasificación por comparación de templates de voz
- Procesamiento optimizado en C

---

## Características

- Captura de audio en tiempo real usando PortAudio
- Extracción MFCC implementada manualmente en C
- Reconocimiento de comandos por templates
- Arquitectura orientada a sistemas embebidos
- Preparado para futura integración con microcontroladores

---

## Estado del proyecto

Proyecto actualmente en desarrollo.

Funciones implementadas:

- [x] Captura de audio en tiempo real
- [x] Buffer circular de audio
- [x] Extracción MFCC
- [x] Clasificación básica por templates
- [x] Detección de silencio

Funciones planeadas:

- [ ] Clasificación mediante DTW
- [ ] Mejor filtrado de ruido
- [ ] Optimización para ESP32
- [ ] Activación de dispositivos reales
- [ ] Sistema wake-word más robusto
- [ ] Mejora del manejo de palabras desconocidas

---

## Arquitectura del sistema

```text
Audio Input
     ↓
Buffer Circular
     ↓
Preprocesamiento
     ↓
Extracción MFCC
     ↓
Clasificador
     ↓
Comando Detectado
