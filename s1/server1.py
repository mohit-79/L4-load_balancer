# server1.py

from http.server import BaseHTTPRequestHandler, HTTPServer
import time

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        time.sleep(5)  # 5 second delay

        self.send_response(200)
        self.send_header("Content-type", "text/plain")
        self.end_headers()

        self.wfile.write(
            b"Hello from Server 1"
        )

HTTPServer(
    ("0.0.0.0", 8081),
    Handler
).serve_forever()