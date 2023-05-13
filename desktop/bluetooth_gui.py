import asyncio
import tkinter as tk
import bleak

#class BluetoothGUI:
#    def __init__(self):
#        self.window = tk.Tk()
#        self.window.title("Bluetooth Device Viewer")
#
#        self.device_label = tk.Label(self.window, text="Select device:")
#        self.device_label.pack()
#
#        self.device_var = tk.StringVar(value="")
#        self.device_menu = tk.OptionMenu(self.window, self.device_var, "")
#        self.device_menu.pack()
#
#        self.refresh_button = tk.Button(self.window, text="Refresh", command=self.refresh_devices)
#        self.refresh_button.pack()
#
#        self.services_button = tk.Button(self.window, text="Show Services", command=self.show_services)
#        self.services_button.pack()
#
#        self.services_text = tk.Text(self.window)
#        self.services_text.pack()
#
#        self.service_label = tk.Label(self.window, text="Service:")
#        self.service_label.pack()
#
#        self.service_entry = tk.Entry(self.window)
#        self.service_entry.pack()
#
#        self.characteristic_label = tk.Label(self.window, text="Characteristic:")
#        self.characteristic_label.pack()
#
#        self.characteristic_entry = tk.Entry(self.window)
#        self.characteristic_entry.pack()
#
#        self.value_label = tk.Label(self.window, text="Value:")
#        self.value_label.pack()
#
#        self.value_entry = tk.Entry(self.window)
#        self.value_entry.pack()
#
#        self.set_button = tk.Button(self.window, text="Publish Value", command=self.set_characteristic)
#        self.set_button.pack()
#
#    def set_characteristic(self):
#        address = self.device_var.get()
#        service_uuid = self.service_entry.get()
#        characteristic_uuid = self.characteristic_entry.get()
#        value = self.value_entry.get()
#        if address and value:
#            asyncio.run(self.set_characteristic_value(address, service_uuid, characteristic_uuid, value))
#        self.show_services()
#
#    async def set_characteristic_value(self, address, service_uuid, characteristic_uuid, value):
#        async with bleak.BleakClient(address) as client:
#            await client.write_gatt_char(characteristic_uuid, value.encode())
#
#    async def get_services(self, address):
#        async with bleak.BleakClient(address) as client:
#            services = await client.get_services()
#            return services
#        
#    async def get_descriptor(self, address, descriptor):
#        async with bleak.BleakClient(address) as client:
#            value = await client.read_gatt_descriptor(descriptor.handle)
#            return value.decode()
#        
#    def show_services(self):
#        address = self.device_var.get()
#        if address:
#            services = asyncio.run(self.get_services(address))
#            self.services_text.delete(1.0, tk.END)
#            for service in services:
#                self.services_text.insert(tk.END, f"UUID: {service.uuid}\n", "bold")
#                self.services_text.insert(tk.END, f"|Handle: {service.handle}\n")
#                self.services_text.insert(tk.END, "|Characteristics:\n")
#                for characteristic in service.characteristics:
#                    self.services_text.insert(tk.END, f"|--------UUID: {characteristic.uuid}\n", "bold")
#                    self.services_text.insert(tk.END, f"|\tHandle: {characteristic.handle}\n")
#                    self.services_text.insert(tk.END, f"|\tProperties: {characteristic.properties}\n")
#                    if not f"{characteristic.uuid}".startswith("0000"):
#                        try:
#                            descriptor = characteristic.descriptors[0] # get the first descriptor
#                            description = asyncio.run(self.get_descriptor(address, descriptor))
#                            self.services_text.insert(tk.END, f"|\tDescripton: \"{description}\"\n")
#                        except bleak.exc.BleakError:
#                            self.services_text.insert(tk.END, f"|\tDescripton: Unable to read Description\n")
#                        try:
#                            value = self.get_characteristic_value(address, service.uuid, characteristic.uuid)
#                            self.services_text.insert(tk.END, f"|\tValue: {value}\n")
#                        except bleak.exc.BleakError:
#                            self.services_text.insert(tk.END, f"|\tValue: Unable to read value\n")
#                self.services_text.insert(tk.END, f"\n")
#
#
#    def get_characteristic_value(self, address, service_uuid, characteristic_uuid):
#        async def inner():
#            async with bleak.BleakClient(address) as client:
#                value = await client.read_gatt_char(characteristic_uuid)
#                return value.decode()
#        try:
#            return asyncio.run(inner())
#        except UnicodeDecodeError as e:
#            print(f"Error decoding characteristic value: {e}")
#            return None

