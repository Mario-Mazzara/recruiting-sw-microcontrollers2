import serial
import time
import matplotlib.pyplot as plt
import numpy as np
import sounddevice as sd
import threading

# Audio parameters
fs = 10000            # Sampling rate
amplitude = 0.02       # volume
base_freq = 400      
scaling = 2         


current_freq = base_freq  
phase = 0  


freq_lock = threading.Lock()

def audio_callback(outdata, frames, time_info, status):
    global phase, current_freq
    t = (np.arange(frames) + phase) / fs
    # Use the current frequency; we acquire the lock for thread safety
    with freq_lock:
        freq = current_freq
    sine_wave = amplitude * np.sin(2 * np.pi * freq * t)
    outdata[:] = sine_wave.reshape(-1, 1)
    phase = (phase + frames) % fs

def start_audio_stream():
    """Start the continuous audio stream in non-blocking mode."""
    stream = sd.OutputStream(channels=1, callback=audio_callback, samplerate=fs)
    stream.start()
    return stream

def find_serial_port():
    """Scan for an available port from /dev/ttyACM0 to /dev/ttyACM20."""
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

def send_command(ser, command):
    """Send a command to the STM32."""
    ser.write((command + '\n').encode())

def wait_for_data(ser):
    """Wait until data arrives in the serial buffer."""
    print("Waiting for data...")
    while ser.in_waiting == 0:
        time.sleep(0.1)
    print("Data detected, starting real-time plot and audio modulation...")

def real_time_plot_and_audio(ser):
    """Continuously read serial data, update plots, and modulate audio from the 1st value."""
    global current_freq
    # Set up interactive plotting with two subplots
    plt.ion()
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 6))
    
    data1 = []  
    data2 = []  
    x = []      
    sample = 0

    try:
        while True:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='replace').strip()
                # Expecting two numbers separated by whitespace
                parts = line.split()
                if len(parts) >= 2:
                    try:
                        num1 = float(parts[0])
                        num2 = float(parts[1])
                    except ValueError:
                        # Skip this line if conversion fails.
                        continue
                    
                    data1.append(num1)
                    data2.append(num2)
                    x.append(sample)
                    sample += 1

                    # Update frequency based on analogue value:
                    with freq_lock:
                        current_freq = base_freq + num1 * scaling

                    # Update first subplot (analogue)
                    ax1.clear()
                    ax1.plot(x, data1, marker='o', color='b')
                    ax1.set_xlabel('Sample')
                    ax1.set_ylabel('Value 1')
                    ax1.set_title('Real-time Plot: Value 1')
                    
                    # Update second subplot (digital)
                    ax2.clear()
                    ax2.plot(x, data2, marker='o', color='r')
                    ax2.set_xlabel('Sample')
                    ax2.set_ylabel('Value 2')
                    ax2.set_title('Real-time Plot: Value 2')
                    
                    plt.tight_layout()
                    plt.pause(0.01)
                else:
                    # If not enough data, skip the line.
                    continue
    except KeyboardInterrupt:
        print("\nExiting real-time plot and audio modulation...")
    finally:
        ser.close()
        plt.ioff()
        plt.show()

if __name__ == "__main__":
    port = find_serial_port()
    if not port:
        print("No available /dev/ttyACMx port found.")
        exit(1)

    # Open the found port
    ser = serial.Serial(port, baudrate=115200, timeout=1)

    # Prompt user for a command to send to STM32
    command = input("Enter command to send to STM32: ")
    send_command(ser, command)

    # Wait until data is detected
    wait_for_data(ser)
    
    # Start the audio stream in the background
    audio_stream = start_audio_stream()
    
    # Start reading data, plotting, and modulating audio in real time
    real_time_plot_and_audio(ser)
    
    # When done, stop the audio stream
    audio_stream.stop()
