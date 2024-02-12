# channel capacity
import numpy as np
from numpy import linalg as LA
import matplotlib.pyplot as plt

Mr = 4 # Количество приемных антенн
Mt = 4 # Количество передающих антенн
counter = 1000 # Количество итераций моделирования на каждой точке SNR
SNR_dBs = [i for i in range(1, 21)]
C_open = np.empty((len(SNR_dBs), counter))
for c in range(counter):
  H_chan = (np.random.randn(Mr, Mt) + 1j * np.random.randn(Mr, Mt)) / np.sqrt(2) # Генерация матрицы канала
  for idx, SNR_dB in enumerate(SNR_dBs): # Итерация по SNR
    SNR = 10 ** (SNR_dB / 10)
    H_sq = np.dot(H_chan, np.matrix(H_chan, dtype=complex).H) # HH^H
    C_open[idx, c] = np.log2(LA.det(1 + SNR * H_sq / Mt)) # Вычисление спетральной эфф (проп. спос)

C_open_erg = np.mean(C_open, axis = 1) # Для каждого значения SNR вычисление средней проп. спос по всем реализациям канала
print(f'{len(C_open_erg)}')
plt.plot(SNR_dBs, C_open_erg, label = 'Средняя пропускная способность')
plt.title('Средняя пропускная способность')
plt.xlabel('Отношение сигнал-шум -- SNR (dB)')
plt.ylabel('Capacity(bps/Hz)')
plt.legend()
plt.grid()
plt.show()
