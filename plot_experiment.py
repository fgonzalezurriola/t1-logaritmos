import os
import pandas as pd
import matplotlib.pyplot as plt

os.makedirs('results/graphics', exist_ok=True)
df_qs = pd.read_csv('results/quicksort_results.csv')
df_ms = pd.read_csv('results/mergesort_results.csv')

qs_by_m = df_qs.groupby('m').agg({'IO_operations': 'mean','time_seconds': 'mean'}).reset_index()
ms_by_m = df_ms.groupby('m').agg({'IO_operations': 'mean','time_seconds': 'mean'}).reset_index()

# Gráfico 1: I/Os
plt.figure(figsize=(8, 4))
plt.plot(qs_by_m['m'], qs_by_m['IO_operations'], 'o-', label='Quicksort')
plt.plot(ms_by_m['m'], ms_by_m['IO_operations'], 's-', label='Mergesort')
plt.xlabel('m')
plt.ylabel('I/Os')
plt.legend()
plt.tight_layout()
plt.savefig('results/graphics/ios.png', dpi=300)
plt.close()

# Gráfico 2: Tiempo
plt.figure(figsize=(8, 4))
plt.plot(qs_by_m['m'], qs_by_m['time_seconds'], 'o-', label='Quicksort')
plt.plot(ms_by_m['m'], ms_by_m['time_seconds'], 's-', label='Mergesort')
plt.xlabel('m')
plt.ylabel('Tiempo (s)')
plt.legend()
plt.tight_layout()
plt.savefig('results/graphics/time.png', dpi=300)
plt.close()

# Gráfico 3: I/Os en escala logarítmica
plt.figure(figsize=(8, 4))
plt.plot(qs_by_m['m'], qs_by_m['IO_operations'], 'o-', label='Quicksort')
plt.plot(ms_by_m['m'], ms_by_m['IO_operations'], 's-', label='Mergesort')
plt.xlabel('m')
plt.ylabel('I/Os (log)')
plt.yscale('log')
plt.legend()
plt.tight_layout()
plt.savefig('results/graphics/log_ios.png', dpi=300)
plt.close()
