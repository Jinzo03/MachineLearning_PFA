import pandas as pd
import os
import matplotlib.pyplot as plt
import seaborn as sns
import math as mt
import numpy as np
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.metrics.pairwise import cosine_similarity
from sklearn.preprocessing import MinMaxScaler



dataset_path = r"dataset1\iot_equipment_monitoring_dataset.csv"  

df = pd.read_csv(dataset_path)


# Basic exploration
print(" Dataset Preview:")
print(df.head(10))

#print("\n📄 Dataset Info:")
#print(df.info(10))

#print("\n📈 Basic Stats:")
#print(df.describe(10))

# Check for missing values
#print("\n Missing values per column:")
#print(df.isnull().sum())




