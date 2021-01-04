#ifndef NOTFOUNDPAGE_INO
#define NOTFOUNDPAGE_INO 1

void handleNotFound() {
    server.send(404, F("text/plain"), F("404: Not found"));
}

#endif
