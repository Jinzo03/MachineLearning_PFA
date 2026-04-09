#ifndef MODEL_ESP32_REEL_H
#define MODEL_ESP32_REEL_H

/*
 * Modele     : Regression Logistique
 * Capteurs   : Tension (V) + Courant (A)
 * N features : 30 (15 x 2 capteurs)
 * Bal. Acc.  : 0.8190
 * Fenetre    : 30 echantillons
 * Stride     : 15 echantillons
 * Classes    : 0=Normal, 1=Surcharge
 * score() retourne log-odds : >0 = Surcharge, <=0 = Normal
 */

double score(double * input) {
    return 0.03003006960505197 + input[0] * -0.2712265771283173 + input[1] * -2.0041227584887458 + input[2] * -0.28008536011335955 + input[3] * -0.07535119979855853 + input[4] * -0.38975888216657395 + input[5] * 0.4816751267682722 + input[6] * -1.4203800873062837 + input[7] * -0.14190183696943465 + input[8] * -0.19245006093524053 + input[9] * -0.6234654667369987 + input[10] * -0.30655294400325706 + input[11] * 0.0 + input[12] * 0.013632930034306927 + input[13] * -0.3342117395795566 + input[14] * -2.7959063872076957 + input[15] * -0.34167499602665025 + input[16] * 1.1128497436229832 + input[17] * -0.33575577514652716 + input[18] * -0.2222608856722623 + input[19] * -0.4549602872152218 + input[20] * 1.034287113364287 + input[21] * 0.6097949274524089 + input[22] * -0.18872404537260276 + input[23] * 0.07092089515615438 + input[24] * 0.6813338015020174 + input[25] * -0.8989680765971336 + input[26] * 0.0 + input[27] * -1.432299241257737 + input[28] * -0.35135747506146336 + input[29] * 2.492184940164123;
}
#endif // MODEL_ESP32_REEL_H