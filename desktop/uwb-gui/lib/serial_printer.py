from lib.widget import Widget
import tkinter as tk
import serial
import threading
from serial.tools import list_ports

class SerialWidget(Widget):
    def __init__(self, parent, the_name):
        super().__init__(parent, the_name)
        self.type="SerialWidget"
        self.border.configure(bg="grey")

        # first Frame with Widgets
        self.top_frame = tk.Frame(self.border)
        self.top_frame.pack(side="top", fill="x")

        self.refresh_button = tk.Button(self.top_frame, text="Refresh", command=self.refresh_devices)
        self.refresh_button.pack(side="left")

        self.port_label = tk.Label(self.top_frame, text="Port:")
        self.port_label.pack(side="left")
        
        self.available_ports = []
        self.selected_port = tk.StringVar()
        
        self.port_menu = tk.OptionMenu(self.top_frame, self.selected_port, "", *self.available_ports)
        self.port_menu.pack(side="left")
        
        self.baudrate_label = tk.Label(self.top_frame, text="Baudrate:")
        self.baudrate_label.pack(side="left")
        
        self.available_bauds = [300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200]
        self.selected_baud = tk.StringVar()

        self.baud_menu = tk.OptionMenu(self.top_frame, self.selected_baud, "115200", *self.available_bauds)
        self.baud_menu.pack(side="left")
        
        # second Frame with Widgets
        self.bottom_frame = tk.Frame(self.border)
        self.bottom_frame.pack(side="bottom", fill="x")

        self.connect_button = tk.Button(self.bottom_frame, text="Connect", command=self.connect_to_port)
        self.connect_button.pack(side="left")

        self.clear_button = tk.Button(self.bottom_frame, text="Clear", command=self.clear_console)
        self.clear_button.pack(side="left")
        
        # Textfield for Serial print
        self.console_text = tk.Text(self.border, height=10)
        self.console_text.pack(side="left", fill='both', expand=True, padx=5, pady=5)

    def refresh_devices(self):
        self.available_ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_menu['menu'].delete(0, 'end')
        for port in self.available_ports:
            self.port_menu['menu'].add_command(label=port, command=tk._setit(self.selected_port, port))

    def connect_to_port(self, event=None):
        try:
            self.serial_conn = serial.Serial(self.selected_port.get(), self.selected_baud.get())
            self.console_text.config(state=tk.NORMAL)
            self.console_text.delete(1.0, tk.END)
            self.console_text.insert(tk.END, f"Connected to {self.selected_port.get()} with {self.selected_baud.get()} baudrate.\n")
            self.console_text.config(state=tk.DISABLED)
            self.serial_conn.flushInput()
            self.serial_conn.flushOutput()
            self.serial_conn.timeout = 0.5
            thread = threading.Thread(target=self.read_port_task)
            thread.start()
        except serial.SerialException as e:
            self.console_text.config(state=tk.NORMAL)
            self.console_text.insert(tk.END, f"Could not connect to {self.selected_port.get()}: {e}\n")
            self.console_text.config(state=tk.DISABLED)

    def clear_console(self):
        self.console_text.config(state=tk.NORMAL)
        self.console_text.delete(1.0, tk.END)
        self.console_text.config(state=tk.DISABLED)
        
    def read_port_task(self):
        while True:
            try:
                data = self.serial_conn.readline().decode().replace('\r', ' ')
                if data:
                    self.console_text.config(state=tk.NORMAL)
                    self.console_text.insert(tk.END, data)
                    self.console_text.config(state=tk.DISABLED)
                    self.console_text.yview(tk.END)
                    if int(self.console_text.index('end-1c').split('.')[0]) > 50:
                        self.console_text.config(state=tk.NORMAL)
                        self.console_text.delete('1.0', '2.0')
                        self.console_text.config(state=tk.DISABLED)
            except Exception as e:
                print(f"Error reading from serial port: {e}")
