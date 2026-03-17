# Walkthrough — Diagnostic Moteur CC 12V (ESP32)

## Ce qui a été créé

### 1. [DC_Motor_Fault_Detection_ESP32.ipynb](file:///c:/Users/iyedd/Downloads/ITSC-main/ITSC-main/DC_Motor_Fault_Detection_ESP32.ipynb)
Notebook Colab principal (37 cellules) pour entraîner et comparer 8 modèles ML.
- **Approche** : Les 3 signaux sont mappés aux futurs capteurs physiques du système embarqué (Courant, Tension, Vitesse).
- **Structure** : Professionnelle, organisée en 11 sections, incluant corrélations, distributions et F1-score.

### 2. [Rapport_Diagnostic_Moteur_CC_12V.md](file:///c:/Users/iyedd/Downloads/ITSC-main/ITSC-main/Rapport_Diagnostic_Moteur_CC_12V.md)
Rapport technique détaillé de 7 chapitres expliquant l'intégralité de la méthodologie (choix du dataset, feature engineering, modèles, déploiement ESP32).

### 3. [DC_Motor_Fault_Detection_ESP32_3Phases.ipynb](file:///c:/Users/iyedd/Downloads/ITSC-main/ITSC-main/DC_Motor_Fault_Detection_ESP32_3Phases.ipynb)
Notebook Colab alternatif (38 cellules) reprenant l'approche originale du dataset ITSC.
- **Approche** : Analyse directe des 3 phases de courant (Phase A, Phase B, Phase C) sans projection vers d'autres types de capteurs.
- **Contenu** : Même rigueur et structure (feature engineering, 8 modèles ML, GridSearchCV, export ESP32), mais descriptions et visualisations adaptées pour des courants triphasés.

## Remapping des Classes (Commun aux deux approches)

| Classe cible | Classes ITSC sources (1000x3) | Fichiers | Signification |
|---|---|---|---|
| **Normal** | `SC_HLT` | 5 | État sain, aucun défaut |
| **Surcharge** | Niveaux 1-2 (10-20%) des 3 phases | 30 | Simulation de surcharge (défauts légers) |
| **Court-circuit** | Niveaux 3-4 (30-40%) des 3 phases | 30 | Simulation de court-circuit (défauts sévères) |

## Modèles Entraînés et Optimisés

1. **Decision Tree** : Ultra-léger, export direct en C
2. **Random Forest (10 arbres)** : Meilleure robustesse, reste léger
3. **SVM Linéaire** : Hyperplan simple, faible mémoire RAM requise
4. **Logistic Regression** : Algorithme le plus frugal en calcul
5. **Perceptron Multicouche (MLP)** : Architecture légère (< 20 neurones cachés)
6. **HistGradientBoosting** : Souvent le plus performant en termes d'accuracy
7. **XGBoost (50 arbres)** : Très performant pour les signaux tabulaires extraits
8. *KNN (exclu pour l'esprit ESP32, nécessite le stockage du dataset entier)*

## Stratégie d'Extraction des Features (ESP32-compatible)

Pour chaque signal (Capteur ou Phase), **15 features** sont calculées en C sur le microcontrôleur :

* **Temporelles (10)** : Moyenne, Écart-type (Std), Valeur Efficace (RMS), Max, Min, Peak-to-Peak, Kurtosis, Skewness, Différence Absolue Moyenne (MAD), Intervalle Interquartile (IQR).
* **Fréquentielles (5 via FFT)** : Énergie Spectrale, Fréquence Dominante, Fréquence Moyenne, Dispersion Spectrale, Ratio du Pic Spectral.

Soit un total de **45 caractéristiques** envoyées en entrée des modèles après normalisation avec `StandardScaler`.

## Guide d'Utilisation

1. Ouvrez l'un des notebooks `.ipynb` sur **Google Colab**.
2. Décommentez la cellule d'installation initiale (`!git clone ...`).
3. Modifiez `DATASET_ROOT` pour pointer vers `ITSC/dataset/Cropped_Signals_SF/Cropped_Signals_SF`.
4. Exécutez ; le script optimisera tous les modèles, affichera les graphiques et générera :
   - `model_esp32.c` : Code C du meilleur modèle
   - `scaler_params.h` : Paramètres pour la normalisation en temps réel sur ESP32
