import os
import subprocess
import shutil
import sys
import time
import json
import requests
from multiprocessing import Pool, cpu_count

# --- Configuration ---
ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "audiobooks")
OUTPUT_DIR_NAME = "normalized_audiobooks"
NORMALIZED_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), OUTPUT_DIR_NAME)
METADATA_REPORT_FILE = os.path.join(NORMALIZED_DIR, "metadata_report.jsonl")
AUDIO_EXTS = ['.mp3', '.m4a', '.m4b']
IMAGE_EXTS = ['.jpg', '.jpeg', '.png']

# --- Main Logic ---

def main():
    """Scan for metadata, then run parallel processing for audiobook conversion."""
    if sys.stdout.encoding != 'utf-8':
        sys.stdout.reconfigure(encoding='utf-8')
        sys.stderr.reconfigure(encoding='utf-8')

    # 1. Setup output directory
    setup_output_directory()

    # 2. Perform metadata pre-scan
    print("--- Starting Metadata Scan ---")
    normal_books, problem_books = scan_metadata()
    print(f"--- Metadata Scan Complete ---")
    print(f"Found {len(normal_books)} normal books and {len(problem_books)} problematic books to process.")
    print(f"A detailed report is available in {METADATA_REPORT_FILE}\n")

    # 3. Process normal books with user confirmation
    if normal_books:
        print("--- Planned Conversion (Normal Books) ---")
        for book in normal_books:
            print(f"- {book['dir_name']}")
        print()

        if not get_user_confirmation("Do you want to proceed with the conversion of normal books? (y/n): "):
            print("Conversion cancelled by user. Exiting.")
            sys.exit(0)

        num_processes = min(cpu_count(), len(normal_books))
        print(f"--- Starting Normal Audiobook Conversion using {num_processes} parallel processes ---")
        with Pool(processes=num_processes) as pool:
            pool.map(process_book_task, normal_books)

    # 4. Process problematic books with separate user confirmation
    if problem_books:
        print("\n--- Planned Conversion (Problematic Books) ---")
        for book in problem_books:
            print(f"- {book['dir_name']}")
        print()

        if get_user_confirmation("Some books were identified as problematic (e.g., ffprobe errors). Do you want to attempt to process them? (y/n): "):
            num_processes = min(cpu_count(), len(problem_books))
            print(f"--- Starting Problematic Audiobook Conversion using {num_processes} parallel processes ---")
            with Pool(processes=num_processes) as pool:
                pool.map(process_book_task, problem_books)
        else:
            print("Problematic book conversion skipped by user.")

    print("\n--- Normalization Process Complete ---")
    print(f"Your normalized audiobooks are in: {NORMALIZED_DIR}")

def get_user_confirmation(prompt):
    while True:
        try:
            choice = input(prompt).lower()
            if choice in ['y', 'yes']:
                return True
            elif choice in ['n', 'no']:
                return False
            else:
                print("Invalid input. Please enter 'y' or 'n'.")
        except (EOFError, KeyboardInterrupt):
            print("\nOperation cancelled by user.")
            return False

def get_book_details(dir_name):
    """Extracts and cleans author and title from a directory name."""
    parts = [p.strip() for p in dir_name.split(' - ')]

    
    if len(parts) >= 2:
        author = parts[0]
        title = ' - '.join(parts[1:])
        
        # Clean up title from common artifacts
        title = title.replace(f"by {author}", "").strip()
        title = title.replace("- mp3", "").strip()
        title = title.replace("mp3", "").strip()
        if title.lower().endswith("audiobook"):
            title = title[:-9].strip()
    else:
        author = "UnknownAuthor"
        title = dir_name

    return {"author": author, "title": title}


