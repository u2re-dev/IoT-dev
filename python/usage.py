#!/usr/bin/env python3
"""
Пример перевода класса TuyaDevice34 из C++ в Python для CircuitPython.
В этом примере используются функции, реализованные в модуле libtuya (например,
encrypt_data_ecb, decrypt_data_ecb, encode_tuya_code, prepare_tuya_code, checksum_tuya_code,
encode_remote_hmac, encode_hmac_key).
Также используются некоторые служебные функции для работы с сетью/сокетами.
"""

import time
import socket
import json
from libtuya import (encrypt_data_ecb, decrypt_data_ecb, encode_tuya_code, prepare_tuya_code,
                     checksum_tuya_code, encode_remote_hmac, encode_hmac_key)
# Если используются функции для протокола 3.5, их тоже можно импортировать:
# from libtuya import encrypt_data_gcm, decrypt_data_gcm

# --- Заглушки для функций отладки и работы с сетью ---
def debug_log(msg):
    print("[DEBUG]", msg)

def debug_code(buf):
    # Выводим буфер в виде шестнадцатеричной строки
    print("[DEBUG CODE]", buf.hex())

def wait_and_send(client, data):
    """Заглушка для функции, которая отправляет данные через клиентское соединение."""
    # Возможная реализация: client.send(data)
    client.send(data)

def wait_for_receive(client, timeout_ms=100):
    """Заглушка для функции, которая ждёт данные с клиента в течение timeout_ms миллисекунд.
       Возвращает полученные данные в виде bytearray.
    """
    # Реализуйте согласно возможностям вашего устройства/сокетной библиотеки.
    # Здесь просто возвращаем пустой bytearray.
    return bytearray()

def connect_to_device(client, ip_bytes):
    """Простая функция подключения к устройству по IP.
       ip_bytes – bytes длиной 4, полученные, например, через socket.inet_aton()
    """
    # Реализуйте подключение, например, вызовом client.connect((ip, port))
    ip_str = socket.inet_ntoa(ip_bytes)
    debug_log("Подключаемся к устройству с IP: " + ip_str)
    client.connect(ip_str)  # пример вызова; port можно задать внутри клиента

# Если необходимо – заглушки для выделения payload и получения команд из входящего пакета.
def get_tuya_payload(in_buffer):
    """
    Извлекает полезную нагрузку из принятого пакета.
    Например, если заголовок имеет фиксированную длину (16 байт для протокола 3.4),
    можно вернуть in_buffer[16:].
    """
    HEADER_SIZE = 16
    return in_buffer[HEADER_SIZE:]

def get_tuya_cmd(in_buffer):
    """
    Извлекает код команды из принятого пакета.
    Например, если код команды записан в байтах 8-12 заголовка (big‑endian),
    возвращаем целое число.
    """
    # Здесь предполагается, что in_buffer[8:12] содержит CMD_ID
    if len(in_buffer) < 12:
        return None
    return int.from_bytes(in_buffer[8:12], "big")


# --- Класс устройства Tuya ---

