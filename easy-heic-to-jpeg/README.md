# HEIC to JPEG Converter

A simple GUI tool to batch convert HEIC images as native to iPhone, to JPEG format using Python, [CustomTkinter](https://github.com/TomSchimansky/CustomTkinter), [Pillow](https://github.com/python-pillow/Pillow), and [pillow-heif](https://github.com/bigcat88/pillow_heif).

## Features

- Convert single or multiple `.heic` files to `.jpg`
- Batch convert all HEIC files in a folder
- Adjustable JPEG quality
- Progress bar and success/failure notifications

## Requirements

- Python 3.7+
- [Pillow](https://pypi.org/project/Pillow/)
- [pillow-heif](https://pypi.org/project/pillow-heif/)
- [CustomTkinter](https://pypi.org/project/customtkinter/)

Install dependencies with:

```sh
pip install pillow pillow-heif customtkinter
```

## Usage

1. Run the script:

    ```sh
    python easy-heic-to-jpeg.py
    ```

3. Use the GUI to select files or a folder, adjust JPEG quality, and start converting.