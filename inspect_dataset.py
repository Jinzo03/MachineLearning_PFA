import pandas as pd
path = "dataset1\\iot_equipment_monitoring_dataset.csv"
# read only few rows to be fast
df = pd.read_csv(path, nrows=5)
print("Columns:", list(df.columns))
print("\nDtypes:")
print(df.dtypes)
print("\nFirst 5 rows:")
print(df.head())
# Check for missing values
print("\n Missing values per column:")
print(df.isnull().sum())

# list object columns and value counts sample
obj_cols = [c for c in df.columns if df[c].dtype == 'object']
print("\nObject columns (sample value counts):")
for c in obj_cols:
    print("==", c, "==")
    try:
        print(df[c].value_counts().head(10))
    except Exception as e:
        print("  error:", e)
#iyed jloud
