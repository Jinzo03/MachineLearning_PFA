import re
import sys
from typing import List
import pandas as pd
import numpy as np
from sklearn.impute import KNNImputer
from sklearn.preprocessing import StandardScaler

# ---------- CONFIG ----------
INPUT = "dataset1\\iot_equipment_monitoring_dataset.csv"
OUTPUT = "dataset1\\iot_equipment_monitoring_dataset_cleaned.csv"

# thresholds
COL_MISSING_COL_THRESHOLD = 0.5   # candidate for removal if >50% missing (only numeric by default)
ROW_MISSING_THRESHOLD = 0.5       # drop rows with >50% columns missing (after imputations)
WINDOW_NAN_RATIO_DROP = 0.2       # for detected window columns: drop row if >20% NaN inside window
USE_KNN_FOR_NUMERIC = False       # set True to use KNN on numeric columns
KNN_NEIGHBORS = 5

# Protection / preservation:
# exact column names (case-sensitive) you never want dropped or turned into UNKNOWN
PRESERVE_COLUMNS: List[str] = [
    "fault_type", "fault typ", "fault", "label", "Sensor_ID"
]
# regex patterns: if a column name matches any pattern, preserve it
PRESERVE_REGEX = [r"fault", r"type", r"status", r"fault[_\s]?typ"]

# Label candidates (prefer explicit names)
LABEL_CANDIDATES = ['label','Label','target','class','fault','status','Sensor_ID']

# ---------- Helper functions ----------
def matches_preserve(name: str) -> bool:
    if name in PRESERVE_COLUMNS:
        return True
    for pat in PRESERVE_REGEX:
        if re.search(pat, name, flags=re.I):
            return True
    return False

def detect_label(df: pd.DataFrame) -> str:
    # 1) check explicit candidates
    for c in LABEL_CANDIDATES:
        if c in df.columns:
            return c
    # 2) prefer object/category low-cardinality columns
    candidate = None
    min_ratio = 1.1
    n = len(df)
    for c in df.columns:
        nunique = df[c].nunique(dropna=True)
        if nunique <= 1:
            continue
        ratio = nunique / max(1, n)
        # prefer low cardinality and object/category types
        score = ratio
        if df[c].dtype == "object" or str(df[c].dtype).startswith("category"):
            score *= 0.5
        if score < min_ratio:
            min_ratio = score
            candidate = c
    return candidate

