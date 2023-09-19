import tkinter as tk
import tkinter.simpledialog as simpledialog
from lib.widget import Widget
from lib.ble_service_widget import BleServiceWidget
from lib.ble_config_widget import BleConfigWidget
from lib.serial_printer import SerialWidget
import json

class Dashboard(tk.Frame):
    def __init__(self, parent, **kwargs):
        super().__init__(parent, **kwargs)

        self.widgets = []
        self.maxindex = 0
        self.islocked = False

        self.configure(bg="#555555")  # Color the Border of the Window
        self.canvas = tk.Canvas(self, bg="#333333")  # Color the Canvas of the Application
        #self.canvas = tk.Canvas(self, bg="white")
        self.canvas.pack(fill="both", expand=True, padx=10, pady=10)
        self.canvas.config(width=700, height=500)

        self.add_widget_button = tk.Button(self, text="add widget", command=self.add_widget)
        self.add_widget_button.pack(side="left")

        self.delete_widget_button = tk.Button(self, text="delete widget", command=self.delete_widget)
        self.delete_widget_button.pack(side="left")

        self.save_button = tk.Button(self, text="save dashboard", command=self.save_dashboard)
        self.save_button.pack(side="left")

        self.load_button = tk.Button(self, text="load dashboard", command=self.load_dashboard)
        self.load_button.pack(side="left")

        self.lock_button = tk.Button(self, text="lock dashboard", command=self.toggle_resize_and_drag)
        self.lock_button.pack(side="right")
        try:
            self.load_dashboard()
            self.toggle_resize_and_drag()
        except:
            pass

    def add_widget(self):
        widget_types = ["BleServiceWidget", "BleConfigWidget", "SerialWidget", "MasterWidget"]
        dialog = MyDialog(self, "Select Widget Type", widget_types)
        result = dialog.result
        if result is not None:
            name = str(len(self.widgets) + 1) + ". - " + result
            if result == "BleServiceWidget":
                widget = BleServiceWidget(self.canvas, name)
                widget.place(x=0, y=0, width=600, height=280, anchor="nw")
            elif result == "BleConfigWidget":
                widget = BleConfigWidget(self.canvas, name)
                widget.place(x=0, y=0, width=600, height=280, anchor="nw")
            elif result == "SerialWidget":
                widget = SerialWidget(self.canvas, name)
                widget.place(x=0, y=0, width=550, height=260, anchor="nw")
            elif result == "MasterWidget":
                widget = Widget(self.canvas, name)
                widget.place(x=0, y=0, width=150, height=100, anchor="nw")
            self.widgets.append(widget)
            self.maxindex = self.maxindex + 1


    def delete_widget(self):
        if self.widgets:
            selection = simpledialog.askinteger("Delete Widget", "Enter index of widget to delete:", minvalue=1, maxvalue=self.maxindex)
            if not selection:
                return  # cancel was clicked
            for widget in self.widgets:
                if widget.name.startswith(str(selection)):
                    widget.destroy()
                    self.widgets.remove(widget)

    def save_dashboard(self):
        widget_list = []
        for widget in self.widgets:
            widget_data = {
                'name': widget.name,
                'type': widget.type,
                'x': widget.winfo_x(),
                'y': widget.winfo_y(),
                'width': widget.winfo_width(),
                'height': widget.winfo_height()
            }
            widget_list.append(widget_data)
        data = {'widgets': widget_list, 'max_index': self.maxindex}
        with open('dashboard.json', 'w') as f:
            json.dump(data, f)

    def load_dashboard(self):
        with open('dashboard.json', 'r') as f:
            data = json.load(f)
        widget_list = data['widgets']
        for widget in self.widgets:
            widget.destroy()
        self.widgets = []
        for widget_data in widget_list:
            if widget_data['type'] == "BleServiceWidget":
                widget = BleServiceWidget(self.canvas, widget_data['name'])
            elif widget_data['type'] == "BleConfigWidget":
                widget = BleConfigWidget(self.canvas, widget_data['name'])
            elif widget_data['type'] == "SerialWidget":
                widget = SerialWidget(self.canvas, widget_data['name'])
            else:
                widget = Widget(self.canvas, widget_data['name'])
            widget.place(x=widget_data['x'], y=widget_data['y'], width=widget_data['width'], height=widget_data['height'], anchor="nw")
            self.widgets.append(widget)
        self.maxindex = data['max_index']

    def toggle_resize_and_drag(self):
        if self.islocked == False:
            for widget in self.widgets:
                widget.disable_interaction()
            self.islocked = True
            self.lock_button.configure(text="unlock dashboard")
            self.add_widget_button['state'] = 'disabled'
            self.delete_widget_button['state'] = 'disabled'
            self.save_button['state'] = 'disabled'
            self.load_button['state'] = 'disabled'
        else:
            for widget in self.widgets:
                widget.enable_interaction()
            self.islocked = False
            self.lock_button.configure(text="lock dashboard")
            self.add_widget_button['state'] = 'normal'
            self.delete_widget_button['state'] = 'normal'
            self.save_button['state'] = 'normal'
            self.load_button['state'] = 'normal'

class MyDialog(simpledialog.Dialog):
    def __init__(self, parent, title, options):
        self.options = options
        self.selected_option = tk.StringVar()
        super().__init__(parent, title=title)

    def body(self, master):
        label_frame = tk.Frame(master)
        label_frame.pack(side="top", padx=5, pady=5)
        tk.Label(label_frame, text="Please select a Widget-Type:").pack(side="left")
        menu_frame = tk.Frame(master)
        menu_frame.pack(side="bottom", padx=5, pady=5)
        tk.OptionMenu(menu_frame, self.selected_option, *self.options).pack(side="left", padx=5)
        return tk.Label(master, text="")

    def apply(self):
        self.result = self.selected_option.get()


if __name__ == "__main__":
    root = tk.Tk()
    root.geometry("800x600")

    dashboard = Dashboard(root)
    dashboard.pack(fill="both", expand=True)

    root.mainloop()
