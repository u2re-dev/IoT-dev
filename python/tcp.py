#!/usr/bin/env python3
"""
network.py – функции подключения к Wi‑Fi и установления связи с удалённым устройством,
а также функция разбора IP-адреса (ipv4_parse), портированная с C++ для CircuitPython.
"""

import time
import socket
from python.debug import debug_line_with_interval, debug_log
import wifi  # модуль, предоставляемый CircuitPython для работы с Wi‑Fi (на некоторых устройствах)
import ipaddress

#────────────────────────────────────────────
# Функция для подключения к Wi‑Fi (аналог connectWifi)
def connect_wifi(ssid, password):
    """
    Подключается к заданной сети Wi‑Fi.
    Возвращает True при успешном подключении, иначе – False.
    """
    debug_log("Переключаем Wi‑Fi в режим STA")
    # На некоторых платформах может потребоваться: wifi.radio.enabled = True
    start = time.monotonic()
    try:
        wifi.radio.connect(ssid, password)
    except Exception as err:
        debug_log("Ошибка подключения: " + str(err))
        return False

    # Ожидаем подключения до 10 секунд
    while not wifi.radio.ipv4_address:
        debug_line_with_interval(".", 0.1)
        if (time.monotonic() - start) > 10:
            break

    if wifi.radio.ipv4_address:
        debug_log("\nConnection Success")
        # (При необходимости, можно вызывать дополнительные функции инициализации, например, esp_netif_init)
        return True
    else:
        debug_log("\nConnection Failed")
        return False

#────────────────────────────────────────────
# Функция для подключения к устройству по заданному IP и порту (аналог connectToDevice)
def connect_to_device(client, local_ip, port):
    """
    Пытается установить соединение с устройством по указанному IP и порту.
    client – объект сокета
    local_ip – строка вида "192.168.1.100" или объект ipaddress.IPv4Address
    port – целое число
    Возвращает True, если соединение установлено, иначе – False.
    """
    start = time.monotonic()
    success = False
    ip_str = str(local_ip) if not isinstance(local_ip, str) else local_ip
    while (time.monotonic() - start) < 10:
        try:
            client.connect((ip_str, port))
            success = True
            break
        except Exception as err:
            debug_line_with_interval(".", 0.1)
            # Если клиент уже подключён, можно завершить цикл:
            try:
                client.getpeername()
                success = True
                break
            except Exception:
                pass
    if success:
        debug_log("\nDevice Connection Success")
    else:
        debug_log("\nDevice Connection Failed")
    return success

#────────────────────────────────────────────
# Функция отправки данных (аналог waitAndSend)
def wait_and_send(client, data):
    """
    Отправляет данные через клиентский сокет.
    Если Wi‑Fi не подключён, выводит сообщение об ошибке.
    """
    # Проверяем наличие подключения (в простейшем случае можно смотреть wifi.radio.ipv4_address)
    if not wifi.radio.ipv4_address:
        debug_log("WiFi is not connected")
        return

    start = time.monotonic()
    try:
        # Если сокет заблокирован для записи, можно добавить ожидание; здесь – пробуем сразу отправить
        client.send(data)
    except Exception as err:
        debug_log("Ошибка отправки: " + str(err))
        
#────────────────────────────────────────────
# Функция ожидания приёма данных (аналог waitForReceive)
def wait_for_receive(client, timeout_ms):
    """
    Ожидает поступления данных от клиента до timeout_ms миллисекунд.
    Возвращает кортеж (done, data),
    где done = True, если данные получены, иначе False.
    """
    done = False
    start = time.monotonic()
    received = bytearray()
    # Переводим timeout в секунды
    timeout_s = timeout_ms / 1000
    # Устанавливаем небольшой timeout на recv
    client.settimeout(0.1)
    while (time.monotonic() - start) < timeout_s:
        try:
            chunk = client.recv(1024)
            if chunk:
                received.extend(chunk)
                done = True
                break
        except Exception:
            # Если данных нет – ловим исключение таймаута
            pass
    if not done:
        # Дополнительные проверки (в оригинале выводятся сообщения об отключении)
        try:
            client.getpeername()
        except Exception:
            debug_log("Device Disconnected")
        if not wifi.radio.ipv4_address:
            debug_log("Network Disconnected")
    return done, received
