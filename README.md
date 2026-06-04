#  TinyML DC Motor Fault Diagnosis — ESP32

> **Real-time overload detection for 12V DC motors running fully on-device — no cloud, no ML library — using Logistic Regression models exported as pure C arithmetic via m2cgen.**

---

##  About

This project deploys trained machine learning models directly onto an ESP32 microcontroller for real-time fault diagnosis of a 12V DC motor. A sliding window of sensor readings is collected at 2 Hz, 45 statistical and spectral features are extracted entirely in firmware, the feature vector is normalized using embedded StandardScaler parameters, and a m2cgen-exported Logistic Regression model produces an inference decision — all without any ML library or network connection. Two hardware configurations are provided: a full 3-sensor variant (voltage, current, speed) and a reduced 2-sensor fallback (voltage and current only).



---

##  System Architecture

```
ESP32 Hardware
│
├── PIN 34 (ADC1) ── Voltage Divider ──────────► Voltage reading (V)
├── PIN 35 (ADC1) ── ACS712-5A ────────────────► Current reading (A)
└── PIN 26 (GPIO) ── Hall Effect Encoder ───────► Speed reading (RPM)  [3-sensor only]
        │
        ▼
Circular Buffer (WINDOW_SIZE=30, STRIDE=15)
        │
        ▼
Feature Extraction (per sensor × number of sensors)
├── 10 temporal features: mean, std, RMS, max, min,
│                         peak-to-peak, kurtosis, skewness, MAD, IQR
└──  5 spectral features: energy, dominant freq, mean freq,
                          spectral std, peak ratio  [simplified DFT]
        │
        ▼
StandardScaler normalization  x_norm = (x − mean) / scale
        │
        ▼
m2cgen Logistic Regression  score() → log-odds
        │
        ├── score > 0  ──► ⚠  SURCHARGE  → Red LED ON
        └── score ≤ 0  ──► ✓  Normal     → Green LED ON
```

---

##  Model Variants

| Variant | Sensors | Features | Model | Balanced Accuracy | Dataset |
|---------|---------|----------|-------|-------------------|---------|
| `exportesp32` | Voltage + Current + Speed | 45 | Logistic Regression | **82.35%** | Real ESP32 data |
| `exportesp_2capteurs` | Voltage + Current | 30 | Logistic Regression | **81.90%** | Real ESP32 data |
| `ITSC-main` *(reference only)* | Voltage + Current + Speed | 45 | Random Forest | **92.31%** | ITSC benchmark dataset |

> **Why Logistic Regression and not Random Forest?**
> The Random Forest trained on the ITSC dataset achieved 92.31% accuracy but its m2cgen export produced a C file too large to fit within ESP32's flash memory constraints. Logistic Regression compiles down to a single arithmetic expression — a dot product of 45 or 30 coefficients — making it the practical choice for constrained deployment while retaining acceptable accuracy on the real sensor dataset.

---

##  Feature Engineering

Each sensor signal contributes **15 features** computed over a 30-sample sliding window with 50% overlap (stride = 15):

### Temporal Features (10 per sensor)

| Feature | Description |
|---------|-------------|
| Mean | Signal average |
| Std | Standard deviation |
| RMS | Root mean square |
| Max | Maximum value |
| Min | Minimum value |
| Peak-to-Peak | `Max − Min` |
| Kurtosis | Excess kurtosis (distribution sharpness) |
| Skewness | Distribution asymmetry |
| MAD | Mean absolute deviation between consecutive samples |
| IQR | Interquartile range (`Q75 − Q25`) |

### Spectral Features (5 per sensor)

Computed via a simplified on-device DFT over the 30-sample window (15 frequency bins, 900 multiplications — negligible on ESP32):