def scan_metadata():
    """Scan all book directories, report missing metadata, and return a list of tasks."""
    books_to_process = []
    normal_books = []
    problem_books = []

    # Use a temporary name for the report file to avoid conflict
    temp_report_path = os.path.join(ROOT_DIR, "metadata_report.jsonl.tmp")

    with open(temp_report_path, "w", encoding="utf-8") as report:
        for dir_name in sorted(os.listdir(ROOT_DIR)):
            dir_path = os.path.join(ROOT_DIR, dir_name)
            if not os.path.isdir(dir_path) or dir_name == OUTPUT_DIR_NAME:
                continue

            audio_files = []
            image_file = None
            is_problematic = False
            
            book_details = get_book_details(dir_name)
            clean_title = book_details["title"]

            for item in sorted(os.listdir(dir_path)):
                file_ext = os.path.splitext(item)[1].lower()
                if file_ext in AUDIO_EXTS:
                    full_audio_path = os.path.join(dir_path, item)
                    audio_files.append(full_audio_path)
                    if get_audio_duration(full_audio_path) is None:
                        is_problematic = True
                elif file_ext in IMAGE_EXTS and not image_file:
                    image_file = os.path.join(dir_path, item)
            
            if not audio_files:
                continue

            if not image_file:
                # Pass the cleaned title to the cover art downloader
                image_file = download_cover_art(clean_title, dir_path)

            metadata_status = {
                "book_title": dir_name,
                "cleaned_title": clean_title,
                "audio_files_found": len(audio_files),
                "cover_art_found": image_file is not None,
                "is_problematic": is_problematic
            }
            report.write(json.dumps(metadata_status) + "\n")
            
            book_info = {
                "dir_path": dir_path,
                "dir_name": dir_name,
                "audio_files": audio_files,
                "image_file": image_file,
                "is_problematic": is_problematic
            }

            if is_problematic:
                problem_books.append(book_info)
            else:
                normal_books.append(book_info)
    
    # Move the temporary report to the final destination
    if os.path.exists(METADATA_REPORT_FILE):
        os.remove(METADATA_REPORT_FILE)
    shutil.move(temp_report_path, METADATA_REPORT_FILE)

    return normal_books, problem_books


def setup_output_directory():
    """Create or clear the output directory for a clean run."""
    if not os.path.exists(NORMALIZED_DIR):
        print(f"Creating output directory: {NORMALIZED_DIR}")
        os.makedirs(NORMALIZED_DIR)
    else:
        print(f"Clearing output directory: {NORMALIZED_DIR}")
        for item in os.listdir(NORMALIZED_DIR):
            item_path = os.path.join(NORMALIZED_DIR, item)
            try:
                if os.path.isfile(item_path) or os.path.islink(item_path):
                    os.unlink(item_path)
                elif os.path.isdir(item_path):
                    shutil.rmtree(item_path)
            except Exception as e:
                print(f"  [Error] Failed to delete {item_path}. Reason: {e}")


def get_normalized_filename(dir_name):
    """Generates a normalized filename in the format BookTitleByAuthor.m4b."""
    book_details = get_book_details(dir_name)
    title = book_details["title"]
    author = book_details["author"]

    title_formatted = ''.join(filter(str.isalnum, title))
    author_formatted = ''.join(filter(str.isalnum, author))

    if not title_formatted:
        title_formatted = "Untitled"

    return f"{title_formatted}By{author_formatted}.m4b"
''


def process_book_task(task):
    """Wrapper function to handle a single book processing task for the pool."""
    start_time = time.time()
    dir_name = task['dir_name']
    print(f"[Processing] Starting: {dir_name}")

    # Create a dedicated output folder for the book
    book_output_dir = os.path.join(NORMALIZED_DIR, dir_name)
    if not os.path.exists(book_output_dir):
        os.makedirs(book_output_dir)

    normalized_filename = get_normalized_filename(dir_name)
    output_filename = os.path.join(book_output_dir, normalized_filename)

    try:
        if len(task['audio_files']) == 1 and task['audio_files'][0].lower().endswith('.m4b'):
            handle_single_m4b(task['audio_files'][0], task['image_file'], output_filename, dir_name)
        else:
            handle_multiple_files(task['audio_files'], task['image_file'], output_filename, dir_name, book_output_dir)
    except Exception as e:
        print(f"[Error] An unexpected error occurred while processing {dir_name}: {e}")

    elapsed_time = time.time() - start_time
    print(f"[Finished] Book: {dir_name} | Time: {elapsed_time:.2f}s | Output: {normalized_filename}")

def handle_single_m4b(input_m4b, image_file, output_filename, title):
    """Handles an existing M4B file, adding a cover if needed."""
    ffprobe_cmd = ["ffprobe", "-v", "error", "-select_streams", "v:0", "-show_entries", "stream=codec_type", "-of", "csv=p=0", input_m4b]
    result = subprocess.run(ffprobe_cmd, capture_output=True, text=True, check=False, encoding='utf-8')
    has_cover = "video" in result.stdout

    if has_cover:
        shutil.copy(input_m4b, output_filename)
    elif image_file:
        ffmpeg_cmd = ["ffmpeg", "-i", input_m4b, "-i", image_file, "-map", "0:a", "-map", "1:v", "-c:a", "copy", "-c:v", "png", "-disposition:v", "attached_pic", "-metadata", f"title={title}", output_filename]
        run_ffmpeg(ffmpeg_cmd, title)
    else:
        shutil.copy(input_m4b, output_filename)

