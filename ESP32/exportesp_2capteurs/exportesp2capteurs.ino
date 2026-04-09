// ================================================================
// MonProjet_2Capteurs.ino
// Diagnostic de défauts Moteur CC 12V — ESP32
// Modèle : Régression Logistique (m2cgen) - 2 Capteurs
// ================================================================

#include <math.h>
#include "scaler_params_TC.h"

// ── Déclaration de la fonction du modèle ─────────────────────────
// (définie dans model_esp32_TC.c)
#include "model_esp32_TC.h"

// ── Broches ESP32 ────────────────────────────────────────────────
#define PIN_TENSION  34   // ADC1 — diviseur de tension [cite: 1975]
#define PIN_COURANT  35   // ADC1 — ACS712 [cite: 1975]
#define PIN_LED_OK   2    // LED verte  → Normal [cite: 1975]
#define PIN_LED_ALT  4    // LED rouge  → Surcharge [cite: 1975]

// ── Calibration des capteurs (à adapter à ton montage) ───────────
#define VCC             3.3f    // tension de référence ESP32 [cite: 1976]
#define ADC_RESOLUTION  4095.0f // résolution 12 bits [cite: 1976]
#define TENSION_DIVISOR 0.3f    // rapport diviseur : R2/(R1+R2) [cite: 1976]
                                // ex: R1=20kΩ, R2=6.8kΩ → 0.254
#define ACS712_OFFSET   1.65f   // tension de sortie à courant=0 (VCC/2) [cite: 1976]
#define ACS712_SENS     0.185f  // sensibilité V/A (ACS712-5A: 0.185 V/A) [cite: 1976, 1977]

// ── Buffer circulaire (fenêtre glissante) ────────────────────────
float buf_tension[WINDOW_SIZE]; [cite: 1977]
float buf_courant[WINDOW_SIZE]; [cite: 1978]
int   buf_index   = 0; [cite: 1978]
bool  buf_full    = false; [cite: 1978]
int   stride_count = 0; [cite: 1979]

// ================================================================
// LECTURE DES CAPTEURS
// ================================================================
float readTension() {
    int raw = analogRead(PIN_TENSION); [cite: 1981]
    float v_adc = (raw / ADC_RESOLUTION) * VCC; [cite: 1982]
    return v_adc / TENSION_DIVISOR; // tension moteur réelle [cite: 1982, 1983]
}

float readCourant() {
    int raw = analogRead(PIN_COURANT); [cite: 1983]
    float v_adc = (raw / ADC_RESOLUTION) * VCC; [cite: 1984]
    return (v_adc - ACS712_OFFSET) / ACS712_SENS; // courant en A [cite: 1984, 1985]
}

// ================================================================
// EXTRACTION DES 30 FEATURES [cite: 2060, 2071]
// ================================================================

// ── Tri simple pour IQR ──────────────────────────────────────────
void sortArray(float* arr, float* sorted, int n) {
    for (int i = 0; i < n; i++) sorted[i] = arr[i]; [cite: 1988]
    for (int i = 0; i < n-1; i++) [cite: 1989]
        for (int j = i+1; j < n; j++) [cite: 1989]
            if (sorted[j] < sorted[i]) { float t = sorted[i]; sorted[i] = sorted[j]; sorted[j] = t; } [cite: 1989, 1990]
}

