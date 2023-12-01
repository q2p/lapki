import numpy as np
import matplotlib.pyplot as plt
c=10**8;
f=2.5*10**9;
lam = c/f;
d = 0.5*lam

def array_response_vector(array, phi, d, lam):
  #N = array.shape
  v = np.exp(1j*2*np.pi*array*d/lam*np.sin(phi))
  return v

array4 = np.arange(0,4)
array8 = np.arange(0,8)

phi = np.linspace(-np.pi, np.pi, 360*8)
phi_45 = 60 * np.pi / 180

v4 = array_response_vector(array4, phi_45, d, lam)
print("Направленные векторы для 4 элементов:")
for elem in v4:
  print(elem)

v8 = array_response_vector(array8, phi_45, d, lam)
print("Направленные векторы для 8 элементов:")
for elem in v8:
  print(elem)

AF4 = np.sum(v4)
AF8 = np.sum(v8)

print("\nКоэффициент AF в виде суммы для 4 элементов, 60 градусов:\n", AF4)
print("Коэффициент AF  в виде суммы для 8 элементов, 60 градусов:\n", AF8)

teta0 =   60*np.pi/180
psi = 2 * (np.pi * d / lam) * np.sin(teta0)

AF_4 = 1 + np.exp(1j*psi) + np.exp(1j*2*psi) + np.exp(1j*3*psi) # коэфф АР = сумме элементов управл вектора a(teta)
AF_8 = 1 + np.exp(1j*psi) + np.exp(1j*2*psi) + np.exp(1j*3*psi) + np.exp(1j*4*psi)  + np.exp(1j*5*psi)  + np.exp(1j*6*psi)  + np.exp(1j*7*psi)

print("\nКоэффициент AF при помощи выражения для 4 элементов, 60 градусов:\n", AF_4)
print("Коэффициент AF при помощи выражения для 8 элементов, 60 градусов:\n", AF_8)

psi = 2 * (np.pi * d / lam) * np.sin(phi)
teta0 = 60 * np.pi / 180
delta = -(2 * np.pi * d / lam) * np.sin(teta0)


AF_4 = 1 +  np.exp(1j*psi)  +  np.exp(1j*2*psi)  + np.exp(1j*3*psi)  #коэфф АР = сумме элементов управл вектора a(teta)
AF_8 = 1 +  np.exp(1j*psi)  +  np.exp(1j*2*psi)  + np.exp(1j*3*psi) + np.exp(1j*4*psi)  + np.exp(1j*5*psi)  + np.exp(1j*6*psi)  + np.exp(1j*7*psi)
AFs_4 = 1 + np.exp(1j * (psi + delta)) + np.exp(1j * 2 * (psi + delta)) + np.exp(1j * 3 * (psi + delta))
AFs_8 = 1 + np.exp(1j * (psi + delta)) + np.exp(1j * 2 * (psi + delta)) + np.exp(1j * 3 * (psi + delta)) + np.exp(1j * 4 * (psi + delta)) + np.exp(1j * 5 * (psi + delta)) + np.exp(1j * 6 * (psi + delta)) + np.exp(1j * 7 * (psi + delta))

AF_4 = np.abs(AF_4)**2
AF_8 = np.abs(AF_8)**2
AFs_4 = np.abs(AFs_4)**2
AFs_8 = np.abs(AFs_8)**2

plt.figure(figsize=(10, 5))

plt.title("Диаграмма для 4 элементов с поворотом")
plt.subplot(1, 2, 1, polar=True)
plt.polar(phi, AF_4)

plt.title("Диаграмма для 8 элементов с поворотом")
plt.subplot(1, 2, 2, polar=True)
plt.polar(phi, AF_8)

plt.tight_layout()

plt.figure(figsize=(10, 5))

plt.subplot(1, 2, 1, polar=True)
plt.polar(phi, AFs_4)
plt.title("Диаграмма для 4 элементов с поворотом")

plt.subplot(1, 2, 2, polar=True)
plt.polar(phi, AFs_8)
plt.title("Диаграмма для 8 элементов с поворотом")

plt.tight_layout()
plt.show()
