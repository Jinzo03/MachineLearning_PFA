import re
import numpy as np
import pandas as pd
from scipy import stats
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler, LabelEncoder
from sklearn.ensemble import RandomForestClassifier
from sklearn.neural_network import MLPClassifier
from sklearn.metrics import classification_report, confusion_matrix, accuracy_score
import joblib

DATA_PATH = "dataset1\\iot_equipment_monitoring_dataset.csv"
# Si tu as repéré la colonne label via inspect_dataset.py, mets le nom ici :
LABEL_COL = "label"   # <-- Remplace par le nom exact (ex: "fault_type" ou "Status")