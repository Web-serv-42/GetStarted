#!/usr/bin/env python3
import cgi
import cgitb

# Enable detailed error reporting in the browser (useful for debugging)
cgitb.enable()

# Create instance of FieldStorage to hold form data
form = cgi.FieldStorage()

# Get data from a form field named 'user_name'
name = form.getvalue('user', 'Guest')

print("Content-Type: text/html\r\n")
print(f"<h1>Hello, {name}!</h1>")
