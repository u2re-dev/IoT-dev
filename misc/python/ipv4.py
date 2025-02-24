#────────────────────────────────────────────
# Функция для разбора строки IPv4 (аналог ipv4_parse)
# Определим коды ошибок, как в оригинале (значения можно менять по необходимости)
IPV4_SUCCESS = 0
ERROR_IPV4_DATA_OVERFLOW = 1
ERROR_IPV4_NO_SYMBOL = 2
ERROR_IPV4_INPUT_OVERFLOW = 3
ERROR_IPV4_INVALID_SYMBOL = 4
ERROR_IPV4_NOT_ENOUGH_INPUT = 5

#
def ipv4_parse(ip_string):
    """
    Парсит строку с IPv4-адресом (например, "192.168.1.100").
    Возвращает кортеж (err_code, result)
      err_code == IPV4_SUCCESS при успешном разборе, иначе – код ошибки (целое число).
      result – список из 4 целых чисел или None при ошибке.
    """
    # Преобразуем строку в bytes (ASCII)
    if isinstance(ip_string, str):
        ip_bytes = ip_string.encode("utf-8")
    else:
        ip_bytes = ip_string
    string_length = len(ip_bytes)
    
    result = [0, 0, 0, 0]
    at_least_one_symbol = False
    data = 0
    string_index = 0
    result_index = 0

    while string_index < string_length:
        symbol = ip_bytes[string_index]
        if 48 <= symbol <= 57:  # digit?
            data = data * 10 + (symbol - 48)
            if data > 255:
                return ERROR_IPV4_DATA_OVERFLOW, None
            at_least_one_symbol = True
        elif symbol == ord('.'):
            if result_index < 3:
                if at_least_one_symbol:
                    result[result_index] = data
                    data = 0
                    result_index += 1
                    at_least_one_symbol = False
                else:
                    return ERROR_IPV4_NO_SYMBOL, None
            else:
                return ERROR_IPV4_INPUT_OVERFLOW, None
        else:
            return ERROR_IPV4_INVALID_SYMBOL, None
        string_index += 1

    if result_index == 3:
        if at_least_one_symbol:
            result[result_index] = data
            return IPV4_SUCCESS, result
        else:
            return ERROR_IPV4_NOT_ENOUGH_INPUT, None
    else:
        # Например, если строка "127.0" – недостаточно данных
        return ERROR_IPV4_NOT_ENOUGH_INPUT, None
