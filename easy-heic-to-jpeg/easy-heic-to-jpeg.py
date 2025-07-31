import os
import customtkinter as ctk
from tkinter import filedialog, messagebox
from PIL import Image
import pillow_heif

# === Conversion Function ===
def convert_heic_to_jpeg(input_path, output_dir, quality):
    try:
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        output_path = os.path.join(output_dir, base_name + ".jpg")

        heif_file = pillow_heif.read_heif(input_path)
        image = Image.frombytes(
            heif_file.mode,
            heif_file.size,
            heif_file.data,
            "raw",
        )
        image.save(output_path, "JPEG", quality=quality)
        return True
    except Exception as e:
        print(f"Error converting {input_path}: {e}")
        return False

# === File Handling ===
def process_files(files):
    output_dir = filedialog.askdirectory(title="Choose output folder")
    if not output_dir:
        return

    total = len(files)
    progress_bar.set(0)  # Reset progress bar to 0
    success = 0

    for i, path in enumerate(files):
        if convert_heic_to_jpeg(path, output_dir, quality_var.get()):
            success += 1
        progress_bar.set((i + 1) / total)  # Update progress as a fraction
        app.update_idletasks()

    messagebox.showinfo("Done", f"Converted {success}/{total} files")

def select_files():
    files = filedialog.askopenfilenames(filetypes=[("HEIC files", "*.heic")])
    if files:
        process_files(files)

def select_folder():
    folder = filedialog.askdirectory(title="Choose folder with HEIC files")
    if folder:
        heic_files = [
            os.path.join(folder, f)
            for f in os.listdir(folder)
            if f.lower().endswith(".heic")
        ]
        if heic_files:
            process_files(heic_files)
        else:
            messagebox.showinfo("No HEIC", "No HEIC files found in this folder.")

# === App UI ===
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

app = ctk.CTk()
app.geometry("500x400")
app.title("HEIC to JPEG Converter")

title = ctk.CTkLabel(app, text="Convert HEIC → JPEG", font=ctk.CTkFont(size=22, weight="bold"))
title.pack(pady=20)

quality_var = ctk.IntVar(value=90)
quality_label = ctk.CTkLabel(app, text="JPEG Quality")
quality_label.pack()
quality_slider = ctk.CTkSlider(app, from_=50, to=100, variable=quality_var, number_of_steps=10)
quality_slider.pack(pady=10, fill="x", padx=40)

batch_var = ctk.BooleanVar(value=True)
batch_toggle = ctk.CTkCheckBox(app, text="Enable batch folder conversion", variable=batch_var)
batch_toggle.pack(pady=10)

button_frame = ctk.CTkFrame(app, fg_color="transparent")
button_frame.pack(pady=10)

file_btn = ctk.CTkButton(button_frame, text="Select Files", command=select_files)
file_btn.grid(row=0, column=0, padx=10)

folder_btn = ctk.CTkButton(button_frame, text="Select Folder", command=select_folder)
folder_btn.grid(row=0, column=1, padx=10)

exit_btn = ctk.CTkButton(app, text="Exit", command=app.destroy, fg_color="gray")
exit_btn.pack(pady=(0, 10))

progress_bar = ctk.CTkProgressBar(app, orientation="horizontal", mode="determinate")
progress_bar.set(0)
progress_bar.pack(pady=20, fill="x", padx=40)

app.mainloop()
