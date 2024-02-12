import random

# Возведение в степень по модулю
def mod_exp(a, x, p):
    res = 1
    a0 = a
    n = x
    while n:
        if n & 1:
            res = (res * a0) % p
        a0 = (a0 * a0) % p
        n >>= 1
    return res

# Алгоритм Евклида, для нахождения наибольшего общего делителя
def gcd(a, b):
    while b != 0:
        r = a % b
        a = b
        b = r
    return a

# Нахождение взаимно-простого числа
def gen_coprime(p):
    while True:
        result = random.randint(2, p)
        if gcd(p, result) == 1:
            return result

# Нахождение простого числа в заданых пределах
def gen_prime(min, max):
    while True:
        p = random.randint(min, max)
        if check_prime(p):
            return p

# Проверка, простое ли число по теореме Ферма
def check_prime(p):
    if p <= 1:
        return False
    if p == 2:
        return True
    for _ in range(128):
        a = random.randint(2, p - 1)
        if mod_exp(a, p - 1, p) != 1 or gcd(p, a) != 1:
            return False
    return True

# Отправка числа по соединению
async def send_num(snd, num):
    snd.write(f"{num}\n".encode())
    await snd.drain()

# Получение числа по соединению
async def read_num(rcv):
    return int((await rcv.readline()).strip())
