import tkinter as tk

class Widget(tk.Frame):
    def __init__(self, parent, the_name, **kwargs):
        super().__init__(parent, **kwargs)
        self.type="MasterWidget"
        
        self.border = tk.Frame(self, bd=1, relief="solid", bg="red")
        self.border.pack(fill="both", expand=True)

        self.title_frame = tk.Frame(self.border, bd=0, bg=self.border.cget('bg'))
        self.title_frame.pack(side="top", fill="x", padx=0, pady=0)

        self.name = the_name
        self.title_label = tk.Label(self.title_frame, text=self.name, font=("Helvetica", 12, "bold"), anchor="w")
        self.title_label.pack(side="left", fill="both", expand=True)
        self.title_label.configure(bg='grey')


        self.dragging = False
        self.resizing = False
        self.resize_corner = None
        self.last_x = 0
        self.last_y = 0
        self.enable_interaction()

    def enable_interaction(self):
        self.title_label.bind("<ButtonPress-1>", self.start_interaction)
        self.title_label.bind("<B1-Motion>", self.do_interaction)
        self.title_label.bind("<ButtonRelease-1>", self.stop_interaction)
        self.title_label.bind("<Motion>", self.update_cursor)

        self.border.bind("<ButtonPress-1>", self.start_interaction)
        self.border.bind("<B1-Motion>", self.do_interaction)
        self.border.bind("<ButtonRelease-1>", self.stop_interaction)
        self.border.bind("<Motion>", self.update_cursor)

    def disable_interaction(self):
        self.title_label.unbind("<ButtonPress-1>")
        self.title_label.unbind("<B1-Motion>")
        self.title_label.unbind("<ButtonRelease-1>")
        self.title_label.unbind("<Motion>")

        self.border.unbind("<ButtonPress-1>")
        self.border.unbind("<B1-Motion>")
        self.border.unbind("<ButtonRelease-1>")
        self.border.unbind("<Motion>")

    """Interaction splits functionality of resize and drag&drop"""
    def start_interaction(self, event):
        if self.get_resize_corner(event).startswith("bottom"):
            self.start_resize(event)
        else:
            self.start_drag(event)

    def do_interaction(self, event):
        if self.resizing:
            self.do_resize(event)
        if self.dragging:
            self.do_drag(event)
    
    def stop_interaction(self, event):
        if self.resizing:
            self.stop_resize(event)
        if self.dragging:
            self.stop_drag(event)

    """drag&drop functionality"""
    def start_drag(self, event):
        self.dragging = True
        self.last_x = event.x_root
        self.last_y = event.y_root
        self.lift()

    def do_drag(self, event):
        if self.dragging:
            dx = event.x_root - self.last_x
            dy = event.y_root - self.last_y
            x, y = self.winfo_x() + dx, self.winfo_y() + dy
            if x < 0:
                x = 0
            if y < 0:
                y = 0
            canvas_width = self.master.winfo_width() - self.winfo_width()
            if x > canvas_width:
                x = canvas_width
            canvas_height = self.master.winfo_height() - self.winfo_height()
            if y > canvas_height:
                y = canvas_height
            self.place(x=x, y=y, anchor="nw")
            self.last_x = event.x_root
            self.last_y = event.y_root


    def stop_drag(self, event):
        self.dragging = False

    """resize functionality"""
    def start_resize(self, event):
        if self.get_resize_corner(event):
            self.resizing = True
            self.resize_corner = self.get_resize_corner(event)
            self.last_x = event.x_root
            self.last_y = event.y_root
            self.lift()

    def do_resize(self, event):
        if self.resizing:
            dx = event.x_root - self.last_x
            dy = event.y_root - self.last_y
            if self.resize_corner == "bottom_left_corner":
                new_x = self.winfo_x() + dx
                new_width = self.winfo_width() - dx
                new_height = self.winfo_height() + dy
                self.place(x=new_x, y=self.winfo_y(), anchor="nw")
                self.place(width=new_width, height=new_height)
            elif self.resize_corner == "bottom_right_corner":
                new_width = self.winfo_width() + dx
                new_height = self.winfo_height() + dy
                self.place(width=new_width, height=new_height)
            self.last_x = event.x_root
            self.last_y = event.y_root

    def stop_resize(self, event):
        self.resizing = False
        self.resize_corner = None

    def get_resize_corner(self, event):
        """Returns the corner where the resize is taking place"""
        x = event.x
        y = event.y
        if x < 10 and y > self.winfo_height() - 10:
            return "bottom_left_corner"
        elif x > self.winfo_width() - 10 and y > self.winfo_height() - 10:
            return "bottom_right_corner"
        elif y < 20:
            return "fleur"
        else:
            return None

    def update_cursor(self, event):
        """Updates the cursor based on the position of the mouse"""
        cursor = self.get_resize_corner(event)
        if cursor is None:
            cursor = ""
        self.border.config(cursor=cursor)