def handle_multiple_files(audio_files, image_file, output_filename, title, temp_path):
    """Concatenates multiple audio files into a single M4B, with chapters."""
    list_filename = os.path.join(temp_path, "temp_file_list.txt")
    temp_audio_files = []

    # Re-encode problematic files if necessary
    for audio_file in audio_files:
        if os.path.splitext(audio_file)[1].lower() == '.m4a' and get_audio_duration(audio_file) is None:
            re_encoded_path = os.path.join(temp_path, os.path.basename(audio_file).replace('.m4a', '_reencoded.m4a'))
            if re_encode_audio_file(audio_file, re_encoded_path, title):
                temp_audio_files.append(re_encoded_path)
            else:
                # If re-encoding fails, fall back to original file (chapter will be skipped)
                temp_audio_files.append(audio_file)
        else:
            temp_audio_files.append(audio_file)

    with open(list_filename, "w", encoding="utf-8") as f:
        for audio_file in temp_audio_files:
            safe_path = audio_file.replace("\\", "/").replace("'", "'\\''")
            f.write(f"file '{safe_path}'\n")

    metadata_filename = create_chapter_metadata_file(temp_audio_files, temp_path)

    # --- FFmpeg Command Construction ---
    # 1. Define all inputs first
    ffmpeg_cmd = [
        "ffmpeg",
        "-f", "concat", "-safe", "0", "-i", list_filename,  # Input 0: Audio files list
        "-i", metadata_filename,                          # Input 1: Chapters metadata
    ]
    if image_file:
        ffmpeg_cmd.extend(["-i", image_file])             # Input 2: Cover image

    # 2. Map streams and metadata
    ffmpeg_cmd.extend(["-map", "0:a"]) # Map audio from input 0
    if image_file:
        ffmpeg_cmd.extend(["-map_metadata", "1"]) # Map chapters from input 1
        ffmpeg_cmd.extend(["-map", "2:v"])# Map video from input 2
    else:
        ffmpeg_cmd.extend(["-map_metadata", "1"])

    # 3. Set codecs and output options
    first_audio_ext = os.path.splitext(temp_audio_files[0])[1].lower()
    if first_audio_ext == '.mp3':
        ffmpeg_cmd.extend(["-c:a", "aac", "-b:a", "128k"])
    else:
        ffmpeg_cmd.extend(["-c:a", "copy"])

    if image_file:
        ffmpeg_cmd.extend(["-c:v", "png", "-disposition:v", "attached_pic"])

    ffmpeg_cmd.extend(["-metadata", f"title={title}", output_filename])
    
    run_ffmpeg(ffmpeg_cmd, title)

    # --- Cleanup ---
    try:
        os.remove(list_filename)
        os.remove(metadata_filename)
        for f in temp_audio_files:
            if "_reencoded.m4a" in f:
                os.remove(f)
    except OSError:
        pass

def create_chapter_metadata_file(audio_files, temp_path):
    """Generates a metadata file with chapter markers for each audio file."""
    metadata_filename = os.path.join(temp_path, "chapters_metadata.txt")
    total_duration_ms = 0
    with open(metadata_filename, "w", encoding="utf-8") as f:
        f.write(";FFMETADATA1\n")
        for i, audio_file in enumerate(audio_files):
            duration_s = get_audio_duration(audio_file)
            if duration_s is None:
                continue

            start_time = total_duration_ms
            end_time = total_duration_ms + int(duration_s * 1000)
            chapter_title = os.path.splitext(os.path.basename(audio_file))[0]

            f.write("[CHAPTER]\n")
            f.write("TIMEBASE=1/1000\n")
            f.write(f"START={start_time}\n")
            f.write(f"END={end_time}\n")
            f.write(f"title={chapter_title}\n")

            total_duration_ms = end_time
    return metadata_filename

