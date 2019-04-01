import os
import time
import SocketServer
import SimpleHTTPServer

PORT = 8081

class HTTPCacheRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def send_head(self):
        if self.command != "POST" and self.headers.get('If-Modified-Since', None):
            filename = self.path.strip("/")
            if os.path.isfile(filename):
                a = time.gmtime(os.path.getmtime(filename))
                b = time.strptime(self.headers.get('If-Modified-Since', None), "%a, %d %b %Y %H:%M:%S GMT")
                if a < b:
                    self.send_response(304)
                    self.end_headers()
                    return None
        return SimpleHTTPServer.SimpleHTTPRequestHandler.send_head(self)

    def end_headers(self):
        filename = self.path.strip("/")
        if filename == "2.binary":
            self.send_header('Cache-control', 'no-cache')
        else:
            self.send_header('Cache-control', 'must-revalidate')
        SimpleHTTPServer.SimpleHTTPRequestHandler.end_headers(self)

s = SocketServer.ThreadingTCPServer(("", PORT), HTTPCacheRequestHandler)
s.allow_reuse_address = True
print "Serving on port", PORT
s.serve_forever()