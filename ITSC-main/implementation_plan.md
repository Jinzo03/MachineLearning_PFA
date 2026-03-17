# Notebook Colab : Entraînement ML pour Diagnostic de Moteurs CC 12V (ESP32)

## Contexte

Le dataset ITSC contient des signaux de courant triphasés (3 phases A, B, C) d'un moteur à induction à cage d'écureuil. Il comporte **13 classes** originales (1 sain + 12 niveaux de défauts par phase). Le dossier `Cropped_Signals_SF` contient des fichiers CSV pré-traités de **1000×3 échantillons** (1000 points temporels, 3 phases), organisés en **13 sous-dossiers** avec 5 répétitions chacun.

L'utilisateur souhaite adapter ce dataset pour un **problème de classification à 3 classes** :
- **Normal** (aucun défaut) → `SC_HLT`
- **Court-circuit** → Toutes les classes de défauts ITSC (SC_A1-4, SC_B1-4, SC_C1-4)
- **Surcharge** (simulée à partir des données de défaut sévère, ou une approche expliquée)

> [!IMPORTANT]
> Le dataset ITSC original ne contient **pas** de données de surcharge explicites. Le remapping proposé est :
> - **Normal** = `SC_HLT` (pas de défaut)
> - **Défaut léger** → remappé comme **"surcharge"** (niveaux 1-2, soit 10%-20%)
> - **Défaut sévère** → remappé comme **"court-circuit"** (niveaux 3-4, soit 30%-40%)
>
> L'utilisateur devra confirmer si cette attribution est acceptable pour son cas d'utilisation.

## Contrainte Cible : ESP32

L'algorithme doit être **léger** pour tourner sur ESP32. On privilégiera :
- **Decision Tree** / **Random Forest** (petit nombre d'arbres)
- **K-Nearest Neighbors (KNN)** avec peu de features
- **SVM linéaire** ou **Logistic Regression**
- **Réseau de neurones très simple** (MLP 1 couche cachée, max ~20 neurones)
- Export du modèle pour ESP32 (C header via `m2cgen` ou `sklearn-porter`)

## Proposed Changes

### [NEW] [DC_Motor_Fault_Detection_ESP32.ipynb](file:///c:/Users/iyedd/Downloads/ITSC-main/ITSC-main/DC_Motor_Fault_Detection_ESP32.ipynb)

Notebook Colab complet avec les sections suivantes :

1. **Configuration & Installation** — pip install des dépendances
2. **Chargement du dataset** — Lecture des CSV depuis `Cropped_Signals_SF`, remapping en 3 classes
3. **Visualisation exploratoire** — Signaux bruts, distribution des classes
4. **Extraction de features** — Features temporelles (mean, std, RMS, peak, kurtosis, skewness) et fréquentielles (FFT, énergie spectrale) par phase
5. **Préparation des données** — Normalisation, split train/test, stratification
6. **Entraînement de modèles** :
   - Decision Tree
   - Random Forest (max 10 arbres)
   - KNN
   - SVM linéaire
   - Logistic Regression
   - MLP simplifié
7. **Évaluation & Comparaison** — Accuracy, matrice de confusion, classification report, comparaison taille mémoire
8. **Sélection du meilleur modèle** pour ESP32
9. **Export en C** — Génération de code C avec `m2cgen` pour déploiement ESP32
10. **Instructions d'intégration ESP32** — Guide pour intégrer le code exporté

## Verification Plan

### Automated Tests
- Le notebook sera exécutable de bout en bout dans Google Colab
- Les cellules de vérification afficheront l'accuracy et les métriques de chaque modèle
- Le code C exporté sera compilable et vérifiable

### Manual Verification
- L'utilisateur peut ouvrir le fichier `.ipynb` dans Google Colab et exécuter toutes les cellules séquentiellement
- Vérifier que les matrices de confusion montrent des résultats cohérents (>80% accuracy attendue)
- Vérifier que le fichier C exporté est syntaxiquement correct
