import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('results/arity_results.txt')
plt.figure(figsize=(10,6))
plt.plot(df['Arity'], df["I/O's"], marker='o')
plt.xlabel('Aridad')
plt.ylabel('Cantidad de I/O')
plt.title('Cantidad de I/O vs Aridad en Mergesort Externo')
plt.grid(True)
plt.savefig('results/arity_plot.png')