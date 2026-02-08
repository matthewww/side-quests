
import json

def find_sessions_by_type(file_path, session_types):
    results = {stype: [] for stype in session_types}
    with open(file_path, 'r') as f:
        for line in f:
            try:
                session = json.loads(line)
                title = session.get("item", {}).get("additionalFields", {}).get("title", "No Title")
                body = session.get("item", {}).get("additionalFields", {}).get("body", "")
                
                duration_str = "(Time not available)"
                if "<br>" in body:
                    try:
                        time_part = body.split("<br>")[1].strip()
                        start_str, end_str = [t.strip() for t in time_part.split('-')]
                        start_h, start_m = map(int, start_str.split(':'))
                        end_h, end_m = map(int, end_str.split(':'))
                        duration = (end_h * 60 + end_m) - (start_h * 60 + start_m)
                        duration_str = f"({duration} minutes)"
                    except (ValueError, IndexError):
                        duration_str = "(Time format error)"

                tags = session.get("tags", [])
                for tag in tags:
                    if tag.get("name") in session_types:
                        session_type_name = tag["name"]
                        results[session_type_name].append(f"{title} {duration_str}")
            except json.JSONDecodeError:
                continue
    
    for stype, sessions in results.items():
        print(f"--- {stype} Sessions ---")
        if sessions:
            for s in sorted(sessions):
                print(s)
        else:
            print(f"No sessions found for type: {stype}")
        print("")

if __name__ == "__main__":
    find_sessions_by_type('sessions.jsonl', ["Interactive Training", "Workshop"])
