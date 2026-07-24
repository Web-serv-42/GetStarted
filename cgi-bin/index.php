#!/usr/bin/env php-cgi
<?php
// Read content length from environment
$content_length = intval(getenv('CONTENT_LENGTH') ? : 0);

// Read raw text from the safe input buffer wrapper
$data = "";
if ($content_length > 0)
{
    // FIX: php://input retains the body data even after PHP initializes
    $data = file_get_contents("php://input", false, null, 0, $content_length);
}

// In PHP, strings are binary-safe by default, no explicit decode needed.
// $safe_text = htmlspecialchars($data, ENT_QUOTES, 'UTF-8'); // To prevent injection

// Build HTML response
$response = "<!DOCTYPE html>
<html>
<head>
    <meta charset=\"UTF-8\">
    <title>CGI HTML Generated Page</title>
</head>
<body>
    <p>" . $data . "</p>
</body>
</html>";

// while (true) { usleep(100000); }

// Send body
echo $response;