import serial


# Open serial port
ser = serial . Serial('/dev/ttyACM0', 115200, timeout=1)


# Transfer ASCII characters
data=f"PINSTAT 0"
ser . write(f"{data}\r".encode())


# Receive response
response = ser . read_until() . strip()
print("Res: ", response . decode())


# Finalize
ser . close()
