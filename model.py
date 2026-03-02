import pandas as pd
import os
import matplotlib.pyplot as plt
import seaborn as sns
import math as mt
import numpy as np
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.metrics.pairwise import cosine_similarity
from sklearn.preprocessing import MinMaxScaler
import tensorflow as tf
from tensorflow import keras


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
print("\n Missing values per column:")
print(df.isnull().sum())




# Load model directly
"""from transformers import AutoProcessor, AutoModelForImageTextToText

processor = AutoProcessor.from_pretrained("Qwen/Qwen3.5-397B-A17B")
model = AutoModelForImageTextToText.from_pretrained("Qwen/Qwen3.5-397B-A17B")
messages = [
    {
        "role": "user",
        "content": [
            {"type": "image", "url": "https://huggingface.co/datasets/huggingface/documentation-images/resolve/main/p-blog/candy.JPG"},
            {"type": "text", "text": "What animal is on the candy?"}
        ]
    },
]
inputs = processor.apply_chat_template(
	messages,
	add_generation_prompt=True,
	tokenize=True,
	return_dict=True,
	return_tensors="pt",
).to(model.device)

outputs = model.generate(**inputs, max_new_tokens=40)
print(processor.decode(outputs[0][inputs["input_ids"].shape[-1]:]))"""