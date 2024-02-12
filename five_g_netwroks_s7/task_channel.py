import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
from scipy.signal import kaiserord, lfilter, firwin, freqz
from scipy import fftpack
from numpy.linalg import inv
from numpy.linalg import svd

def nnoise(s, snr):
  s2 = s ** 2
  sig_avg_watts = np.mean(s2)
  sig_avg_db = 10 * np.log10(sig_avg_watts)
  noise_avg_db = sig_avg_db - snr_db
  noise_avg_watts = 10 ** (noise_avg_db / 10)
  mean_noise = 0
  noise_volts = np.random.normal(mean_noise, np.sqrt(noise_avg_watts), len(s))
  return noise_volts

fs = 1e5
ts = 1 / fs
t = np.arange(0, 1, ts) # вектор отсчетов времени с шагом инт. дискр
mean = 0
ch_avg_watts = 1
nr = 2 # приемные антенны
nt = 2 # передающие антенны
hr2 = np.random.normal(mean, np.sqrt(ch_avg_watts), len(t))
hi2 = np.random.normal(mean, np.sqrt(ch_avg_watts), len(t))
h22 = (hr2 + 1j * hi2) / np.sqrt(2)
plt.plot(t, h22)
plt.show()

h22 = np.reshape(h22, ((2, 2, 25000)))
d1 = np.array([-3 + 5 * 1j, -1 -1j])

# rec diversity
d = d1[0]
h = h22[:, 0, 0]
st = h * d
snr_db = 22
ns = nnoise(st, snr_db)
print(f' asd {len(h22[:,0,0])} {len(h22[0,:,0])} {len(h22[0,0,:])}')
plt.plot(h22)
plt.show()
quit()
r = st + ns
s = np.conjugate(h)@r / np.sum((np.abs(h) ** 2))
# tr diversity
htr = h22[0, :, 0]
w = np.conjugate(htr) / np.sum((np.abs(htr) ** 2))
d = d1[0]
x = w * d
str1 = np.transpose(htr)
snr_db = 22
nst = nnoise(str1, snr_db)
rtr = (str1 + nst)@x

# spatial multiplexing
h2 = h22[:, :, 0]
smt = h2@d1
snr_db = 16
ns2 = nnoise(smt, snr_db)
r2 = smt + ns2
h2i = inv(h2)
smr = h2i@r2

# channel capacity
import numpy as np
from numpy import linalg as LA
import matplotlib.pyplot as plt

Mr = 8 # Количество приемных антенн
Mt = 8 # Количество передающих антенн
counter = 1000 # Количество итераций моделирования на каждой точке SNR
SNR_dBs = [i for i in range(1, 21)]
C_open = np.empty((len(SNR_dBs), counter))
for c in range(counter):
  H_chan = (np.random.randn(Mr, Mt) + 1j * np.random.randn(Mr, Mt)) / np.sqrt(2) # Генерация матрицы канала
  for idx, SNR_dB in enumerate(SNR_dBs): # Итерация по SNR
    SNR = 10 ** (SNR_dB / 10)
    H_sq = np.dot(H_chan, np.matrix(H_chan, dtype=complex).H) # HH^H
    C_open[idx, c] = np.log2(LA.det(1 + SNR * H_sq / Mt)) # Вычисление спетральной эфф (проп . спос)

C_open_erg = np.mean(C_open, axis = 1) # Для каждого значения SNR вычисление средней проп. спос по всем реализациям канала
plt.plot(SNR_dBs, C_open_erg, label = 'Средняя пропускная способность')
plt.title('Средняя пропускная способность')
plt.xlabel('Отношение сигнал-шум -- SNR (dB)')
plt.ylabel('Capacity(bps/Hz)')
plt.legend()
plt.grid()
plt.show()