| Feature | Description |
|---------|-------------|
| Spectral Energy | Sum of squared amplitudes across all bins |
| Dominant Frequency | Normalized index of the highest-amplitude bin |
| Mean Frequency | Amplitude-weighted average frequency |
| Spectral Std | Standard deviation of spectral amplitudes |
| Peak Ratio | Dominant bin amplitude / total spectral amplitude |

**Total features: 15 × 3 sensors = 45** (3-sensor) or **15 × 2 sensors = 30** (2-sensor).

---

##  Inference Pipeline Detail

### 1. Circular Buffer

Sensor readings are stored in a fixed-size ring buffer (`buf_tension`, `buf_courant`, `buf_vitesse`). The buffer fills entirely before the first prediction fires, then slides by `STRIDE=15` samples per inference cycle — giving 50% window overlap for denser temporal coverage.

### 2. StandardScaler Normalization

Scaler parameters (mean and scale per feature) are frozen from training and stored as `float` arrays in the header files (`scaler_params_reel.h`, `scaler_params_TC.h`). Normalization runs as:

```c
scaled[i] = ((double)raw_features[i] - SCALER_MEAN[i]) / SCALER_SCALE[i];
```

Features are cast to `double` before normalization to match the precision of the m2cgen-exported model coefficients.

### 3. m2cgen Model Export

m2cgen transpiles a trained scikit-learn Logistic Regression into a single C function. The result is a self-contained `score()` that takes the 45 or 30 normalized doubles and returns a single log-odds value — no external dependency, no heap allocation:

```c
double score(double * input) {
    return BIAS + input[0] * w0 + input[1] * w1 + ... + input[44] * w44;
}
```

Decision threshold: `score > 0.0` → **Surcharge (1)**, else **Normal (0)**.

---

##  Hardware Configuration

### Pin Mapping

| Pin | Role | Component |
|-----|------|-----------|
| GPIO 34 (ADC1) | Voltage reading | Resistive voltage divider |
| GPIO 35 (ADC1) | Current reading | ACS712-5A sensor |
| GPIO 26 | Speed reading (interrupt) | Hall effect encoder |
| GPIO 2 | Green LED | Normal state indicator |
| GPIO 4 | Red LED | Overload state indicator |

### Sensor Calibration Constants

| Parameter | Value | Notes |
|-----------|-------|-------|
| `VCC` | 3.3 V | ESP32 ADC reference |
| `ADC_RESOLUTION` | 4095 | 12-bit ADC |
| `TENSION_DIVISOR` | 0.3 | R2/(R1+R2) — adjust to your divider |
| `ACS712_OFFSET` | 1.65 V | Output at zero current (VCC/2) |
| `ACS712_SENS` | 0.185 V/A | ACS712-5A sensitivity |
| `HALL_PPR` | 20 | Encoder pulses per revolution |

### Acquisition Parameters

| Parameter | Value |
|-----------|-------|
| Sampling rate | 2 Hz (500 ms delay) |
| Window size | 30 samples |
| Stride | 15 samples (50% overlap) |
| Buffer fill time | ~15 seconds |

---

##  Getting Started

### Prerequisites