// ── DFT simplifiée pour les 5 features fréquentielles ────────────
void computeSpectralFeatures(float* x, int n,
    float* energy, float* domFreq, float* meanFreq,
    float* specStd, float* peakRatio)
{
    int half = n / 2; [cite: 1990]
    float spec[15];  // amplitudes spectrales (n/2 = 15) [cite: 1991]
    float sumSpec = 0.0f; [cite: 1991]
    float maxSpec = 0.0f; [cite: 1991]
    int   maxIdx  = 0; [cite: 1992]

    for (int k = 0; k < half; k++) {
        float re = 0, im = 0; [cite: 1992]
        for (int t = 0; t < n; t++) { [cite: 1993]
            float angle = 2.0f * M_PI * k * t / n; [cite: 1993]
            re += x[t] * cosf(angle); [cite: 1994]
            im -= x[t] * sinf(angle); [cite: 1994]
        }
        spec[k]  = sqrtf(re*re + im*im); [cite: 1995]
        sumSpec += spec[k]; [cite: 1995]
        if (spec[k] > maxSpec) { maxSpec = spec[k]; maxIdx = k; } [cite: 1996]
    }

    // Énergie spectrale
    *energy = 0; [cite: 1997]
    for (int k = 0; k < half; k++) *energy += spec[k] * spec[k]; [cite: 1998]
    // Fréquence dominante (normalisée 0..0.5)
    *domFreq = (sumSpec > 1e-10f) ? (float)maxIdx / n : 0.0f; [cite: 1999]
    // Fréquence moyenne pondérée
    float wsum = 0; [cite: 2000]
    for (int k = 0; k < half; k++) wsum += ((float)k / n) * spec[k]; [cite: 2001]
    *meanFreq = (sumSpec > 1e-10f) ? wsum / sumSpec : 0.0f; [cite: 2002]
    // Dispersion spectrale (écart-type des amplitudes)
    float smean = sumSpec / half; [cite: 2003]
    float svar  = 0; [cite: 2003]
    for (int k = 0; k < half; k++) svar += (spec[k]-smean)*(spec[k]-smean); [cite: 2004]
    *specStd = sqrtf(svar / half); [cite: 2004]
    // Ratio du pic
    *peakRatio = (sumSpec > 1e-10f) ? maxSpec / sumSpec : 0.0f; [cite: 2005]
}

// ── Feature extraction complète (30 features) ────────────────────
void extractFeatures(float* tension, float* courant, int n, float* features)
{
    float* signals[2] = { tension, courant }; // 2 Capteurs uniquement [cite: 2006, 2057]
    int    fi = 0;  // index feature courant 

    for (int s = 0; s < 2; s++) { // Boucle réduite à 2 (au lieu de 3) 
        float* x = signals[s]; [cite: 2007]
        // ── Statistiques temporelles ──────────────────────────────
        float mean = 0, rms = 0, mx = x[0], mn = x[0]; [cite: 2008]
        for (int i = 0; i < n; i++) { [cite: 2009]
            mean += x[i]; [cite: 2009]
            rms  += x[i] * x[i]; [cite: 2010]
            if (x[i] > mx) mx = x[i]; [cite: 2010]
            if (x[i] < mn) mn = x[i]; [cite: 2010]
        }
        mean /= n; [cite: 2011]
        rms   = sqrtf(rms / n); [cite: 2011]
        
        float var = 0, sk3 = 0, sk4 = 0; [cite: 2012]
        for (int i = 0; i < n; i++) { [cite: 2013]
            float d = x[i] - mean; [cite: 2013]
            var += d*d; sk3 += d*d*d; sk4 += d*d*d*d; [cite: 2014]
        }
        float std_val = sqrtf(var / n); [cite: 2014]
        float kurt = (std_val > 1e-10f) ? (sk4/n) / (std_val*std_val*std_val*std_val) - 3.0f : 0.0f; [cite: 2015]
        float skew = (std_val > 1e-10f) ? (sk3/n) / (std_val*std_val*std_val) : 0.0f; [cite: 2016]

        float mad = 0; [cite: 2016]
        for (int i = 1; i < n; i++) mad += fabsf(x[i] - x[i-1]); [cite: 2017]
        mad /= (n - 1); [cite: 2017]
        
        float sorted[WINDOW_SIZE]; [cite: 2018]
        sortArray(x, sorted, n); [cite: 2018]
        float q25 = sorted[n/4]; [cite: 2018]
        float q75 = sorted[3*n/4]; [cite: 2018]
        
        // ── Écriture features temporelles ─────────────────────────
        features[fi++] = mean; [cite: 2019]
        features[fi++] = std_val; [cite: 2019]
        features[fi++] = rms; [cite: 2020]
        features[fi++] = mx; [cite: 2020]
        features[fi++] = mn; [cite: 2020]
        features[fi++] = mx - mn; // peak-to-peak [cite: 2020, 2021]
        features[fi++] = kurt; [cite: 2021]
        features[fi++] = skew; [cite: 2021]
        features[fi++] = mad; [cite: 2021]
        features[fi++] = q75 - q25;  // IQR [cite: 2022]

        // ── Features fréquentielles ───────────────────────────────
        float energy, domF, meanF, specStd, peakR; [cite: 2022]
        computeSpectralFeatures(x, n, &energy, &domF, &meanF, &specStd, &peakR); [cite: 2023]
        features[fi++] = energy; [cite: 2023]
        features[fi++] = domF; [cite: 2023]
        features[fi++] = meanF; [cite: 2023]
        features[fi++] = specStd; [cite: 2023]
        features[fi++] = peakR; [cite: 2024]
    }
}

