import serial
import threading
import time
from collections import deque

import matplotlib
try:
    matplotlib.use("TkAgg")
except Exception:
    pass
import matplotlib.pyplot as plt
import numpy as np

SERIAL_PORT = "COM4"
BAUD_RATE = 115200
BUFFER_SIZE = 200
UPDATE_INTERVAL = 0.01  # plot refresh interval in seconds
ANGLE_MIN = -180.0
ANGLE_MAX = 180.0

# don't open serial at import time; open in reader with retries
ser = None
DEBUG = True

# counters for minimal debug output
_parsed_count = 0
_raw_log_count = 0
RAW_LOG_LIMIT = 20
FALLBACK_NINE_CSV = True
# buffers for the two plotted traces
sensors = {
    "acc": {"x": deque([0.0] * BUFFER_SIZE, maxlen=BUFFER_SIZE),
            "y": deque([0.0] * BUFFER_SIZE, maxlen=BUFFER_SIZE)},
}

lock = threading.Lock()
new_data_event = threading.Event()
running = threading.Event()
running.set()

x_axis = np.arange(BUFFER_SIZE)

plt.ion()
fig, ax = plt.subplots(1, 1, figsize=(9, 4))

lines = {}
sensor_names = ["acc"]
title = "Accelerometer"

lines["acc"] = []
for comp in ("x", "y"):
    ln, = ax.plot(x_axis, sensors["acc"][comp], label=f"{comp.upper()}")
    lines["acc"].append(ln)
ax.set_xlim(0, BUFFER_SIZE - 1)
ax.set_ylim(ANGLE_MIN, ANGLE_MAX)
ax.set_ylabel(f"{title} (deg)")
ax.set_xlabel("Samples")
ax.grid(True, alpha=0.3)
ax.legend(loc="upper left")

plt.show(block=False)
fig.canvas.draw()
plt.pause(0.01)


def serial_reader():
    """Read serial data in either of these formats:
    - new format: val1,val2\n
    - legacy format: x_acc,y,z x_gy,y,z x_mag,y,z\n\r
    """
    # ensure serial is open (retry if necessary)
    global ser
    while running.is_set():
        try:
            if ser is None or not getattr(ser, "is_open", False):
                try:
                    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
                    print(f"Opened serial {SERIAL_PORT} @ {BAUD_RATE}")
                except Exception as e:
                    print(f"Failed to open serial {SERIAL_PORT}: {e}")
                    time.sleep(1.0)
                    continue

            line = ser.readline().decode(errors="ignore").strip()
            if not line:
                continue

            parsed = None
            if "," in line:
                parts = [p.strip() for p in line.split(",") if p.strip() != ""]
                if len(parts) == 2:
                    try:
                        parsed = tuple(map(float, parts))
                    except Exception:
                        parsed = None

            if parsed is None:
                # fallback to the legacy whitespace-separated 3x3 payload format
                groups = line.split()
                if len(groups) != 3:
                    if FALLBACK_NINE_CSV:
                        parts = [p for p in line.replace(';', ',').split(',') if p != '']
                        if len(parts) == 9:
                            groups = [','.join(parts[0:3]), ','.join(parts[3:6]), ','.join(parts[6:9])]
                        else:
                            if DEBUG and _raw_log_count < RAW_LOG_LIMIT:
                                print(f"RAW_LINE[{_raw_log_count}]: {repr(line)}")
                                _raw_log_count += 1
                            continue

                parsed_groups = []
                valid = True
                for g in groups:
                    parts = g.split(",")
                    if len(parts) != 3:
                        valid = False
                        break
                    try:
                        parsed_groups.append(tuple(map(float, parts)))
                    except Exception:
                        valid = False
                        break

                if not valid:
                    continue

                parsed = parsed_groups

            with lock:
                if isinstance(parsed, tuple):
                    val1, val2 = parsed
                    sensors["acc"]["x"].append(val1)
                    sensors["acc"]["y"].append(val2)

                    new_data_event.set()
                    if DEBUG:
                        _parsed_count += 1
                        if _parsed_count % 50 == 0:
                            print(f"parsed {_parsed_count} samples; pair=({val1:.3f},{val2:.3f})")
                else:
                    # order: acc, gyro, mag
                    (ax, ay, az), (gx, gy, gz), (mx, my, mz) = parsed
                    sensors["acc"]["x"].append(ax)
                    sensors["acc"]["y"].append(ay)
                    sensors["acc"]["z"].append(az)

                    sensors["gyro"]["x"].append(gx)
                    sensors["gyro"]["y"].append(gy)
                    sensors["gyro"]["z"].append(gz)

                    sensors["mag"]["x"].append(mx)
                    sensors["mag"]["y"].append(my)
                    sensors["mag"]["z"].append(mz)

                    new_data_event.set()
                    if DEBUG:
                        _parsed_count += 1
                        if _parsed_count % 50 == 0:
                            print(f"parsed {_parsed_count} samples; last acc=({ax:.3f},{ay:.3f},{az:.3f}) gyro=({gx:.3f},{gy:.3f},{gz:.3f}) mag=({mx:.3f},{my:.3f},{mz:.3f})")

        except Exception:
            continue


def on_close(event):
    running.clear()


fig.canvas.mpl_connect("close_event", on_close)
reader_thread = threading.Thread(target=serial_reader, daemon=True)
reader_thread.start()

try:
    while running.is_set():
        if new_data_event.is_set():
            with lock:
                for sname in sensor_names:
                    for ln, comp in zip(lines[sname], ("x", "y")):
                        try:
                            values = np.asarray(sensors[sname][comp], dtype=float)
                            x_vals = np.arange(len(values))
                            ln.set_data(x_vals, values)
                        except Exception as e:
                            if DEBUG:
                                print(f"set_ydata error for {sname}.{comp}: {e}")
                new_data_event.clear()

            ax.relim()
            ax.autoscale_view()
            fig.canvas.draw_idle()
            fig.canvas.flush_events()

        # use plt.pause so GUI events are processed
        plt.pause(UPDATE_INTERVAL)

finally:
    running.clear()
    reader_thread.join(timeout=0.2)
    ser.close()
