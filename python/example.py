#────────────────────────────────────────────
# Пример использования функций network
from python.debug import debug_code, debug_line, debug_line_with_interval, debug_log
from python.device import TuyaDevice34
from python.ipv4 import IPV4_SUCCESS, ipv4_parse
from python.tcp import connect_to_device, connect_wifi, wait_and_send, wait_for_receive
from python.tuya import decrypt_data_ecb, encode_hmac_key, encode_remote_hmac, encode_tuya_code, encrypt_data_ecb

#
if __name__ == '__main__':
    # Пример подключения к Wi‑Fi
    WIFI_SSID = "MySSID"
    WIFI_PASS = "MyPassword"
    if connect_wifi(WIFI_SSID, WIFI_PASS):
        debug_log("Wi‑Fi подключён: " + str(wifi.radio.ipv4_address))
    else:
        debug_log("Не удалось подключиться к Wi‑Fi")

    # Создаём TCP‑клиент (сокет)
    # В CircuitPython используется стандартный модуль socket и, возможно, socketpool (на некоторых устройствах)
    client = socket.socket()
    client.setblocking(True)

    # Пример разбора IPv4-адреса
    err, ip_list = ipv4_parse("192.168.1.100")
    if err == IPV4_SUCCESS:
        # Можно также использовать ipaddress.IPv4Address, если требуется
        ip_addr = ipaddress.IPv4Address(".".join(str(n) for n in ip_list))
        debug_log("Parsed IP: " + str(ip_addr))
    else:
        debug_log("Ошибка разбора IP: код " + str(err))

    # Подключаемся к устройству (например, на порту 6668)
    if connect_to_device(client, ip_addr, 6668):
        debug_log("Устройство подключено")
    else:
        debug_log("Не удалось подключиться к устройству")

    # Пример отправки данных:
    data_to_send = b"Hello, device!"
    wait_and_send(client, data_to_send)

    # Пример приёма данных с таймаутом 100 мс:
    done, received = wait_for_receive(client, 100)
    if done:
        debug_log("Получены данные длиной " + str(len(received)))
        debug_log(received.hex())
    else:
        debug_log("Данные не получены")
        
    # Закрываем соединение
    client.close()

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
    plain = b'{}'
    
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

# Пример использования:
if __name__ == "__main__":
    debug_log("Hello, DebugLog!")
    debug_line("Hello, DebugLine! ")
    for i in range(5):
        debug_line_with_interval(".", 100)  # вывод точки каждые 100 мс
        time.sleep(0.05)  # смоделировать короткую задержку
    print()  # перевод строки после цикла
    sample_code = bytes([0x01, 0x2A, 0x0F, 0xB3])
    debug_code(sample_code)

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