class BluetoothGUI:
    def __init__(self):
        self.window = tk.Tk()
        self.window.title("Bluetooth Device Viewer")

        self.device_label = tk.Label(self.window, text="Select device:")
        self.device_label.pack()

        self.device_var = tk.StringVar(value="")
        self.device_menu = tk.OptionMenu(self.window, self.device_var, "")
        self.device_menu.pack()

        self.refresh_button = tk.Button(self.window, text="Refresh", command=self.refresh_devices)
        self.refresh_button.pack()

        self.services_button = tk.Button(self.window, text="Show Services", command=self.show_services)
        self.services_button.pack()

        self.services_text = tk.Text(self.window)
        self.services_text.pack()

        self.characteristic_label = tk.Label(self.window, text="Characteristic:")
        self.characteristic_label.pack()

        self.characteristic_entry = tk.Entry(self.window)
        self.characteristic_entry.pack()

        self.value_label = tk.Label(self.window, text="Value:")
        self.value_label.pack()

        self.value_entry = tk.Entry(self.window)
        self.value_entry.pack()

        self.set_button = tk.Button(self.window, text="Publish Value", command=self.set_characteristic)
        self.set_button.pack()

        self.client = None
        self.loop = asyncio.get_event_loop()
        self.refresh_devices()

    def set_characteristic(self):
        address = self.device_var.get()
        characteristic_uuid = self.characteristic_entry.get()
        value = self.value_entry.get()
        if address and value and characteristic_uuid:
            task = self.set_characteristic_value(address, characteristic_uuid, value)
            #self.loop.create_task(task)
            self.loop.run_until_complete(task)
            self.show_services()

    async def set_characteristic_value(self, address, characteristic_uuid, value):
        if not self.client:
            self.client = bleak.BleakClient(address)
            await self.client.connect()
        await self.client.write_gatt_char(characteristic_uuid, value.encode())

    async def get_services(self, address):
        if not self.client:
            self.client = bleak.BleakClient(address)
            await self.client.connect()
        services = await self.client.get_services()
        return services
        
    async def get_descriptor(self, address, descriptor):
        if not self.client:
            self.client = bleak.BleakClient(address)
            await self.client.connect()
        value = await self.client.read_gatt_descriptor(descriptor.handle)
        return value.decode()
    
    async def get_characteristic_value(self, address, characteristic_uuid):
        if not self.client:
            self.client = bleak.BleakClient(address)
            await self.client.connect()
        value = await self.client.read_gatt_char(characteristic_uuid)
        return value.decode()
        
    def show_services(self):
        address = self.device_var.get()
        if address:
            task = self.print_service_infos(address)
            #self.loop.create_task(task)
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

    async def discover_devices(self):
        devices = await bleak.discover()
        return devices

    def refresh_devices(self):
        devices = asyncio.run(self.discover_devices())
        self.device_menu["menu"].delete(0, tk.END)
        for d in devices:
            label = f"{d.address} - {d.name}" if d.name else d.address
            self.device_menu["menu"].add_command(label=label, command=tk._setit(self.device_var, d.address))

    def run(self):
        self.window.mainloop()

if __name__ == "__main__":
    gui = BluetoothGUI()
    gui.run()
