// ================================================================
// MonProjet.ino
// Diagnostic de défauts Moteur CC 12V — ESP32
// Modèle : Régression Logistique (m2cgen)
// ================================================================

#include <math.h>
#include "scaler_params_reel.h"

// ── Déclaration de la fonction du modèle ─────────────────────────
// (définie dans model_esp32_reel.c)
 #include "model_esp32_reel.h"

// ── Broches ESP32 ────────────────────────────────────────────────
#define PIN_TENSION  34   // ADC1 — diviseur de tension
#define PIN_COURANT  35   // ADC1 — ACS712
#define PIN_VITESSE  26   // GPIO — signal encodeur Hall (ou ADC)
#define PIN_LED_OK   2    // LED verte  → Normal
#define PIN_LED_ALT  4    // LED rouge  → Surcharge

// ── Calibration des capteurs (à adapter à ton montage) ───────────
#define VCC             3.3f    // tension de référence ESP32
#define ADC_RESOLUTION  4095.0f // résolution 12 bits
#define TENSION_DIVISOR 0.3f    // rapport diviseur : R2/(R1+R2)
                                // ex: R1=20kΩ, R2=6.8kΩ → 0.254
#define ACS712_OFFSET   1.65f   // tension de sortie à courant=0 (VCC/2)
#define ACS712_SENS     0.185f  // sensibilité V/A (ACS712-5A: 0.185 V/A)
#define HALL_PPR        20      // impulsions par tour de l'encodeur

// ── Buffer circulaire (fenêtre glissante) ────────────────────────
float buf_tension[WINDOW_SIZE];
float buf_courant[WINDOW_SIZE];
float buf_vitesse[WINDOW_SIZE];
int   buf_index   = 0;
bool  buf_full    = false;
int   stride_count = 0;

// ── Variables encodeur Hall ───────────────────────────────────────
volatile uint32_t hall_count = 0;
unsigned long     last_rpm_time = 0;

void IRAM_ATTR onHallPulse() {
    hall_count++;
}

// ================================================================
// LECTURE DES CAPTEURS
// ================================================================
float readTension() {
    int raw = analogRead(PIN_TENSION);
    float v_adc = (raw / ADC_RESOLUTION) * VCC;
    return v_adc / TENSION_DIVISOR;  // tension moteur réelle
}

float readCourant() {
    int raw = analogRead(PIN_COURANT);
    float v_adc = (raw / ADC_RESOLUTION) * VCC;
    return (v_adc - ACS712_OFFSET) / ACS712_SENS;  // courant en A
}

float readVitesse() {
    // Calcul RPM toutes les 500 ms
    unsigned long now = millis();
    float dt = (now - last_rpm_time) / 1000.0f;
    if (dt < 0.01f) return buf_vitesse[(buf_index - 1 + WINDOW_SIZE) % WINDOW_SIZE];
    
    noInterrupts();
    uint32_t cnt = hall_count;
    hall_count = 0;
    interrupts();
    last_rpm_time = now;
    
    return (cnt / (float)HALL_PPR) * (60.0f / dt);  // RPM
}

// ================================================================
// EXTRACTION DES 45 FEATURES
// (même logique que Python : extract_features())
// ================================================================

// ── Tri simple pour IQR ──────────────────────────────────────────
void sortArray(float* arr, float* sorted, int n) {
    for (int i = 0; i < n; i++) sorted[i] = arr[i];
    for (int i = 0; i < n-1; i++)
        for (int j = i+1; j < n; j++)
            if (sorted[j] < sorted[i]) { float t = sorted[i]; sorted[i] = sorted[j]; sorted[j] = t; }
}

// ── DFT simplifiée pour les 5 features fréquentielles ────────────
// Sur 30 pts, c'est 900 multiplications — rapide sur ESP32
void computeSpectralFeatures(float* x, int n,
    float* energy, float* domFreq, float* meanFreq,
    float* specStd, float* peakRatio)
{
    int half = n / 2;
    float spec[15];  // amplitudes spectrales (n/2 = 15)
    float sumSpec = 0.0f;
    float maxSpec = 0.0f;
    int   maxIdx  = 0;

    for (int k = 0; k < half; k++) {
        float re = 0, im = 0;
        for (int t = 0; t < n; t++) {
            float angle = 2.0f * M_PI * k * t / n;
            re += x[t] * cosf(angle);
            im -= x[t] * sinf(angle);
        }
        spec[k]  = sqrtf(re*re + im*im);
        sumSpec += spec[k];
        if (spec[k] > maxSpec) { maxSpec = spec[k]; maxIdx = k; }
    }

    // Énergie spectrale
    *energy = 0;
    for (int k = 0; k < half; k++) *energy += spec[k] * spec[k];

    // Fréquence dominante (normalisée 0..0.5)
    *domFreq = (sumSpec > 1e-10f) ? (float)maxIdx / n : 0.0f;

    // Fréquence moyenne pondérée
    float wsum = 0;
    for (int k = 0; k < half; k++) wsum += ((float)k / n) * spec[k];
    *meanFreq = (sumSpec > 1e-10f) ? wsum / sumSpec : 0.0f;

    // Dispersion spectrale (écart-type des amplitudes)
    float smean = sumSpec / half;
    float svar  = 0;
    for (int k = 0; k < half; k++) svar += (spec[k]-smean)*(spec[k]-smean);
    *specStd = sqrtf(svar / half);

    // Ratio du pic
    *peakRatio = (sumSpec > 1e-10f) ? maxSpec / sumSpec : 0.0f;
}

