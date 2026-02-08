# AWS Summit Agenda Analysis: Key Insights and Queries

Here is a summary of the key insights we gathered from analyzing the AWS Summit session data, along with all the `jq` and shell commands used to extract this information.

## Key Insights

1.  **Session Volume**: There are 138 sessions in total, but 30 of these are repeated, leaving 108 unique sessions.
2.  **Missing Information**:
    *   35 sessions are missing a start and end time in the data.
    *   A large number of these are **Breakout Sessions** (18) and **Chalk Talks** (9).
    *   However, every session has a session type assigned.
3.  **Session Duration**:
    *   Session lengths vary significantly, from 15 minutes to 150 minutes.
    *   The most common duration is **45 minutes** (68 sessions).
4.  **Session Scheduling Patterns**:
    *   The busiest time slot is **12:00**, with 17 concurrent sessions.
    *   There is a strong correlation between the start time and the session type:
        *   **On the hour**: Mostly Breakout Sessions.
        *   **:20 & :40**: Exclusively Lightning Talks.
        *   **:30**: Exclusively Chalk Talks.
        *   **:45**: Exclusively Breakout Sessions.

## `jq` and Shell Queries Used

Below are the commands used to analyze the `sessions.jsonl` file.

### Finding Unique Locations and Session Types

-   **Unique Locations**:
    ```bash
    jq -r '.item.additionalFields.location' sessions.jsonl | sort | uniq
    ```
-   **Unique Session Types**:
    ```bash
    jq -r '.tags[] | select(.tagNamespaceId == "GLOBAL#local-tags-emea-event-agenda-session-type").name' sessions.jsonl | sort | uniq
    ```

### Analyzing Session Repetition and Duration

-   **Find Repeated Sessions**:
    ```bash
    jq -r '.item.additionalFields.title' sessions.jsonl | sort | uniq -c | sort -nr | grep -v '^\s*1 '
    ```
-   **Calculate and Summarize Session Durations**:
    ```bash
    jq -r '.item.additionalFields.body' sessions.jsonl | awk -F '<br> ' '/<br>/{print $2}' | sed 's/ //g' | awk -F '-' '{
        start_parts_count = split($1, start_parts, ":");
        end_parts_count = split($2, end_parts, ":");
        if (start_parts_count == 2 && end_parts_count == 2) {
            start_time = start_parts[1] * 60 + start_parts[2];
            end_time = end_parts[1] * 60 + end_parts[2];
            duration = end_time - start_time;
            print duration " minutes";
        }
    }' | sort -n | uniq -c | sort -nr
    ```

### Analyzing Session Timings and Schedules

-   **Get Unique Start Times**:
    ```bash
    jq -r '.item.additionalFields.body' sessions.jsonl | grep '<br>' | awk -F '<br> ' '{print $2}' | cut -d'-' -f1 | sed 's/ //g' | grep -E '^[0-9]{2}:[0-9]{2}$' | sort -u
    ```
-   **Count Sessions per Start Time (Ordered by Count)**:
    ```bash
    jq -r '.item.additionalFields.body' sessions.jsonl | grep '<br>' | awk -F '<br> ' '{print $2}' | cut -d'-' -f1 | sed 's/ //g' | grep -E '^[0-9]{2}:[0-9]{2}$' | sort | uniq -c | sort -n -k1,1r
    ```
-   **Count Sessions per Start Time (Ordered by Time)**:
    ```bash
    jq -r '.item.additionalFields.body' sessions.jsonl | grep '<br>' | awk -F '<br> ' '{print $2}' | cut -d'-' -f1 | sed 's/ //g' | grep -E '^[0-9]{2}:[0-9]{2}$' | sort | uniq -c | sort -k2
    ```
-   **Total Number of Sessions with a Start Time**:
    ```bash
    jq -r '.item.additionalFields.body' sessions.jsonl | grep '<br>' | awk -F '<br> ' '{print $2}' | cut -d'-' -f1 | sed 's/ //g' | grep -E '^[0-9]{2}:[0-9]{2}$' | wc -l
    ```

### Correlating Session Types and Times

-   **Correlate Start Times with Session Types**:
    ```bash
    jq -r 'select(.item.additionalFields.body | contains("<br>")) | . as $session | ($session.item.additionalFields.body | split("<br> ")[1] | split("-")[0] | gsub(" "; "")) as $startTime | ($session.tags[] | select(.tagNamespaceId == "GLOBAL#local-tags-emea-event-agenda-session-type").name) as $sessionType | "\($startTime)\t\($sessionType)"' sessions.jsonl | grep -E '^[0-9]{2}:[0-9]{2}' | sort | uniq -c | sort -k2,2 -k1,1nr
    ```
-   **Count Non-Breakout Sessions per Start Time**:
    ```bash
    jq -r 'select(.item.additionalFields.body | contains("<br>")) | . as $session | ($session.item.additionalFields.body | split("<br> ")[1] | split("-")[0] | gsub(" "; "")) as $startTime | ($session.tags[] | select(.tagNamespaceId == "GLOBAL#local-tags-emea-event-agenda-session-type").name) as $sessionType | select($sessionType != "Breakout Session") | $startTime' sessions.jsonl | grep -E '^[0-9]{2}:[0-9]{2}$' | sort | uniq -c | sort -k2
    ```
-   **List Non-Breakout Session Types per Start Time**:
    ```bash
    jq -r 'select(.item.additionalFields.body | contains("<br>")) | . as $session | ($session.item.additionalFields.body | split("<br> ")[1] | split("-")[0] | gsub(" "; "")) as $startTime | ($session.tags[] | select(.tagNamespaceId == "GLOBAL#local-tags-emea-event-agenda-session-type").name) as $sessionType | select($sessionType != "Breakout Session") | "\($startTime)\t\($sessionType)"' sessions.jsonl | grep -E '^[0-9]{2}:[0-9]{2}' | sort -k1,1 | awk -F'\\t' '{
        if ($1 != prev_time && prev_time != "") {
            print prev_time ": " count " sessions [" types "]"
            types = ""
            count = 0
        }
        if ($1 != prev_time) {
            prev_time = $1
            types = $2
            count = 1
        } else {
            types = types ", " $2
            count++
        }
    } END {
        print prev_time ": " count " sessions [" types "]"
    }'
    ```

### Finding Sessions with Missing Information

-   **Find Sessions Without a Session Type**:
    ```bash
    jq -r 'select(all(.tags[]; .tagNamespaceId != "GLOBAL#local-tags-emea-event-agenda-session-type")) | .item.additionalFields.title' sessions.jsonl
    ```
-   **Find Sessions Without a Time**:
    ```bash
    jq -r 'select(.item.additionalFields.body | contains("<br>") | not) | .item.additionalFields.title' sessions.jsonl
    ```
-   **Find Session Types for Talks Without a Time**:
    ```bash
    jq -r 'select(.item.additionalFields.body | contains("<br>") | not) | .tags[] | select(.tagNamespaceId == "GLOBAL#local-tags-emea-event-agenda-session-type").name' sessions.jsonl | sort | uniq -c | sort -nr
    ```
