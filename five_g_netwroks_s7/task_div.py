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

h22 = np.reshape(h22, ((2, 2, 25000)))
d1 = np.array([-3 + 5 * 1j, -1 -1j])

# rec diversity
d = d1[0]
h = h22[:, 0, 0]
st = h * d
snr_db = 22
ns = nnoise(st, snr_db)
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
