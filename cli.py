import serial

# Configure the serial connection
def find_serial_port():
    for i in range(21):
        port = f"/dev/ttyACM{i}"
        try:
            ser = serial.Serial(port, baudrate=115200, timeout=1)
            ser.close()
            print(f"Found available port: {port}")
            return port
        except serial.SerialException:
            continue
    return None

def send_command(command):
    """Send a command to the STM32 and print the response."""
    ser.write((command + '\n').encode())  # Send command with newline
if __name__ == "__main__":
    port = find_serial_port()
    if not port:
        print("No available /dev/ttyACMx port found.")
        exit(1)

    # Open the found port
    ser = serial.Serial(port, baudrate=115200, timeout=1)

    try:
        while True:
            cmd = input("Enter command: ")  # User input
            if cmd.lower() in ["exit", "quit"]:
                break
            send_command(cmd)
    except KeyboardInterrupt:
        print("\nExiting...")

    ser.close()