// ── Feature extraction complète (45 features) ────────────────────
void extractFeatures(float* tension, float* courant, float* vitesse,
                     int n, float* features)
{
    float* signals[3] = { tension, courant, vitesse };
    int    fi = 0;  // index feature courant

    for (int s = 0; s < 3; s++) {
        float* x = signals[s];

        // ── Statistiques temporelles ──────────────────────────────
        float mean = 0, rms = 0, mx = x[0], mn = x[0];
        for (int i = 0; i < n; i++) {
            mean += x[i];
            rms  += x[i] * x[i];
            if (x[i] > mx) mx = x[i];
            if (x[i] < mn) mn = x[i];
        }
        mean /= n;
        rms   = sqrtf(rms / n);

        float var = 0, sk3 = 0, sk4 = 0;
        for (int i = 0; i < n; i++) {
            float d = x[i] - mean;
            var += d*d; sk3 += d*d*d; sk4 += d*d*d*d;
        }
        float std_val = sqrtf(var / n);
        float kurt = (std_val > 1e-10f) ? (sk4/n) / (std_val*std_val*std_val*std_val) - 3.0f : 0.0f;
        float skew = (std_val > 1e-10f) ? (sk3/n) / (std_val*std_val*std_val) : 0.0f;

        float mad = 0;
        for (int i = 1; i < n; i++) mad += fabsf(x[i] - x[i-1]);
        mad /= (n - 1);

        float sorted[WINDOW_SIZE];
        sortArray(x, sorted, n);
        float q25 = sorted[n/4];
        float q75 = sorted[3*n/4];

        // ── Écriture features temporelles ─────────────────────────
        features[fi++] = mean;
        features[fi++] = std_val;
        features[fi++] = rms;
        features[fi++] = mx;
        features[fi++] = mn;
        features[fi++] = mx - mn;     // peak-to-peak
        features[fi++] = kurt;
        features[fi++] = skew;
        features[fi++] = mad;
        features[fi++] = q75 - q25;  // IQR

        // ── Features fréquentielles ───────────────────────────────
        float energy, domF, meanF, specStd, peakR;
        computeSpectralFeatures(x, n, &energy, &domF, &meanF, &specStd, &peakR);
        features[fi++] = energy;
        features[fi++] = domF;
        features[fi++] = meanF;
        features[fi++] = specStd;
        features[fi++] = peakR;
    }
}

// ================================================================
// NORMALISATION + PRÉDICTION
// ================================================================
int predict() {
    // 1. Extraire les 45 features brutes
    float raw_features[N_FEATURES];
    extractFeatures(buf_tension, buf_courant, buf_vitesse,
                    WINDOW_SIZE, raw_features);

    // 2. Normaliser : x_norm = (x - mean) / scale
    double scaled[N_FEATURES];
    for (int i = 0; i < N_FEATURES; i++) {
        scaled[i] = ((double)raw_features[i] - SCALER_MEAN[i])
                    / SCALER_SCALE[i];
    }

    // 3. Inférence : score() retourne log-odds de Surcharge
    double result = score(scaled);

    // 4. Décision
    return (result > 0.0) ? 1 : 0;  // 1=Surcharge, 0=Normal
}

// ================================================================
// SETUP & LOOP
// ================================================================
void setup() {
    Serial.begin(115200);
    pinMode(PIN_LED_OK,  OUTPUT);
    pinMode(PIN_LED_ALT, OUTPUT);
    pinMode(PIN_VITESSE, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_VITESSE), onHallPulse, RISING);

    analogReadResolution(12);  // 12 bits sur ESP32

    Serial.println("=== Diagnostic Moteur CC 12V ===");
    Serial.println("Fenetre : 30 pts  |  Stride : 15  |  Modele : Reg. Logistique");
    Serial.println("Remplissage du buffer initial...");

    last_rpm_time = millis();
}

void loop() {
    // ── Lecture des 3 capteurs ────────────────────────────────────
    float t_val = readTension();
    float c_val = readCourant();
    float v_val = readVitesse();

    // ── Mise à jour du buffer circulaire ─────────────────────────
    buf_tension[buf_index] = t_val;
    buf_courant[buf_index] = c_val;
    buf_vitesse[buf_index] = v_val;
    buf_index = (buf_index + 1) % WINDOW_SIZE;

    if (!buf_full && buf_index == 0) buf_full = true;

    // ── Affichage des valeurs brutes ──────────────────────────────
    Serial.printf("T=%.2fV  I=%.3fA  V=%.1fRPM  ",
                  t_val, c_val, v_val);

    // ── Prédiction toutes les STRIDE acquisitions ─────────────────
    if (buf_full) {
        stride_count++;
        if (stride_count >= STRIDE) {
            stride_count = 0;

            int etat = predict();

            if (etat == 1) {
                Serial.println("→ ⚠ SURCHARGE DÉTECTÉE !");
                digitalWrite(PIN_LED_OK,  LOW);
                digitalWrite(PIN_LED_ALT, HIGH);
            } else {
                Serial.println("→ ✓ Normal");
                digitalWrite(PIN_LED_OK,  HIGH);
                digitalWrite(PIN_LED_ALT, LOW);
            }
        } else {
            Serial.println("(buffer)");
        }
    } else {
        int remaining = WINDOW_SIZE - buf_index;
        Serial.printf("(init : %d pts restants)\n", remaining);
    }

    delay(500);  // 1 acquisition toutes les 500 ms = 2 Hz
}