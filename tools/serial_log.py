import datetime
import sys
import time

import serial

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM4"
OUT = sys.argv[2] if len(sys.argv) > 2 else "local/longrun.log"

with open(OUT, "a", encoding="utf-8", buffering=1) as f:
    while True:
        try:
            with serial.Serial(PORT, 115200, timeout=1) as s:
                f.write(f"# {datetime.datetime.now().isoformat()} opened {PORT}\n")
                while True:
                    line = s.readline().decode("utf-8", errors="replace").rstrip()
                    if line:
                        f.write(f"{datetime.datetime.now().strftime('%H:%M:%S')} {line}\n")
        except serial.SerialException as e:
            f.write(f"# {datetime.datetime.now().isoformat()} serial error: {e}\n")
            time.sleep(2)