

import json
import re
from collections import Counter

# Using a slightly expanded set of stop words to filter out more noise.
STOP_WORDS = {
    'a', 'an', 'the', 'and', 'with', 'for', 'to', 'in', 'on', 'of', 'is', 'at', 
    'your', 'you', 'ai', 'aws', 'amazon', 'it', 'what', 'how', 'that', 'by', 
    'from', 'into', 'its', 'not', 'be', 'are', 'can', 'will', 'up', 'down',
    'g', 'vs', 'go', 'q', 'all', 'out', 'get', 'use', 'using'
}

def get_least_common_words(file_path):
    word_counts = Counter()
    with open(file_path, 'r') as f:
        for line in f:
            try:
                session = json.loads(line)
                title = session.get("item", {}).get("additionalFields", {}).get("title", "")
                if title:
                    words = re.findall(r'\b\w+\b', title.lower())
                    filtered_words = [word for word in words if word not in STOP_WORDS]
                    word_counts.update(filtered_words)
            except json.JSONDecodeError:
                continue
    
    # Find all words that only appear once
    weird_words = [word for word, count in word_counts.items() if count == 1]
    
    print("A selection of the most unique or 'weird' words (appearing only once):")
    # Sort them alphabetically for readability and take a sample
    for word in sorted(weird_words)[:25]: # Limiting to 25 for readability
        print(f"- {word.capitalize()}")

if __name__ == "__main__":
    get_least_common_words('sessions.jsonl')