- [Arduino IDE](https://www.arduino.cc/en/software) with ESP32 board support installed
- ESP32 development board
- ACS712-5A current sensor module
- Resistive voltage divider (R1=20kΩ, R2=6.8kΩ or equivalent ratio ≈ 0.254–0.3)
- *(3-sensor variant only)* Hall effect encoder, 20 PPR

### 1. Select Your Variant

| Situation | Folder to use |
|-----------|---------------|
| You have voltage + current + speed sensors | `ESP32/exportesp32/` |
| You only have voltage + current sensors | `ESP32/exportesp_2capteurs/` |

### 2. Open in Arduino IDE

Open the `.ino` file in the chosen folder. All three files in that folder (`.ino`, `.h` model, `.h` scaler) must remain in the same directory — Arduino automatically includes same-folder headers.

### 3. Adjust Calibration Constants

Edit the calibration `#define` values at the top of the sketch to match your actual voltage divider ratio and ACS712 module variant:

```c
#define TENSION_DIVISOR  0.3f    // Measure R2/(R1+R2) on your board
#define ACS712_OFFSET    1.65f   // Measure output voltage at zero current
#define ACS712_SENS      0.185f  // 0.185 for ACS712-5A, 0.100 for ACS712-20A
```

### 4. Flash the ESP32

Select your ESP32 board and COM port in Arduino IDE, then upload. Open the Serial Monitor at **115200 baud** to observe live readings and inference decisions:

```
=== Diagnostic Moteur CC 12V ===
Fenetre : 30 pts  |  Stride : 15  |  Modele : Reg. Logistique
Remplissage du buffer initial...
T=9.71V  I=0.198A  V=1142.3RPM  (init : 25 pts restants)
...
T=9.68V  I=0.201A  V=1138.6RPM  → ✓ Normal
T=8.92V  I=0.847A  V=432.1RPM   → ⚠ SURCHARGE DÉTECTÉE !
```

---

##  File Reference

| File | Purpose |
|------|---------|
| `exportesp32.ino` | 3-sensor main sketch — sensor reading, buffer management, prediction loop |
| `exportesp2capteurs.ino` | 2-sensor main sketch — same pipeline without speed sensor |
| `model_esp32_reel.h` | m2cgen LR export — 45-feature `score()` function |
| `model_esp32_TC.h` | m2cgen LR export — 30-feature `score()` function |
| `scaler_params_reel.h` | `SCALER_MEAN[45]` and `SCALER_SCALE[45]` arrays + defines |
| `scaler_params_TC.h` | `SCALER_MEAN[30]` and `SCALER_SCALE[30]` arrays + feature index comments |
| `ITSC-main/model_esp32.c` | Reference Random Forest (3 classes, ITSC dataset) — not deployable on ESP32 |
| `dataset jloud/*.c/.h` | Source model and scaler files used to generate the ESP32 header exports |

---

##  Limitations & Known Constraints

- **Binary classification only:** The deployed Logistic Regression distinguishes Normal vs. Overload. The ITSC-trained Random Forest supports a third class (Short-circuit) but cannot be deployed due to code size.
- **Static scaler parameters:** Scaler means and scales are frozen at training time. Significant changes to sensor hardware or motor operating conditions require retraining and re-exporting.
- **2 Hz sampling rate:** Adequate for slow fault evolution (overload), but insufficient for detecting transient or high-frequency electrical faults.
- **No persistence:** Inference decisions are only output to Serial and LEDs. There is no logging, alerting, or communication layer.
- **ADC non-linearity:** The ESP32 ADC has known non-linearity near the rail voltages. For best results, keep measured voltages within 0.15–3.1 V, or apply an ADC correction curve.

---

##  Potential Extensions

- **MQTT/WiFi reporting** — Stream inference decisions to an MQTT broker for remote monitoring dashboards
- **Short-circuit class** — Collect a short-circuit dataset, retrain with a model compact enough for ESP32, and extend classification to 3 classes
- **Higher sampling rate** — Remove the 500 ms delay and use a hardware timer ISR for consistent, faster acquisition
- **ADC calibration** — Apply ESP32 ADC linearization using the `esp_adc_cal` library for more accurate voltage readings
- **OTA model updates** — Use ESP32's OTA capability to push retrained model coefficients without reflashing

---

##  Model Training

Training was conducted in Python using scikit-learn. The trained model and its StandardScaler were exported to C using [m2cgen](https://github.com/BayesWitnesses/m2cgen):

```python
import m2cgen as m2c

# Export Logistic Regression
code = m2c.export_to_c(logistic_regression_model)
with open("model_esp32_reel.c", "w") as f:
    f.write(code)
```

Scaler parameters were extracted manually and written into the corresponding `.h` header files.

---

##  License

This project is released for academic and educational purposes.
