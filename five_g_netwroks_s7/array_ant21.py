import numpy as np
import matplotlib.pyplot as plt
c=10**8;
f=2.5*10**9;
lam = c/f;
d = 0.5*lam
N=4
def array_response_vector(array,phi, d, lam):
    #N = array.shape
    v = np.exp(1j*2*np.pi*array*d/lam*np.sin(phi))
    return v


array = np.arange(0,N)

phi = np.linspace(-np.pi,np.pi,360)
#phi = np.linspace(-np.pi , np.pi , 100) #100 значений углов от -180 до 180
psi = 2*(np.pi * d / lam) * np.sin(phi)
teta0 =   45*np.pi/180 # направление прихода
delta = -(2*np.pi*d/lam)*np.sin(teta0)

AF = 1 +  np.exp(1j*psi)  +  np.exp(1j*2*psi)  + np.exp(1j*3*psi)  #коэфф АР = сумме элементов управл вектора a(teta)
AFs = 1 +  np.exp(1j*(psi+delta))  +  np.exp(1j*2*(psi+delta))  + np.exp(1j*3*(psi+delta)) #+  np.exp(1j*4*(psi+delta))+ np.exp(1j*5*(psi+delta))+ np.exp(1j*6*(psi+delta))+ np.exp(1j*7*(psi+delta))
plt.figure(1)
AF = np.abs(AF)**2
AFs = np.abs(AFs)**2
plt.polar(phi, AFs)

k=2*np.pi/lam

AF1= np.sin((N*k*d/2)*(np.sin(phi))) #(np.sin(phi)-np.sin(teta0)))
AF2=N*np.sin((k*d/2)*(np.sin(phi))) #(np.sin(phi)-np.sin(teta0)))
AFn=AF1/AF2
plt.figure(2)
AFn = np.abs(AFn)**2
plt.polar(phi, AFn)

#Angles = np.linspace(-np.pi,np.pi,360)
#numAngles = Angles.size\
numAngles = phi.size

Ar = np.zeros([N,numAngles], dtype=complex)
for j in range(numAngles):
   Ar[:,j] = array_response_vector(array,phi[j], d, lam)


AF11=np.zeros(phi.size, dtype=complex)
AF11=np.sum(Ar, axis=0)
AF11a = np.abs(AF11)**2
plt.figure(3)
plt.polar(phi, AF11a)
plt.show()
