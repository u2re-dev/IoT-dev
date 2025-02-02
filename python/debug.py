#!/usr/bin/env python3
"""
Модуль debug.py – функции для отладки, аналогичные DebugLog, DebugLine, DebugLineWithInterval и DebugCode из C++.
Для вывода используется стандартный print, который можно перенаправить на последовательный порт.
"""

import time

# Глобальная переменная для измерения интервалов (в миллисекундах)
_last_time = 0

def debug_log(message):
    """
    Вывод строки с переводом строки, аналог DebugLog.
    Возвращает переданное сообщение (для совместимости с вызовами).
    """
    print("[DEBUG]", message)
    return message

def debug_line(message):
    """
    Вывод строки без перевода строки сразу же (аналог DebugLine).
    Здесь вывод осуществляется через print с параметром end="".
    Возвращает сообщение.
    """
    print(message, end="")
    return message

def debug_line_with_interval(message, interval_ms):
    """
    Вывод сообщения, если с момента предыдущего вывода прошло не менее interval_ms миллисекунд.
    Интервал задаётся в миллисекундах.
    Возвращает сообщение.
    """
    global _last_time
    current_ms = time.monotonic() * 1000  # преобразуем в миллисекунды
    # Если первый вызов или разница превышает интервал – выводим сообщение
    if _last_time == 0 or (current_ms - _last_time) >= interval_ms:
        print(message, end="")  # end="" – без переноса строки
        _last_time = current_ms
    return message

#def debug_line_with_interval(msg, interval_seconds):
    #print(msg, end="", flush=True)
    #time.sleep(interval_seconds)

def debug_code(code_bytes):
    """
    Выводит содержимое буфера (bytes) в виде hex-строки в формате:
      0xAA BB CC ...
    Аналог DebugCode из C++.
    Возвращает пустую строку.
    """
    hex_str = "0x" + "".join("{:02X}".format(b) for b in code_bytes)
    print("[DEBUG CODE]", hex_str)
    return ""
