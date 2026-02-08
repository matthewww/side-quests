

import json
import re
from collections import Counter

STOP_WORDS = {
    'a', 'an', 'the', 'and', 'with', 'for', 'to', 'in', 'on', 'of', 'is', 'at', 
    'your', 'you', 'aws', 'amazon', 'it', 'what', 'how', 'that', 'by', 
    'from', 'into', 'its', 'not', 'be', 'are', 'can', 'will', 'up', 'down'
}

def get_top_words(file_path, top_n=5):
    word_counts = Counter()
    with open(file_path, 'r') as f:
        for line in f:
            try:
                session = json.loads(line)
                title = session.get("item", {}).get("additionalFields", {}).get("title", "")
                if title:
                    # Normalize the text: lowercase, find all word sequences
                    words = re.findall(r'\b\w+\b', title.lower())
                    # Filter out stop words
                    filtered_words = [word for word in words if word not in STOP_WORDS]
                    word_counts.update(filtered_words)
            except json.JSONDecodeError:
                continue
    
    print(f"Top {top_n} most common words in session titles:")
    for word, count in word_counts.most_common(top_n):
        print(f"- {word.capitalize()}: {count} times")

if __name__ == "__main__":
    get_top_words('sessions.jsonl', top_n=20)

