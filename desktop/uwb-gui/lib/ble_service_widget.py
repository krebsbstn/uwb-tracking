from lib.widget import Widget
import threading
import asyncio
import tkinter as tk
import bleak

class BleServiceWidget(Widget):
    def __init__(self, parent, the_name):
        super().__init__(parent, the_name)
        self.type="BleServiceWidget"
        self.border.configure(bg="grey")

        self.refresh_thread = None
        self.service_thread = None
        self.set_value_thread = None

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


        self.services_button = tk.Button(self.top_frame, text="Show Services", command=self.show_services)
        self.services_button.pack(side="left")

        # second Frame with Widgets
        self.bottom_frame = tk.Frame(self.border)
        self.bottom_frame.pack(side="bottom", fill="x")

        self.characteristic_label = tk.Label(self.bottom_frame, text="Characteristic:")
        self.characteristic_label.pack(side="left")

        self.characteristic_entry = tk.Entry(self.bottom_frame)
        self.characteristic_entry.pack(side="left")

        self.value_label = tk.Label(self.bottom_frame, text="Value:")
        self.value_label.pack(side="left")

        self.value_entry = tk.Entry(self.bottom_frame)
        self.value_entry.pack(side="left")

        self.set_button = tk.Button(self.bottom_frame, text="Publish Value", command=self.set_characteristic)
        self.set_button.pack(side="left")
        
        # Textfield for Services
        self.services_text = tk.Text(self.border)
        self.services_text.pack(side="left", fill='both', expand=True, padx=5, pady=5)

        self.client = bleak.BleakClient("")
        asyncio.set_event_loop(asyncio.new_event_loop())
        self.loop = asyncio.get_event_loop()
    
    def destroy(self) -> None:
        if self.service_thread is not None:
            self.service_thread.join()
        if self.refresh_thread is not None:
            self.refresh_thread.join()
        if self.set_value_thread is not None:
            self.set_value_thread.join()
        return super().destroy()
    
    def set_characteristic(self):
        self.set_value_thread = threading.Thread(target=self.set_characteristic_thread)
        self.set_value_thread.start()

    def set_characteristic_thread(self):
        async def set_characteristic_value(address, characteristic_uuid, value):
            if not self.client.is_connected:
                self.client = bleak.BleakClient(address)
                await self.client.connect()
            await self.client.write_gatt_char(characteristic_uuid, value.encode())

        address = self.device_var.get()
        characteristic_uuid = self.characteristic_entry.get()
        value = self.value_entry.get()
        if address and value and characteristic_uuid:
            task = set_characteristic_value(address, characteristic_uuid, value)
            self.loop.run_until_complete(task)
            self.show_services()

    def show_services(self):
        address = self.device_var.get()
        if address:
            self.service_thread = threading.Thread(target=self.print_service_infos_thread, args=(address,))
            self.service_thread.start()

    def print_service_infos_thread(self, address):
        task = self.print_service_infos(address)
        self.loop.run_until_complete(task)

    async def print_service_infos(self, address):
        services = await self.get_services(address)
        self.services_text.delete(1.0, tk.END)
        for service in services:
            self.services_text.insert(tk.END, f"UUID: {service.uuid}\n", "bold")
            self.services_text.insert(tk.END, f"|Handle: {service.handle}\n")
            self.services_text.insert(tk.END, "|Characteristics:\n")
            for characteristic in service.characteristics:
                self.services_text.insert(tk.END, f"|--------UUID: {characteristic.uuid}\n", "bold")
                self.services_text.insert(tk.END, f"|\tHandle: {characteristic.handle}\n")
                self.services_text.insert(tk.END, f"|\tProperties: {characteristic.properties}\n")
                if not f"{characteristic.uuid}".startswith("0000"):
                    try:
                        descriptor = characteristic.descriptors[0] # get the first descriptor
                        description = await self.get_descriptor(address, descriptor)
                        self.services_text.insert(tk.END, f"|\tDescripton: \"{description}\"\n")
                    except bleak.exc.BleakError:
                        self.services_text.insert(tk.END, f"|\tDescripton: Unable to read Description\n")
                    try:
                        value = await self.get_characteristic_value(address, characteristic.uuid)
                        self.services_text.insert(tk.END, f"|\tValue: {value}\n")
                    except bleak.exc.BleakError:
                        self.services_text.insert(tk.END, f"|\tValue: Unable to read value\n")
            self.services_text.insert(tk.END, f"\n")

    async def get_services(self, address):
        if not self.client.is_connected:
            self.client = bleak.BleakClient(address)
            await self.client.connect()
        services = await self.client.get_services()
        return services
        
    async def get_descriptor(self, address, descriptor):
        if not self.client.is_connected:
            self.client = bleak.BleakClient(address)
            await self.client.connect()
        value = await self.client.read_gatt_descriptor(descriptor.handle)
        return value.decode()
    
    async def get_characteristic_value(self, address, characteristic_uuid):
        if not self.client.is_connected:
            self.client = bleak.BleakClient(address)
            await self.client.connect()
        value = await self.client.read_gatt_char(characteristic_uuid)
        return value.decode()
    
    def refresh_devices(self):
        self.refresh_thread = threading.Thread(target=self.refresh_devices_thread)
        self.refresh_thread.start()

    def refresh_devices_thread(self):
        async def discover_devices():
            devices = await bleak.discover()
            return devices
        
        devices = asyncio.run(discover_devices())
        self.device_menu["menu"].delete(0, tk.END)
        for d in devices:
            label = f"{d.address} - {d.name}" if d.name else d.address
            self.device_menu["menu"].add_command(label=label, command=tk._setit(self.device_var, d.address))
