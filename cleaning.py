import pandas as pd
import numpy as np
from sklearn.impute import KNNImputer

INPUT = "iot_equipment_monitoring_dataset.csv"
OUTPUT = "iot_equipment_monitoring_dataset_cleaned.csv"

# paramètres 
COL_MISSING_COL_THRESHOLD = 0.5   # supprimer colonne si > 50% missing
ROW_MISSING_THRESHOLD = 0.5       # supprimer ligne si >50% des colonnes (après colonnes supprimées) sont NaN
WINDOW_NAN_RATIO_DROP = 0.2       # pour colonnes fenêtre (I_0..I_N) : si >20% NaN dans la fenêtre, supprimer la ligne
USE_KNN_FOR_NUMERIC = False       # si True, utilise KNNImputer pour colonnes numériques restantes

# 1) charger (rapide)
df = pd.read_csv(INPUT)
n_rows, n_cols = df.shape
print(f"Loaded {INPUT} with {n_rows} rows and {n_cols} cols")

# 2) rapport missing
missing = df.isnull().sum().sort_values(ascending=False)
missing_pct = (missing / len(df)).sort_values(ascending=False)
report = pd.concat([missing, missing_pct], axis=1)
report.columns = ['n_missing', 'pct_missing']
print("\nMissing per column (top 20):")
print(report.head(20))

# 3) supprimer colonnes très vides
cols_to_drop = report[report['pct_missing'] > COL_MISSING_COL_THRESHOLD].index.tolist()
if cols_to_drop:
    print(f"\nDropping {len(cols_to_drop)} columns with >{int(COL_MISSING_COL_THRESHOLD*100)}% missing:")
    print(cols_to_drop)
    df = df.drop(columns=cols_to_drop)
else:
    print("\nNo columns dropped by threshold.")

# 4) identifier colonne label (assume une colonne catégorielle existante)
#    - si tu connais son nom, mets-le ici ; sinon on essaie de deviner
LABEL_CANDIDATES = ['label','Label','target','class','fault','status','Sensor_ID']  # adapte si besoin
label_col = None
for c in LABEL_CANDIDATES:
    if c in df.columns:
        label_col = c
        break
# si introuvable, repère la colonne object la plus plausible
if label_col is None:
    obj_cols = [c for c in df.columns if df[c].dtype == 'object']
    if obj_cols:
        label_col = obj_cols[0]
print("Detected label column:", label_col)

# 5) supprimer lignes sans label
if label_col is not None:
    n_before = len(df)
    df = df[ df[label_col].notnull() ]
    print(f"Dropped {n_before - len(df)} rows with missing label")

# 6) détecter colonnes fenêtre (time-series per-row) : pattern I_0,I_1... ou Current_0...
#    heuristique : colonnes contenant '0','1','2' suffixes and same prefix
cols = df.columns.tolist()
window_groups = {}  # prefix -> list cols
import re
pattern = re.compile(r'(.+?)[_\-]?([0-9]+)$')  # capture prefix + index suffix
for c in cols:
    m = pattern.match(c)
    if m:
        prefix = m.group(1)
        idx = m.group(2)
        window_groups.setdefault(prefix, []).append(c)
# keep only groups with >=5 cols
window_groups = {k:sorted(v, key=lambda x: int(pattern.match(x).group(2))) for k,v in window_groups.items() if len(v) >= 5}
print("\nDetected window-like groups (prefix: n_cols):")
for k,v in window_groups.items():
    print(" -", k, ":", len(v))

# 7) process each window group: interpolation per row or drop if too many NaN
rows_to_drop_idx = set()
for prefix, cols_win in window_groups.items():
    # convert group to numpy
    win = df[cols_win].values
    # compute per-row nan ratio
    nan_counts = np.isnan(win).sum(axis=1)
    nan_ratio = nan_counts / win.shape[1]
    # mark rows to drop
    drop_mask = nan_ratio > WINDOW_NAN_RATIO_DROP
    n_drop = drop_mask.sum()
    print(f"Window {prefix}: {win.shape[1]} cols, dropping {n_drop} rows with >{int(WINDOW_NAN_RATIO_DROP*100)}% NaN in window")
    rows_to_drop_idx.update(df.index[drop_mask].tolist())
    # interpolate remaining NaNs row-wise (linear)
    # we'll apply per-row interpolation: replace NaN with linear interpolation across the window
    for i, idx_row in enumerate(df.index):
        if drop_mask[i]:
            continue
        row_vals = df.loc[idx_row, cols_win].values.astype(float)
        if np.isnan(row_vals).any():
            # linear interpolate in 1D
            x = np.arange(len(row_vals))
            mask = ~np.isnan(row_vals)
            if mask.sum() == 0:
                # all NaN — will be removed by drop mask already
                continue
            interp = np.interp(x, x[mask], row_vals[mask])
            df.loc[idx_row, cols_win] = interp

# drop rows marked
if rows_to_drop_idx:
    print(f"Dropping total {len(rows_to_drop_idx)} rows flagged by window rule")
    df = df.drop(index=list(rows_to_drop_idx))

# 8) For remaining numeric columns, impute median (robust) if small amount of missing
numeric_cols = df.select_dtypes(include=[np.number]).columns.tolist()
print("\nNumeric columns to check:", numeric_cols[:40])
col_missing_after = df[numeric_cols].isnull().sum()
cols_small_missing = col_missing_after[(col_missing_after > 0) & (col_missing_after / len(df) <= 0.2)].index.tolist()
cols_big_missing = col_missing_after[(col_missing_after / len(df) > 0.2)].index.tolist()
print("Columns with small missing (<=20%):", cols_small_missing)
print("Columns with big missing (>20%):", cols_big_missing)

# impute median for small missing
for c in cols_small_missing:
    med = df[c].median()
    df[c].fillna(med, inplace=True)
    print(f"Imputed median for {c} -> {med}")

# for big-missing numeric columns: by default drop them (or you can KNN impute)
if cols_big_missing:
    print("Dropping numeric columns with >20% missing:", cols_big_missing)
    df.drop(columns=cols_big_missing, inplace=True)

# 9) categorical columns: fill with 'UNKNOWN'
cat_cols = df.select_dtypes(include=['object','category']).columns.tolist()
cat_cols = [c for c in cat_cols if c != label_col]  # keep real labels intact
for c in cat_cols:
    nnull = df[c].isnull().sum()
    if nnull > 0:
        df[c].fillna('UNKNOWN', inplace=True)
        print(f"Filled {nnull} NaN in categorical column {c} with 'UNKNOWN'")

# 10) final global row threshold: drop rows with too many NaN
row_nan_ratio = df.isnull().mean(axis=1)
n_before = len(df)
df = df[row_nan_ratio <= ROW_MISSING_THRESHOLD]
print(f"Dropped {n_before - len(df)} rows with >{int(ROW_MISSING_THRESHOLD*100)}% columns missing")

# Option: KNN imputation for numeric small gaps if desired
if USE_KNN_FOR_NUMERIC:
    print("Applying KNN imputer to numeric columns...")
    knn = KNNImputer(n_neighbors=5)
    df[numeric_cols] = knn.fit_transform(df[numeric_cols])

# 11) final report and save
final_missing = df.isnull().sum().sort_values(ascending=False)
print("\nFinal missing per column (top 20):")
print(final_missing.head(20))
print("Final shape:", df.shape)
df.to_csv(OUTPUT, index=False)
print("Saved cleaned dataset to", OUTPUT)