#----------------------------------------------------------------------------------------
# Script to load and read the processed dataset from files separated into folders
#-------------------------------------------------------------------------------------
import argparse
import os
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


# ---------------------------------------------------------------------------
# Loads all CSV files found in the chosen dataset folder.
# - If the folder contains sub-folders, each sub-folder is treated as a class.
# - If it only contains CSV files, the folder itself is treated as one class.
# Example:
#   python Read_dataset_folders.py -d dataset/RAW_Signals
#   python Read_dataset_folders.py -d dataset/Cropped_Signals_SF
# ---------------------------------------------------------------------------
ap = argparse.ArgumentParser()
ap.add_argument(
    "-d",
    "--dataset",
    required=True,
    help="Path to a dataset folder (e.g. dataset/RAW_Signals)",
)
args = vars(ap.parse_args())

# Resolve the dataset path (allowing relative paths from the current working dir)
dataset_path = Path(args["dataset"])
if not dataset_path.is_absolute():
    dataset_path = Path(os.getcwd()) / dataset_path

if not dataset_path.exists():
    raise FileNotFoundError(f"Dataset path not found: {dataset_path}")

# Discover class folders (first-level dirs). If none, use the dataset folder itself.
class_dirs = [p for p in dataset_path.iterdir() if p.is_dir() and not p.name.startswith(".")]
if not class_dirs:
    class_dirs = [dataset_path]

all_class_arrays = []
class_names = []

for class_dir in class_dirs:
    csv_files = sorted([f for f in class_dir.iterdir() if f.suffix.lower() == ".csv"])
    if not csv_files:
        # Skip empty folders but warn the user
        print(f"[WARN] No CSV files in {class_dir}, skipping")
        continue

    # Read every CSV into a numpy array
    signals = []
    for csv_file in csv_files:
        df = pd.read_csv(csv_file, header=None)
        signals.append(df.to_numpy())

    class_array = np.stack(signals, axis=0)
    all_class_arrays.append(class_array)
    class_names.append(class_dir.name)

if not all_class_arrays:
    raise RuntimeError("No CSV data loaded. Check the dataset path and contents.")

Data = np.stack(all_class_arrays, axis=0)

print(f"[INFO] Loaded classes: {class_names}")
print(f"[INFO] Data shape = {Data.shape}")
print(f"[INFO] Example slice Data[0,0,0:10,:] =\n{Data[0,0,0:10,:]}")
