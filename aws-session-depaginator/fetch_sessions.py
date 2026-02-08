
import requests
import json

def fetch_sessions():
    s = requests.Session()
    s.headers.update({
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/114.0.0.0 Safari/537.36'
    })

    with open('sessions.jsonl', 'w') as f:
        for page in range(18):
            url = f'https://aws.amazon.com/api/dirs/items/search?item.directoryId=events-cards-interactive-emea-event-agenda&item.locale=en_US&tags.id=GLOBAL%23local-tags-emea-event-agenda-event-name%23summit-johannesburg-2025&sort_by=item.dateCreated&sort_order=asc&size=8&page={page}'
            response = s.get(url)
            data = response.json()
            for item in data.get('items', []):
                f.write(json.dumps(item) + '\n')

if __name__ == '__main__':
    fetch_sessions()