// ================================================================
// NORMALISATION + PRÉDICTION
// ================================================================
int predict() {
    // 1. Extraire les 30 features brutes
    float raw_features[N_FEATURES]; [cite: 2024]
    extractFeatures(buf_tension, buf_courant, WINDOW_SIZE, raw_features); [cite: 2025]
    
    // 2. Normaliser : x_norm = (x - mean) / scale
    double scaled[N_FEATURES]; [cite: 2026]
    for (int i = 0; i < N_FEATURES; i++) { [cite: 2027]
        scaled[i] = ((double)raw_features[i] - SCALER_MEAN[i]) / SCALER_SCALE[i]; [cite: 2027]
    }

    // 3. Inférence : score() retourne log-odds de Surcharge
    double result = score(scaled); [cite: 2028]
    
    // 4. Décision
    return (result > 0.0) ? 1 : 0; // 1=Surcharge, 0=Normal [cite: 2029, 2030]
}

// ================================================================
// SETUP & LOOP
// ================================================================
void setup() {
    Serial.begin(115200); [cite: 2030]
    pinMode(PIN_LED_OK,  OUTPUT); [cite: 2030]
    pinMode(PIN_LED_ALT, OUTPUT); [cite: 2030]

    analogReadResolution(12);  // 12 bits sur ESP32 [cite: 2031]

    Serial.println("=== Diagnostic Moteur CC 12V ==="); [cite: 2031]
    Serial.println("Capteurs : Tension (V) + Courant (A)"); 
    Serial.println("Fenetre : 30 pts  |  Stride : 15  |  Modele : Reg. Logistique"); [cite: 2032]
    Serial.println("Remplissage du buffer initial..."); [cite: 2033]
}

void loop() {
    // ── Lecture des 2 capteurs ────────────────────────────────────
    float t_val = readTension(); [cite: 2033]
    float c_val = readCourant(); [cite: 2034]

    // ── Mise à jour du buffer circulaire ─────────────────────────
    buf_tension[buf_index] = t_val; [cite: 2034]
    buf_courant[buf_index] = c_val; [cite: 2035]
    buf_index = (buf_index + 1) % WINDOW_SIZE; [cite: 2035]
    
    if (!buf_full && buf_index == 0) buf_full = true; [cite: 2036]

    // ── Affichage des valeurs brutes ──────────────────────────────
    Serial.printf("T=%.2fV  I=%.3fA  ", t_val, c_val); [cite: 2036]

    // ── Prédiction toutes les STRIDE acquisitions ─────────────────
    if (buf_full) { [cite: 2037]
        stride_count++; [cite: 2037]
        if (stride_count >= STRIDE) { [cite: 2038]
            stride_count = 0; [cite: 2038]
            
            int etat = predict(); [cite: 2039]

            if (etat == 1) { [cite: 2039]
                Serial.println("→ ⚠ SURCHARGE DÉTECTÉE !"); [cite: 2039]
                digitalWrite(PIN_LED_OK,  LOW); [cite: 2040]
                digitalWrite(PIN_LED_ALT, HIGH); [cite: 2040]
            } else {
                Serial.println("→ ✓ Normal"); [cite: 2040]
                digitalWrite(PIN_LED_OK,  HIGH); [cite: 2041]
                digitalWrite(PIN_LED_ALT, LOW); [cite: 2041]
            }
        } else {
            Serial.println("(buffer)"); [cite: 2041]
        }
    } else {
        int remaining = WINDOW_SIZE - buf_index; [cite: 2042]
        Serial.printf("(init : %d pts restants)\n", remaining); [cite: 2043]
    }

    delay(500); // 1 acquisition toutes les 500 ms = 2 Hz [cite: 2043, 2044]
}