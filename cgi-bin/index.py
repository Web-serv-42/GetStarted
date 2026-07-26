#!/usr/bin/env python3

import os
import sys
import html

# Read content length
content_length = int(os.environ.get("CONTENT_LENGTH", 0))

# Read raw text from stdin
data = sys.stdin.buffer.read(content_length)

# Decode text
text = data.decode("utf-8", errors="replace")

# Escape HTML to prevent injection
# safe_text = html.escape(text)

# Build HTML response
response = f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>PY CGI HTML Generated Page</title>
</head>
<body>
    <p>{text}</p>
</body>
</html>
"""

# while True:
#     continue

response_bytes = response.encode("utf-8")

# Send body
sys.stdout.buffer.write(response_bytes)