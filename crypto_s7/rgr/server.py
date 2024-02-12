import random
import asyncio
from shared import *

# Задаём открытые параметры
t = 40

p = gen_prime(10 ** 8, 10 ** 9)
q = gen_prime(10 ** 8, 10 ** 9)
n = p * q

# Сервер проверяет наличие у клиента секрета
async def handle_connection(rcv, snd):
    # Делимся открытыми параметрами с клиентом
    await send_num(snd, n)
    await send_num(snd, t)
    print(f"Отправляем клиенту модуль n: {n}.")
    print(f"Отпарвляем клиенту число аккредитаций t: {t}.")

    # Получаем открытый ключ
    v = await read_num(rcv)
    print(f"Клиент прислал открытый ключ v: {v}.")

    # Проводим несколько раундов проверок
    is_ok = True
    for i in range(1, t+1):
        x = await read_num(rcv)
        e = random.randint(0, 1)
        await send_num(snd, e)
        y = await read_num(rcv)
        # Проводим проверку
        is_ok = mod_exp(y, 2, n) == (x * (v**e)) % n
        await send_num(snd, 1 if is_ok else 0)
        # Если клиент не смог доказать знание секрета, предварительно завершаем проверки.
        if not is_ok:
            print(f"Аккредитация №[{i}/{t}] провалилась.")
            break
        if i % 10 == 0:
            print(f"Аккредитация №[{i}/{t}] прошла успешно.")

    # Уведомляем клиент о том, получилось ли у него доказать знание секрета.
    if is_ok:
        message = f"Успех! Уверенность в решении: {1 - 1 / (2 ** t)}\n"
    else:
        message = "Провал.\n"

    print(message)
    snd.write(message.encode())
    await snd.drain()
    snd.close()
    await snd.wait_closed()


async def main():
    server = await asyncio.start_server(handle_connection, "127.0.0.1", 8888)
    async with server:
        await server.serve_forever()

asyncio.run(main())
