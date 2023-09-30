from lib.widget import Widget
import threading
import asyncio
import tkinter as tk
import bleak
import time

import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

class BlePlotPositionWidget(Widget):
    def __init__(self, parent, the_name):
        super().__init__(parent, the_name)
        self.type="BlePlotPositionWidget"
        self.border.configure(bg="#555555")

        self.refresh_thread = None
        self.entry_check_thread = None
        self.print_position_thread = None

        # first Frame with Widgets
        self.top_frame = tk.Frame(self.border)
        self.top_frame.pack(side="top", fill="x")
        
        self.refresh_button = tk.Button(self.top_frame, text="Refresh", command=self.refresh_devices)
        self.refresh_button.pack(side="left")

        self.device_label = tk.Label(self.top_frame, text="Device:")
        self.device_label.pack(side="left")

        self.device_var = tk.StringVar(value="")
        self.device_menu = tk.OptionMenu(self.top_frame, self.device_var, "")
        self.device_menu.pack(side="left")

        self.services_button = tk.Button(self.top_frame, text="Connect", command=self.connect)
        self.services_button.pack(side="left")

        # Matplotlib-Figur und Achse erstellen
        self.fig, self.ax = plt.subplots()  # Festlegen der Größe der Figur (6x4 Zoll)
        self.ax.set_xlabel('X-Achse')
        self.ax.set_ylabel('Y-Achse')
        self.ax.set_title('Koordinaten-Plot')

        # Achsenlimits festlegen (Beispiel: von -10 bis 10 auf beiden Achsen)
        self.ax.set_xlim(-10, 10)
        self.ax.set_ylim(-10, 10)

        # Gitter hinzufügen
        self.ax.grid(True)

        self.canvas = FigureCanvasTkAgg(self.fig, master=self)
        self.canvas_widget = self.canvas.get_tk_widget()
        self.canvas_widget.pack()

        self.coordinates = {'x': [], 'y': []}  # Liste für Koordinaten

        

        self.address = None
        self.client = bleak.BleakClient("")
        asyncio.set_event_loop(asyncio.new_event_loop())
        self.loop = asyncio.get_event_loop()

    def destroy(self) -> None:
        if self.refresh_thread is not None:
            self.refresh_thread.join()
        if self.print_position_thread is not None:
            self.print_position_thread.join()
        return super().destroy()
    
    

    async def get_poition_from_bluetooth(self):
        # Definieren Sie hier die UUID, von der Sie die Position lesen möchten
        position_uuid = "76847a0a-2748-4fda-bcd7-74425f0e4a23"

        # Verwenden Sie die BleakClient-Instanz, um die Konfiguration von der UUID zu lesen
        return await self.get_characteristic_value(self.address, position_uuid)

    def update_plot(self):
        self.ax.clear()
        self.ax.scatter(self.coordinates['x'], self.coordinates['y'], marker='x')  # Punkte ohne Verbindung
        self.ax.set_xlabel('X-Achse')
        self.ax.set_ylabel('Y-Achse')
        self.ax.set_title('Koordinaten-Plot')
        self.ax.grid(True)  # Gitter beibehalten
        self.canvas.draw()

    def connect(self):
        address = self.device_var.get()
        if address:
            self.address = address
            self.print_position_thread = threading.Thread(target=self.print_position, args=())
            self.print_position_thread.start()

    def print_position(self):
        while True:
            position_data = self.loop.run_until_complete(self.get_poition_from_bluetooth())
            if position_data:
                print(position_data)
                string_parts = position_data.split(',')
                current_coordinate = [float(part) for part in string_parts]

                # Koordinaten hinzufügen
                self.coordinates['x'].append(current_coordinate[0])
                self.coordinates['y'].append(current_coordinate[1])

                # Plot aktualisieren
                self.update_plot()

            time.sleep(2.5)

            
    async def get_characteristic_value(self, address, characteristic):
        if not self.client.is_connected:
            self.client = bleak.BleakClient(address)
            await self.client.connect()
        value = await self.client.read_gatt_char(characteristic)
        return value.decode()

    async def get_services(self, address):
        if not self.client.is_connected:
            self.client = bleak.BleakClient(address)
            await self.client.connect()
        services = await self.client.get_services()
        return services
    
    def refresh_devices(self):
        self.refresh_thread = threading.Thread(target=self.refresh_devices_task, args=())
        self.refresh_thread.start()

    def refresh_devices_task(self):
        async def discover_devices():
            devices = await bleak.discover()
            return devices
        
        devices = self.loop.run_until_complete(discover_devices())
        self.device_menu["menu"].delete(0, tk.END)
        for d in devices:
            label = f"{d.address} - {d.name}" if d.name else d.address
            self.device_menu["menu"].add_command(label=label, command=tk._setit(self.device_var, d.address))

    def set_characteristic(self, value, characteristic_uuid):
        self.set_value_thread = threading.Thread(target=self.set_characteristic_thread, args=(value, characteristic_uuid))
        self.set_value_thread.start()

    def set_characteristic_thread(self, value, characteristic_uuid):
        async def set_characteristic_value(address, characteristic_uuid, value):
            if not self.client.is_connected:
                self.client = bleak.BleakClient(address)
                await self.client.connect()
            await self.client.write_gatt_char(characteristic_uuid, value.encode())

        address = self.device_var.get()
        if address and value:
            task = set_characteristic_value(address, characteristic_uuid, value)
            while self.loop.is_running():
                pass
            self.loop.run_until_complete(task)