# ---------- Main ----------
def main():
    print("Loading:", INPUT)
    df_orig = pd.read_csv(INPUT)  # keep original for recovery if needed
    df = df_orig.copy()
    n_rows, n_cols = df.shape
    print(f"Loaded {n_rows} rows x {n_cols} cols")

    # quick convert of obvious empty-string placeholders to NaN
    df.replace({"": np.nan, "NA": np.nan, "N/A": np.nan}, inplace=True)

    # 2) missingness report
    missing = df.isnull().sum().sort_values(ascending=False)
    missing_pct = (missing / len(df)).sort_values(ascending=False)
    report = pd.concat([missing, missing_pct], axis=1)
    report.columns = ['n_missing', 'pct_missing']
    print("\nTop missing columns:")
    print(report.head(20))

    # 3) Decide column-level drops carefully
    cols_to_drop = []
    cols_to_keep_but_fill = []
    for col, pct in report['pct_missing'].items():
        if pct > COL_MISSING_COL_THRESHOLD:
            # if column should be preserved by name/regex -> keep and mark for filling or manual review
            if matches_preserve(col):
                print(f"Preserving high-missing column (by preserve rules): {col} ({pct:.2%} missing) -> will fill/keep")
                cols_to_keep_but_fill.append(col)
                continue
            # If column is numeric -> candidate to drop
            if pd.api.types.is_numeric_dtype(df[col]):
                cols_to_drop.append(col)
            else:
                # it's object/categorical: keep and fill with UNKNOWN rather than dropping
                print(f"High-missing categorical column kept and will be filled with 'UNKNOWN': {col} ({pct:.2%} missing)")
                cols_to_keep_but_fill.append(col)

    if cols_to_drop:
        print(f"\nDropping numeric columns with >{int(COL_MISSING_COL_THRESHOLD*100)}% missing: {cols_to_drop}")
        df.drop(columns=cols_to_drop, inplace=True)
    else:
        print("\nNo numeric columns dropped by threshold.")

    # 4) label detection & preserve
    label_col = detect_label(df)
    print("Detected label column:", label_col)

    # 5) drop rows without label (only if label exists)
    if label_col is not None:
        n_before = len(df)
        df = df[df[label_col].notnull()]
        print(f"Dropped {n_before - len(df)} rows with missing label ({label_col}).")

    # 6) detect window-like groups (I_0, I_1, ... or Current-0, etc.)
    cols = df.columns.tolist()
    window_groups = {}
    pattern = re.compile(r'(.+?)[_\-]?([0-9]+)$')
    for c in cols:
        m = pattern.match(c)
        if m:
            prefix = m.group(1)
            window_groups.setdefault(prefix, []).append(c)
    # keep only groups with >= 3 or more columns (adjusted)
    window_groups = {k: sorted(v, key=lambda x: int(pattern.match(x).group(2)))
                     for k, v in window_groups.items() if len(v) >= 3}
    print("\nDetected window-like groups (prefix: n_cols):")
    for k, v in window_groups.items():
        print(" -", k, ":", len(v))

    # 7) process each window group
    rows_to_drop_idx = set()
    for prefix, cols_win in window_groups.items():
        # Coerce window columns to numeric safely (non-numeric -> NaN, but we will not drop whole column)
        df[cols_win] = df[cols_win].apply(pd.to_numeric, errors='coerce')
        win = df[cols_win].values  # numpy array with numeric dtype + NaNs

        nan_counts = np.isnan(win).sum(axis=1)
        nan_ratio = nan_counts / win.shape[1]
        drop_mask = nan_ratio > WINDOW_NAN_RATIO_DROP
        n_drop = int(drop_mask.sum())
        print(f"Window '{prefix}': {win.shape[1]} cols, dropping {n_drop} rows with >{int(WINDOW_NAN_RATIO_DROP*100)}% NaN in window")
        rows_to_drop_idx.update(df.index[drop_mask].tolist())

        # Interpolate remaining rows across columns (vectorized)
        # pandas interpolate with axis=1 performs per-row interpolation across the columns
        # apply limit_direction='both' so edges are filled from nearest known value
        try:
            interp_block = df.loc[~drop_mask, cols_win].interpolate(axis=1, method='linear', limit_direction='both')
            # assign back
            df.loc[~drop_mask, cols_win] = interp_block
        except Exception as e:
            # fallback to safe per-row interpolation if something fails
            print(f"Warning: vectorized interpolation failed for window '{prefix}': {e}\nFalling back to row-wise interpolation.")
            for idx_row in df.loc[~drop_mask].index:
                row_vals = df.loc[idx_row, cols_win].values.astype(float)
                mask = ~np.isnan(row_vals)
                if mask.sum() == 0:
                    continue
                x = np.arange(len(row_vals))
                interp = np.interp(x, x[mask], row_vals[mask])
                df.loc[idx_row, cols_win] = interp

    # drop rows flagged by any window rule
    if rows_to_drop_idx:
        print(f"Dropping total {len(rows_to_drop_idx)} rows flagged by window rules")
        df.drop(index=list(rows_to_drop_idx), inplace=True)

    # 8) Numeric columns: compute missing and decide actions
    numeric_cols = df.select_dtypes(include=[np.number]).columns.tolist()
    print("\nNumeric columns to check:", numeric_cols[:40])
    col_missing_after = df[numeric_cols].isnull().sum()
    cols_small_missing = col_missing_after[(col_missing_after > 0) & (col_missing_after / len(df) <= 0.2)].index.tolist()
    cols_big_missing = col_missing_after[(col_missing_after / len(df) > 0.2)].index.tolist()
    print("Columns with small missing (<=20%):", cols_small_missing)
    print("Columns with big missing (>20%):", cols_big_missing)

    # Impute medians for small missing
    for c in cols_small_missing:
        med = df[c].median()
        df[c].fillna(med, inplace=True)
        print(f"Imputed median for {c} -> {med}")

    # For big-missing numeric columns: drop them by default unless they are preserved
    big_to_drop = [c for c in cols_big_missing if not matches_preserve(c)]
    big_to_keep = [c for c in cols_big_missing if matches_preserve(c)]
    if big_to_drop:
        print("Dropping numeric columns with >20% missing (and not preserved):", big_to_drop)
        df.drop(columns=big_to_drop, inplace=True)
    if big_to_keep:
        print("Preserving numeric columns with >20% missing (by preserve rules):", big_to_keep)
        # optionally, leave them with NaNs or fill with median; here we leave them for user decision

    # 9) Categorical columns: fill object/category columns (except label) with 'UNKNOWN' if they have NaNs
    cat_cols = df.select_dtypes(include=['object', 'category']).columns.tolist()
    cat_cols = [c for c in cat_cols if c != label_col]
    for c in cat_cols:
        nnull = df[c].isnull().sum()
        if nnull > 0:
            # if column is in preserve list, we still fill but print a stronger warning
            if matches_preserve(c):
                print(f"Preserved categorical {c} had {nnull} NaNs -> filling with 'UNKNOWN' (preserved column)")
            df[c].fillna('UNKNOWN', inplace=True)

    # Also fill columns we earlier marked to keep-but-fill (from the high-missing stage)
    for c in cols_to_keep_but_fill:
        if c in df.columns:
            if pd.api.types.is_numeric_dtype(df[c]):
                # numeric preserve: fill with median (but warn)
                med = df[c].median()
                df[c].fillna(med, inplace=True)
                print(f"(Preserved) imputed median for {c} -> {med}")
            else:
                df[c].fillna('UNKNOWN', inplace=True)
                print(f"(Preserved) filled categorical {c} NaNs -> 'UNKNOWN'")

    # 10) final global row threshold
    row_nan_ratio = df.isnull().mean(axis=1)
    n_before = len(df)
    df = df[row_nan_ratio <= ROW_MISSING_THRESHOLD]
    print(f"Dropped {n_before - len(df)} rows with >{int(ROW_MISSING_THRESHOLD*100)}% columns missing")

    # 11) Optionally KNN-impute numeric columns (with scaling)
    if USE_KNN_FOR_NUMERIC:
        num_cols = df.select_dtypes(include=[np.number]).columns.tolist()
        if num_cols:
            print("Applying KNN imputer to numeric columns (with scaling)...")
            scaler = StandardScaler()
            num_data = df[num_cols].values
            # keep mask of columns with any missing (KNN will fill all)
            knn = KNNImputer(n_neighbors=KNN_NEIGHBORS)
            scaled = scaler.fit_transform(np.nan_to_num(num_data, nan=0.0))  # placeholder
            # Better: feed raw numeric data (KNNImputer expects NaN placeholders)
            scaled = scaler.fit_transform(df[num_cols].astype(float))
            imputed_scaled = knn.fit_transform(scaled)
            imputed = scaler.inverse_transform(imputed_scaled)
            df[num_cols] = imputed
            print("KNN imputation completed.")

    # 12) final diagnostics & save
    final_missing = df.isnull().sum().sort_values(ascending=False)
    print("\nFinal missing per column (top 20):")
    print(final_missing.head(20))
    print("Final shape:", df.shape)
    print("Saving cleaned dataset to:", OUTPUT)
    df.to_csv(OUTPUT, index=False)

    # Save summary of columns that were dropped so you can recover if needed
    dropped_columns = set(df_orig.columns) - set(df.columns)
    if dropped_columns:
        print("\nColumns dropped during cleaning (saved to dropped_columns.txt):")
        for c in dropped_columns:
            print(" -", c)
        with open("dropped_columns.txt", "w", encoding="utf-8") as fh:
            for c in sorted(dropped_columns):
                fh.write(c + "\n")
    else:
        print("\nNo columns were dropped.")

if __name__ == "__main__":
    main()