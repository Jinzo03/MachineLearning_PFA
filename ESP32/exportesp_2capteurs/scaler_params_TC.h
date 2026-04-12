#ifndef SCALER_PARAMS_TC_H
#define SCALER_PARAMS_TC_H

#define N_FEATURES  30
#define N_CLASSES   2
#define WINDOW_SIZE 30
#define STRIDE      15 // 2 si voulais plus des données plus rapide mais moins précis


/* Features dans l'ordre :
 * [00] Tension_Mean
 * [01] Tension_Std
 * [02] Tension_RMS
 * [03] Tension_Max
 * [04] Tension_Min
 * [05] Tension_PeakToPeak
 * [06] Tension_Kurtosis
 * [07] Tension_Skewness
 * [08] Tension_MAD
 * [09] Tension_IQR
 * [10] Tension_SpectralEnergy
 * [11] Tension_DominantFreq
 * [12] Tension_MeanFreq
 * [13] Tension_SpectralStd
 * [14] Tension_SpectralPeakRatio
 * [15] Courant_Mean
 * [16] Courant_Std
 * [17] Courant_RMS
 * [18] Courant_Max
 * [19] Courant_Min
 * [20] Courant_PeakToPeak
 * [21] Courant_Kurtosis
 * [22] Courant_Skewness
 * [23] Courant_MAD
 * [24] Courant_IQR
 * [25] Courant_SpectralEnergy
 * [26] Courant_DominantFreq
 * [27] Courant_MeanFreq
 * [28] Courant_SpectralStd
 * [29] Courant_SpectralPeakRatio
 */

const float SCALER_MEAN[N_FEATURES] = {
    9.66242464f,  /* [00] Tension_Mean */
    0.18413464f,  /* [01] Tension_Std */
    9.66424372f,  /* [02] Tension_RMS */
    9.93585366f,  /* [03] Tension_Max */
    9.32812383f,  /* [04] Tension_Min */
    0.60772983f,  /* [05] Tension_PeakToPeak */
    -1.04278764f,  /* [06] Tension_Kurtosis */
    -0.25435116f,  /* [07] Tension_Skewness */
    0.20237756f,  /* [08] Tension_MAD */
    0.31309099f,  /* [09] Tension_IQR */
    84105.32800094f,  /* [10] Tension_SpectralEnergy */
    0.00000000f,  /* [11] Tension_DominantFreq */
    0.00976381f,  /* [12] Tension_MeanFreq */
    72.09259523f,  /* [13] Tension_SpectralStd */
    0.95987686f,  /* [14] Tension_SpectralPeakRatio */
    0.20351176f,  /* [15] Courant_Mean */
    0.00980878f,  /* [16] Courant_Std */
    0.20395379f,  /* [17] Courant_RMS */
    0.21662852f,  /* [18] Courant_Max */
    0.18672420f,  /* [19] Courant_Min */
    0.02990432f,  /* [20] Courant_PeakToPeak */
    0.01331131f,  /* [21] Courant_Kurtosis */
    -0.04013547f,  /* [22] Courant_Skewness */
    0.00542492f,  /* [23] Courant_MAD */
    0.01728940f,  /* [24] Courant_IQR */
    213.61061410f,  /* [25] Courant_SpectralEnergy */
    0.00000000f,  /* [26] Courant_DominantFreq */
    0.00772243f,  /* [27] Courant_MeanFreq */
    1.51383965f,  /* [28] Courant_SpectralStd */
    0.24227200f,  /* [29] Courant_SpectralPeakRatio */
};

const float SCALER_SCALE[N_FEATURES] = {
    0.26608494f,  /* [00] Tension_Mean */
    0.03603698f,  /* [01] Tension_Std */
    0.26617405f,  /* [02] Tension_RMS */
    0.28944320f,  /* [03] Tension_Max */
    0.29324517f,  /* [04] Tension_Min */
    0.19200704f,  /* [05] Tension_PeakToPeak */
    1.30995527f,  /* [06] Tension_Kurtosis */
    0.39391525f,  /* [07] Tension_Skewness */
    0.05994118f,  /* [08] Tension_MAD */
    0.06312754f,  /* [09] Tension_IQR */
    4712.62110571f,  /* [10] Tension_SpectralEnergy */
    1.00000000f,  /* [11] Tension_DominantFreq */
    0.00226352f,  /* [12] Tension_MeanFreq */
    1.98081012f,  /* [13] Tension_SpectralStd */
    0.00783052f,  /* [14] Tension_SpectralPeakRatio */
    0.44215066f,  /* [15] Courant_Mean */
    0.02771968f,  /* [16] Courant_Std */
    0.44292401f,  /* [17] Courant_RMS */
    0.46785640f,  /* [18] Courant_Max */
    0.40834083f,  /* [19] Courant_Min */
    0.07908122f,  /* [20] Courant_PeakToPeak */
    1.41530519f,  /* [21] Courant_Kurtosis */
    0.42670871f,  /* [22] Courant_Skewness */
    0.01268851f,  /* [23] Courant_MAD */
    0.05284381f,  /* [24] Courant_IQR */
    494.22501891f,  /* [25] Courant_SpectralEnergy */
    1.00000000f,  /* [26] Courant_DominantFreq */
    0.02099012f,  /* [27] Courant_MeanFreq */
    3.29022642f,  /* [28] Courant_SpectralStd */
    0.39676823f,  /* [29] Courant_SpectralPeakRatio */
};

#endif
