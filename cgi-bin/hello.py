#!/usr/bin/env python3

import os
import sys
import platform

method = os.environ.get('REQUEST_METHOD', '')
body = ''
if method == 'POST':
    try:
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        body = sys.stdin.read(content_length)
    except:
        body = '[couldn’t read POST data]'

# env info
os_name = platform.system()
os_version = platform.version()
python_version = platform.python_version()
cwd = os.getcwd()
user = os.environ.get('USER') or os.environ.get('USERNAME') or 'Unknown'

print("Content-Type: text/plain\r\n\r\n")

print("✅ CGI script executed successfully!\n")
print(f"👤 User: {user}")
print(f"📦 Python version: {python_version}")
print(f"🖥 OS: {os_name} {os_version}")
print(f"📂 Current working directory: {cwd}")
print(f"🧾 Method: {method}")
print(f"📨 POST body: {body}")

