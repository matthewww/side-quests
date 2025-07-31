# Audiobook Normalization Script

This script provides a suite of tools for organizing and normalizing a collection of audiobooks. It is designed to be run from the command line and can process multiple audiobook formats, automatically handling file conversions, metadata, and cover art.

## Features

- **File Consolidation**: Merges multiple audio files (e.g., `.mp3`, `.m4a`) into a single `.m4b` audiobook file.
- **Chapter Generation**: Automatically creates chapter markers from the individual filenames when combining files.
- **Cover Art Downloader**: If a cover image is missing, the script will search for and download appropriate cover art from the web.
- **Parallel Processing**: Leverages multiple CPU cores to process several audiobooks at once, significantly speeding up the workflow.
- **Metadata Reporting**: Generates a `metadata_report.jsonl` file with details about the audiobooks found and the processing tasks performed.

## Usage

To use the script, simply run it from your terminal:

```bash
python normalize-audiobooks.py
```

The script will automatically scan the `audiobooks` subdirectory for audiobook folders, process them, and place the final `.m4b` files in the `normalized_audiobooks` directory.

## Dependencies

- **FFmpeg**: This script relies on FFmpeg for all audio and video processing. You must have FFmpeg installed and available in your system's PATH.
- **Python 3**: The script is written in Python 3.
- **Python Libraries**: The script uses the `requests` library for downloading cover art. You can install it via pip:
  ```bash
  pip install requests
  ```