def get_audio_duration(file_path):
    """Gets the duration of an audio file in seconds using ffprobe."""
    ffprobe_cmd = [
        "ffprobe", "-v", "error", "-show_entries", "format=duration",
        "-of", "default=noprint_wrappers=1:nokey=1", file_path
    ]
    try:
        result = subprocess.run(ffprobe_cmd, capture_output=True, text=True, check=True, encoding='utf-8')
        return float(result.stdout)
    except subprocess.CalledProcessError as e:
        error_message = e.stderr.strip().splitlines()[-1] if e.stderr else str(e)
        print(f"  [Warning] Could not get duration for {os.path.basename(file_path)}. Chapter will be skipped. Error: {error_message}")
        return None
    except (FileNotFoundError, ValueError) as e:
        print(f"  [Warning] Could not get duration for {os.path.basename(file_path)}. Chapter will be skipped. Error: {e}")
        return None

def re_encode_audio_file(input_file, output_path, title):
    """Re-encodes an audio file to AAC format."""
    print(f"  [Re-encode] Re-encoding {os.path.basename(input_file)} for {title}")
    ffmpeg_cmd = [
        "ffmpeg", "-i", input_file, "-c:a", "aac", "-b:a", "128k",
        output_path
    ]
    try:
        subprocess.run(ffmpeg_cmd, check=True, capture_output=True, text=True, encoding='utf-8')
        print(f"  [Re-encode] Successfully re-encoded {os.path.basename(input_file)}")
        return True
    except subprocess.CalledProcessError as e:
        stderr_lines = e.stderr.splitlines()
        print(f"  [Re-encode Error] Failed to re-encode {os.path.basename(input_file)} for {title}\n    => Details: {" ".join(stderr_lines[-5:])}")
        return False
    except FileNotFoundError:
        print("  [Critical Error] ffmpeg command not found. Please ensure FFmpeg is installed and in your system's PATH.")
        return False



def run_ffmpeg(command, title):
    """Executes an FFmpeg command and prints success or failure."""
    try:
        subprocess.run(command, check=True, capture_output=True, text=True, encoding='utf-8')
    except subprocess.CalledProcessError as e:
        stderr_lines = e.stderr.splitlines()
        print(f"  [FFmpeg Error] Failed on book: {title}\n    => Command: {' '.join(command)}\n    => Details: {" ".join(stderr_lines[-5:])}")
    except FileNotFoundError:
        print("  [Critical Error] ffmpeg command not found. Please ensure FFmpeg is installed and in your system's PATH.")

def download_cover_art(book_title, download_path):
    """Searches for and downloads cover art for a given book title using the Open Library API."""
    print(f"  [Cover Art] Searching for cover for: {book_title}")
    try:
        # 1. Search Open Library for the book
        search_url = f"http://openlibrary.org/search.json?title={requests.utils.quote(book_title)}&fields=*,isbn,cover_i"
        response = requests.get(search_url)
        response.raise_for_status()
        search_results = response.json()

        if not search_results.get('docs'):
            print(f"  [Cover Art] No search results found for: {book_title}")
            return None

        # 2. Find a valid ISBN for the book
        isbn = None
        # Iterate through the top search results to find a valid ISBN
        for doc in search_results.get('docs', []):
            # Prioritize entries that have both an ISBN and a cover ID
            if doc.get('isbn') and doc.get('cover_i'):
                isbn = doc['isbn'][0]
                break
        
        # If no ideal match is found, take the first available ISBN
        if not isbn:
            for doc in search_results.get('docs', []):
                if doc.get('isbn'):
                    isbn = doc['isbn'][0]
                    break

        if not isbn:
            print(f"  [Cover Art] No ISBN found for: {book_title}")
            return None

        # 3. Fetch the cover using the ISBN
        cover_url = f"http://covers.openlibrary.org/b/isbn/{isbn}-L.jpg"
        print(f"  [Cover Art] Downloading from: {cover_url}")
        response = requests.get(cover_url, stream=True)
        response.raise_for_status()

        # Check if a valid image was returned
        if 'content-type' in response.headers and 'image' in response.headers['content-type']:
            # Save the image
            image_filename = os.path.join(download_path, "cover.jpg")
            with open(image_filename, 'wb') as f:
                for chunk in response.iter_content(chunk_size=8192):
                    f.write(chunk)
            print(f"  [Cover Art] Successfully downloaded cover to: {image_filename}")
            return image_filename
        else:
            print(f"  [Cover Art] No cover image found for ISBN: {isbn}")
            return None

    except requests.exceptions.RequestException as e:
        print(f"  [Cover Art] An error occurred while communicating with Open Library for {book_title}: {e}")
        return None
    except Exception as e:
        print(f"  [Cover Art] An unexpected error occurred while downloading cover art for {book_title}: {e}")
        return None

if __name__ == "__main__":
    main()