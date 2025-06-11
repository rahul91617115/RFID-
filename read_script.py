import serial
import time
from datetime import datetime

# Initialize serial connection (adjust COM port and baud rate)
ser = serial.Serial('COM10', 9600, timeout=1)
time.sleep(2)  # Allow connection to stabilize

# File to log attendance
LOG_FILE = "class_attendance.txt"

# Track already logged IDs for the current day
logged_ids = set()

def get_current_date():
    """Returns current date in YYYY-MM-DD format."""
    return datetime.now().strftime("%Y-%m-%d")

try:
    while True:
        # Read ID from serial (e.g., RFID card)
        student_id = ser.readline().decode("utf-8").strip()
        
        if student_id:
            current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            today = get_current_date()
            
            # Skip if ID already logged today
            if (student_id, today) not in logged_ids:
                logged_ids.add((student_id, today))  # Mark as logged
                
                # Write to log file using UTF-8 encoding
                with open(LOG_FILE, "a", encoding="utf-8") as f:
                    f.write(f"[{current_time}] - ID: {student_id}\n")
                
                print(f"✅ Logged: ID {student_id} at {current_time}")

except KeyboardInterrupt:
    print("\n🛑 Attendance logging stopped.")
    ser.close()
