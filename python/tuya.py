# Начало файла libtuya.py

import struct
import hmac
import hashlib
import binascii
import aesio  # модуль, предоставляемый CircuitPython

# Вспомогательная функция для вычисления размера полезной нагрузки
def compute_payload_size(data_length, use_hmac):
    # Размер полезной нагрузки = исходные данные + (32 байта если HMAC, иначе 4 байта) + 4 байта для суффикса
    return data_length + (32 if use_hmac else 4) + 4

# Функции шифрования/дешифрования в ECB-режиме
def encrypt_data_ecb(key, data, use_padding=True):
    # key и data – объекты типа bytes
    # Если используется padding – добавляем PKCS#7-подобное дополнение до кратного 16
    if use_padding:
        pad_len = 16 - (len(data) % 16) if (len(data) % 16) != 0 else 16
        data_padded = data + bytes([pad_len] * pad_len)
    else:
        # Если padding не используется, длина data должна быть кратной 16
        if len(data) % 16:
            raise ValueError("Длина данных должна быть кратной 16, если padding не используется")
        data_padded = data

    cipher = aesio.AES(key)
    out = bytearray()
    # Обрабатываем данные по блокам по 16 байт
    for i in range(0, len(data_padded), 16):
        block = data_padded[i:i+16]
        encrypted = cipher.encrypt(block)
        out.extend(encrypted)
    return bytes(out)


def decrypt_data_ecb(key, data):
    # data – шифрованные данные (длина должна быть кратна 16)
    if len(data) % 16:
        raise ValueError("Длина зашифрованных данных должна быть кратной 16")
    cipher = aesio.AES(key)
    out = bytearray()
    for i in range(0, len(data), 16):
        block = data[i:i+16]
        decrypted = cipher.decrypt(block)
        out.extend(decrypted)
    # Попытка убрать padding (если он использовался)
    pad_len = out[-1]
    if 0 < pad_len <= 16 and out[-pad_len:] == bytes([pad_len]) * pad_len:
        out = out[:-pad_len]
    return bytes(out)


# Функция подготовки кода Tuya (записывает заголовок, резервирует место под данные и суффикс)
def prepare_tuya_code(data_length, cmd_desc):
    # header_len всегда 16 байт
    header_len = 16
    payload_size = compute_payload_size(data_length, cmd_desc.HMAC)
    total_length = header_len + payload_size
    out = bytearray(total_length)
    
    # Запись заголовка (в big‑endian)
    # 0x000055AA
    out[0:4] = (0x000055AA).to_bytes(4, byteorder='big')
    # SEQ_NO
    out[4:8] = int(cmd_desc.SEQ_NO).to_bytes(4, byteorder='big')
    # CMD_ID
    out[8:12] = int(cmd_desc.CMD_ID).to_bytes(4, byteorder='big')
    # payloadSize
    out[12:16] = int(payload_size).to_bytes(4, byteorder='big')
    
    # Зона полезной нагрузки:
    payload = out[header_len:]
    # Обнуляем первые (data_length + (32 если HMAC, иначе 4)) байт
    pad_zone = data_length + (32 if cmd_desc.HMAC else 4)
    for i in range(pad_zone):
        payload[i] = 0
    # Записываем суффикс 0x0000AA55 сразу после заполненной области
    suffix_offset = data_length + (32 if cmd_desc.HMAC else 4)
    payload[suffix_offset:suffix_offset+4] = (0x0000AA55).to_bytes(4, byteorder='big')
    
    return out


# Функция расчёта контрольной суммы/ HMAC и запись их в заготовку кода
def checksum_tuya_code(code, hmac_key=None):
    header_len = 16
    payload_size = int.from_bytes(code[12:16], byteorder='big')
    payload = code[header_len: header_len + payload_size]
    # Вычисляем длину зашифрованной нагрузки (без контрольного значения и суффикса)
    check_length = payload_size - ((32 if hmac_key is not None else 4) + 4)
    
    # Обнуляем область, куда будет записан HMAC/CRC
    checksum_area = (32 if hmac_key is not None else 4)
    for i in range(checksum_area):
        payload[check_length + i] = 0

    if hmac_key is not None:
        # Вычисляем HMAC SHA256 от всего сообщения начиная с заголовка и заканчивая полезной нагрузкой (до контрольной суммы)
        hm = hmac.new(hmac_key, code[0: header_len + check_length], hashlib.sha256).digest()
        # Записываем HMAC в область после полезной нагрузки
        payload[check_length: check_length + 32] = hm
    else:
        # Если HMAC не используется – вычисляем CRC32
        crc = binascii.crc32(code[0: header_len + check_length]) & 0xffffffff
        payload[check_length: check_length + 4] = crc.to_bytes(4, byteorder='big')
    return code


