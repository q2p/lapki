import random
import asyncio
from shared import *

# Клиент подтверждает владение секретом
async def run_client():
    rcv, snd = await asyncio.open_connection("127.0.0.1", 8888)

    # Открытые параметры получаются от сервера
    n = await read_num(rcv)
    t = await read_num(rcv)
    print(f"Сервер прислал модуль n: {n}.")
    print(f"Сервер прислал число аккредитаций t: {t}.")

    s = gen_coprime(n)

    # Открытый ключ отправлятеся серверу
    v = mod_exp(s, 2, n)
    print(f"Отправляем серверу открытый ключ v: {v}.")
    await send_num(snd, v)

    # Учавствуем в нескольких раундах проверки
    for i in range(1, t+1):
        r = random.randint(1, n-1)
        x = mod_exp(r, 2, n)
        await send_num(snd, x)

        e = await read_num(rcv)
        y = (r * (s**e)) % n
        await send_num(snd, y)

        # Если сервер отвергает доказательство, предварительно завершаем проверки
        if await read_num(rcv) == 0:
            print(f"Сервер не принял аккредитацию №[{i}/{t}].")
            break
        if i % 10 == 0:
            print(f"Сервер принял аккредитацию №[{i}/{t}].")

    # Прошла ли верефикация успешно
    print((await rcv.readline()).decode())
    snd.close()
    await snd.wait_closed()

asyncio.run(run_client())
