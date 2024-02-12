import numpy as np
import scipy.io as sp
from numpy.linalg import inv
from numpy import linalg as la
def H_LS(yn, Xp, pilot_loc, Nfft, Nps ):
   LS=np.zeros(48,dtype = complex)
   for k in range(0,24):
      LS[(pilot_loc[k]).astype(int)] = yn[(pilot_loc[k]).astype(int)]/srs[k]
   return LS

srs24 = sp.loadmat("srs24.mat")
srs1=list(srs24.values())
srs=srs1[3]
Hmatr = sp.loadmat("H16_4_35.mat")
hl=list(Hmatr.values())
h=hl[3]

Nfft=48
Nps=2
ip=0
pilot_loc = np.zeros(24)
td = np.ones(48,dtype = complex)
y = np.zeros(48,dtype = complex)

for k in range (0,Nfft):
   if k%Nps==1:
      td[k]=srs[ip]
      pilot_loc[ip]=k
      ip=ip+1
snr_db=15
mean_noise=0
H_ls = np.zeros([4,16],dtype = complex)
for i in range(4):
   for j in range(16):
      h1 = h[j,100:148,i]
      y=td*h1
      y2=y**2
      sig_awg_watts=np.mean(y2)
      sig_awg_db=10*np.log10(sig_awg_watts)
      noise_awg_db=sig_awg_db - snr_db
      noise_awg_watts=10**(noise_awg_db/10)
      noise=np.random.normal(mean_noise, np.sqrt(noise_awg_watts),len(y2))
      yn=y+noise
      hLS=H_LS(yn, srs, pilot_loc, Nfft, Nps)
      H_ls[i][j]=hLS[15]

H=H_ls.T

W=np.zeros([16,4],dtype = 'complex_')
W0=np.zeros([16,4],dtype = 'complex_')

H[0]=H[0]/la.norm(H[0])
H[1]=H[1]/la.norm(H[1])
H[2]=H[2]/la.norm(H[2])
H[3]=H[3]/la.norm(H[3])

H=H.T
H1=H.conj().T
H2=H@H1
W=H1@inv(H2)
w1=W[:,0]
w1=w1/la.norm(w1)
w2=W[:,1]
w2=w2/la.norm(w2)
w3=W[:,2]
w3=w3/la.norm(w3)
w4=W[:,3]
w4=w4/la.norm(w4)
h1 = H[0]
h2 = H[1]
r=h2.T@w1
print(r)

W0[:,0]=w1
W0[:,1]=w2
W0[:,2]=w3
W0[:,3]=w4
Ik=H@W0
print(Ik)

s1=-3-5j
s2=1+3j
t=s1*w1+s2*w2
r1=(t.T@h1)/la.norm(h1.T@w1)
r2=(t.T@h2)/la.norm(h2.T@w2)

gain_matrix_zf = np.abs(H@W0) ** 2
temp_zf = 1 + np.sum(gain_matrix_zf , 1)
temp_zf = np.diag(temp_zf) * np.ones(4)
int_power_zf = temp_zf - gain_matrix_zf
SINR_matrix_zf = gain_matrix_zf / int_power_zf
rate_zf = np.sum(np.log2(1 + snr_db*np.diag(SINR_matrix_zf)))