# Функция кодирования Tuya-кода (подготавливает заголовок, копирует зашифрованные данные и рассчитывает checksum)
def encode_tuya_code(encrypted_data, cmd_desc, hmac_key=None):
    # Здесь encrypted_data уже зашифрованы (например, с помощью encrypt_data_ecb)
    data_length = len(encrypted_data)
    code = prepare_tuya_code(data_length, cmd_desc)
    header_len = 16
    # Копируем зашифрованные данные в зону после заголовка
    code[header_len: header_len + data_length] = encrypted_data
    code = checksum_tuya_code(code, hmac_key if cmd_desc.HMAC else None)
    return code


# Функция вычисления “удалённого” hmac. 
# Алгоритм: сначала расшифровывается remote_nonce, затем рассчитывается HMAC-SHA256 от расшифрованного remote_nonce,
# после чего полученный HMAC шифруется в ECB‑режиме (без padding).
def encode_remote_hmac(original_key, remote_nonce):
    # Предполагается, что remote_nonce имеет длину 16 байт.
    # Сначала расшифровываем remote_nonce
    decrypted_nonce = decrypt_data_ecb(original_key, remote_nonce)
    # Рассчитываем HMAC-SHA256 от расшифрованного nonce (ключ – original_key)
    hm = hmac.new(original_key, decrypted_nonce, hashlib.sha256).digest()  # 32 байта
    # Выделяем буфер длиной 48 байт: первые 32 байта – HMAC, оставшиеся 16 нулевых байт
    buffer = hm + bytes(16)
    # Шифруем полученный буфер в ECB‑режиме без padding
    remote_hmac = encrypt_data_ecb(original_key, buffer, use_padding=False)
    return remote_hmac


# Функция кодирования ключа HMAC. 
# Схема: локально заданный local_nonce и remote_nonce смешиваются по XOR (по 16 байт),
# затем полученный 16-байтовый блок шифруется в ECB‑режиме без padding.
def encode_hmac_key(original_key, remote_nonce, local_nonce):
    if len(remote_nonce) != 16 or len(local_nonce) != 16:
        raise ValueError("remote_nonce и local_nonce должны быть по 16 байт")
    # XOR локального и удалённого значений (16 байт)
    xor_bytes = bytes(a ^ b for a, b in zip(local_nonce, remote_nonce))
    # Шифруем результат (16 байт)
    encrypted = encrypt_data_ecb(original_key, xor_bytes, use_padding=False)
    # Возвращаем первый блок (16 байт)
    return encrypted[:16]


# ===== Опционально: Функции работы в режиме AES-GCM (например, для Tuya протокола 3.5)
# В CircuitPython нет встроенной поддержки GCM, поэтому эти функции либо следует реализовать через стороннюю библиотеку,
# либо пометить как не реализованные.

def decrypt_data_gcm(key, data):
    # data: первые 12 байт – IV, далее зашифрованный блок, затем 16-байтовый тег
    # Здесь используем библиотеку cryptography, если она доступна.
    raise NotImplementedError("AES-GCM не реализован в данном примере для CircuitPython")


def encrypt_data_gcm(key, data):
    raise NotImplementedError("AES-GCM не реализован в данном примере для CircuitPython")


# ===== Пример объекта cmdDesc
class TuyaCmd:
    def __init__(self, seq_no, cmd_id, use_hmac=False):
        self.SEQ_NO = seq_no
        self.CMD_ID = cmd_id
        self.HMAC = use_hmac  # True, если надо использовать HMAC вместо CRC32


# ===== Пример использования функций
if __name__ == '__main__':
    # Пример ключа и данных (16 байт)
    key = b'\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10'
    plain = b'Пример данных для шифрования'
    
    # Шифруем данные (с padding)
    encrypted = encrypt_data_ecb(key, plain, use_padding=True)
    print("Зашифрованные данные (ECB):", encrypted)
    
    # Дешифруем обратно
    decrypted = decrypt_data_ecb(key, encrypted)
    print("Дешифрованные данные:", decrypted)
    
    # Пример формирования Tuya-кода
    cmd = TuyaCmd(seq_no=1, cmd_id=2, use_hmac=False)
    # Пусть encrypted_data – это зашифрованный блок, который требуется обернуть заголовком и контрольной суммой
    tuya_code = encode_tuya_code(encrypted, cmd)
    print("Tuya-код:", tuya_code)
    
    # Пример вычисления удалённого HMAC (при наличии remote_nonce длиной 16 байт)
    remote_nonce = encrypt_data_ecb(key, b'\x00'*16, use_padding=False)   # для примера – зашифрованный ноль
    remote_hmac = encode_remote_hmac(key, remote_nonce)
    print("Удалённый HMAC:", remote_hmac)
    
    # Пример кодирования ключа HMAC (при наличии local_nonce и remote_nonce по 16 байт)
    local_nonce = b'\x11'*16
    hmac_key = encode_hmac_key(key, remote_nonce[:16], local_nonce)
    print("Закодированный HMAC-ключ:", hmac_key)
