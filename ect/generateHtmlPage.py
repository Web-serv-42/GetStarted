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
    <title>CGI HTML Generated Page</title>
</head>
<body>
    <p>{text}</p>
</body>
</html>
"""

while True:
    continue
    

response_bytes = response.encode("utf-8")

# Send headers
# sys.stdout.buffer.write(b"Content-Type: text/html; charset=utf-8\r\n")
# sys.stdout.buffer.write(f"Content-Length: {len(response_bytes)}\r\n".encode())
# sys.stdout.buffer.write(b"\r\n")

# Send body
sys.stdout.buffer.write(response_bytes)