class TuyaDevice34:
    def __init__(self, client):
        """
        Инициализация устройства.
        client – объект вашего соединения (например, TCP-сокет или другой клиент).
        """
        self.client = client
        self.tuya_local_ip = None
        self.tuya_local_key = None  # bytes, длиной 16 байт
        self.device_id = None
        self.device_uid = None
        self.SEQ_NO = 1
        self.linked = False

        # Для работы с HMAC используется выделенный буфер.
        # Аналогом в C++ было: hmac_payload, где hmac_key = hmac_payload + 12.
        # Здесь мы выделяем 12+16 = 28 байт, и будем использовать срез hmac_payload[12:]
        self.hmac_payload = bytearray(28)
        self.hmac_key = self.hmac_payload[12:]  # Обратите внимание: срез здесь создаёт ссылку на данные
        # Базовые буферы для отправки/приёма (размеры можно настроить)
        self.out_buffer = bytearray(1024)
        self.in_buffer = bytearray(1024)
        # Для примера зададим local_nonce – 16 байт, должно быть уникальным для каждого устройства
        self.local_nonce = b'\x11' * 16

    def connect_device(self, tuya_local_ip, tuya_local_key, device_id, device_uid):
        """
        Инициализация параметров подключения.
        tuya_local_ip – строка IP адреса (например, "192.168.1.100")
        tuya_local_key – строка, содержащая ключ (должна быть не менее 16 символов; используем первые 16 байт)
        device_id, device_uid – идентификаторы устройства.
        """
        self.tuya_local_ip = tuya_local_ip
        # Берём первые 16 байт ключа (при необходимости можно добавить проверку)
        self.tuya_local_key = tuya_local_key.encode("utf-8")[:16]
        self.device_id = device_id
        self.device_uid = device_uid

        # Подключаемся к устройству (разбор IP-строки)
        try:
            ip_bytes = socket.inet_aton(tuya_local_ip)
        except OSError as ex:
            debug_log("Неверный IP адрес: " + str(ex))
            return

        connect_to_device(self.client, ip_bytes)

        # Копирование tuya_local_key в область для hmac_key (16 байт)
        # В Python изменять self.hmac_key как срез mutable bytearray можно, если работать напрямую с hmac_payload:
        self.hmac_payload[12:12+16] = self.tuya_local_key
        self.SEQ_NO = 1
        self.linked = False
        debug_log("Устройство подключено.")

    def send_message(self, cmd, data):
        """
        Отправка сообщения.
        cmd – код команды (целое число).
        data – bytes с данными, которые будут включены в Tuya-код.
        """
        # Создаём описание команды в виде словаря (можно заменить на именованный кортеж или класс)
        cmd_desc = {
            "SEQ_NO": self.SEQ_NO,
            "CMD_ID": cmd,
            "HMAC": bytes(self.hmac_key)  # если HMAC используется; здесь передаём непустой объект,
                                           # что будет трактоваться как True в логике libtuya
        }
        self.SEQ_NO += 1
        # Функция encode_tuya_code возвращает готовый пакет (bytes)
        code = encode_tuya_code(data, cmd_desc, self.hmac_key)
        if code:
            debug_log("Sent Code")
            debug_code(code)
            wait_and_send(self.client, code)

    def send_local_nonce(self):
        """
        Отправляет зашифрованное значение local_nonce.
        Используется для установления соединения.
        """
        if not self.client.connected():
            return

        key_len = 16
        # Шифруем local_nonce (без padding – данные уже 16 байт)
        encrypted = encrypt_data_ecb(self.tuya_local_key, self.local_nonce, use_padding=False)
        # Код команды 0x03 (0x3)
        self.send_message(0x03, encrypted)
        debug_log("Отправлен local_nonce")

    def set_dps(self, dps):
        """
        Передаёт набор параметров DPS (как dict).
        Формирует JSON документ и отправляет его.
        """
        sending = {}
        # Например, protocol – 5, время в мс
        sending["protocol"] = 5
        sending["t"] = int(time.time() * 1000)
        sending["data"] = {"dps": dps}
        # Для протокола 3.4 команда 0x0d используется для отправки JSON
        self.send_json(0x0d, sending)

    def send_json(self, cmd, doc):
        """
        Отправка JSON-сообщения (для протокола 3.4).
        cmd – код команды (целое число).
        doc – словарь, который будет сериализован в JSON.
        """
        json_str = json.dumps(doc)
        json_bytes = json_str.encode("utf-8")
        json_len = len(json_bytes)

        # Параметры расчёта длины (для протокола 3.4)
        HEADER_OFFSET = 16
        # В исходном коде: withHeadLen = jsonLen + 15
        with_head_len = json_len + 15
        # Округляем с учётом выравнивания до кратности 16
        encrypt_len = ((with_head_len + 16) // 16) * 16
        prepare_len = encrypt_len

        # Формирование заголовка и резервирование места в буфере out_buffer.
        # Функция prepare_tuya_code возвращает bytearray нужной длины с записанным заголовком.
        # Создаем команду – аналог cmdDesc (SEQ_NO, CMD_ID, HMAC)
        cmd_desc = {
            "SEQ_NO": self.SEQ_NO,
            "CMD_ID": cmd,
            "HMAC": bytes(self.hmac_key)
        }
        self.SEQ_NO += 1

        # Получаем полностью подготовленный буфер (out_buffer) для отправки.
        out_buf = prepare_tuya_code(prepare_len, cmd_desc)
        # Здесь out_buf имеет длину: 16 байт заголовка + prepare_len байт payload

        # Для протокола 3.4:
        # • payload начинается с out_buf[16:]
        # • в первые 15 байт payload должно быть занулено, затем после этого – сериализованный JSON
        payload = out_buf[HEADER_OFFSET:]
        # Обнуляем первые 15 байт payload
        for i in range(15):
            payload[i] = 0
        # Копируем протокольный идентификатор "3.4" в payload[0:3]
        payload[0:3] = b"3.4"
        # Помещаем JSON (не более 512 байт) начиная с payload[15]:
        copy_len = min(512, json_len)
        payload[15:15+copy_len] = json_bytes[:copy_len]

        # Шифруем заготовку.
        # Для протокола 3.4 вызываем encrypt_data_ecb и затем вычисляем контрольную сумму.
        # Примечание: функция encrypt_data_ecb принимает длину данных для шифрования (encrypt_len)
        # и возвращает зашифрованный блок; здесь зашифровываем блок payload (начиная с HEADER_OFFSET).
        encrypted_block = encrypt_data_ecb(self.hmac_key, payload, use_padding=False)
        # Если в вашей библиотеке encrypt_data_ecb меняет содержимое блока, можно сразу работать с payload.
        # Затем рассчитываем контрольную сумму в заголовке:
        out_buf = checksum_tuya_code(out_buf, self.hmac_key)

        debug_log("Sent JSON Code")
        debug_code(out_buf)
        wait_and_send(self.client, out_buf)

    def handle_signal(self):
        """
        Обработка входящих сообщений от устройства.
        Функция должна вызываться периодически (например, в основном цикле).
        """
        if not self.client.connected():
            return

        # Ждем получения данных (здесь timeout 100 мс)
        received = wait_for_receive(self.client, 100)
        if not received or len(received) == 0:
            return

        debug_log("Received Code")
        debug_code(received)

        # Извлекаем полезную нагрузку и командный код.
        payload = get_tuya_payload(received)
        cmd = get_tuya_cmd(received)
        # Например, если cmd == 0x4 – устанавливается связь HMAC
        if cmd == 0x4:
            # Выделяем буфер на 48 байт для hmac
            hmac_buf = bytearray(48)
            # Вызываем функцию, которая сначала расшифровывает remote_nonce (полезная нагрузка),
            # затем вычисляет HMAC и шифрует его.
            # Функция encode_remote_hmac принимает: (hmac_key, remote_nonce)
            remote_hmac = encode_remote_hmac(self.hmac_key, payload)
            hmac_buf[:] = remote_hmac[:48]
            # Отправляем вычисленный hmac командой 0x05
            self.send_message(0x05, bytes(hmac_buf))
            # Также производится обмен ключами: вычисляем новый hmac_key с помощью encode_hmac_key.
            # Заметим, что в C++ передавался tuya_local_key как строка. Здесь используем self.tuya_local_key.
            new_key = encode_hmac_key(self.tuya_local_key, payload, self.hmac_payload)
            # Обновляем hmac_key (учтите, что self.hmac_key – это срез self.hmac_payload,
            # поэтому можно перезаписать соответствующие байты)
            self.hmac_payload[12:12+16] = new_key[:16]
            self.linked = True
            debug_log("Установлена связь (linked = True)")
        # Если cmd == 0x8 – получено JSON-сообщение
        elif cmd == 0x8:
            data_offset = 15
            # В данном случае payload содержит зашифрованный JSON.
            # Сначала дешифруем payload (все данные шифруются функцией decrypt_data_ecb):
            decrypted = decrypt_data_ecb(self.hmac_key, payload)
            # После дешифрования пропускаем первые 15 байт, где находится оформление
            json_part = decrypted[data_offset:]
            try:
                current = json.loads(json_part.decode("utf-8"))
                debug_log("Получен JSON:")
                print(current)
            except Exception as ex:
                debug_log("Ошибка при разборе JSON: " + str(ex))

# Пример использования:
if __name__ == "__main__":
    # Здесь предполагается, что у вас есть класс Client с методами connected(), connect(), send(), etc.
    # Для теста можно создать dummy-класс.
    class DummyClient:
        def __init__(self):
            self._connected = False
        def connect(self, ip):
            self._connected = True
            debug_log("DummyClient: подключено к " + ip)
        def connected(self):
            return self._connected
        def send(self, data):
            debug_log("DummyClient: отправляю данные длиной " + str(len(data)))
    client = DummyClient()
    device = TuyaDevice34(client)
    device.connect_device("192.168.1.100", "0123456789ABCDEF", "device_id_123", "device_uid_456")
    # Отправка local_nonce
    device.send_local_nonce()
    # Пример отправки набора DPS
    sample_dps = {"1": True, "2": 25}
    device.set_dps(sample_dps)
    # В цикле обработки сигналов:
    while True:
        device.handle_signal()
        time.sleep(0.1)
