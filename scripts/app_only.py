import os

Import("env")

firmware_path = os.path.join(env.subst("$BUILD_DIR"), "firmware.bin")

def custom_upload(target, source, env):
    command = (
        f'esptool.py --chip esp32s3 --port {env.subst("$UPLOAD_PORT")} --baud 115200 write_flash 0x190000 {firmware_path}'
    )
    os.system(command)

env.Replace(UPLOADCMD=custom_upload)
