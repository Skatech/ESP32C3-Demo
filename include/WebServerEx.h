#pragma once

#include <WebServer.h>
#include <libb64/cencode.h>
#include <detail/mimetable.h>

#include <util/noncopyable.h>
#include <util/format.h>
#include <util/ip.h>
#include <TaskScheduler.h>

class WebServerEx : public WebServer, public Task {
    IPAddress _addrfrom, _addrlast;
    String _authdata;

protected:
    uint16_t __task_trottle() override { return WiFi.isConnected() ? 1U : 0U; }
    void __task_proc() { handleClient(); }

public:
    NONCOPYABLE(WebServerEx)
    WebServerEx(uint16_t port): WebServer(port) { }

    void text(const String& text, int code = 200) {
        send(code, F("text/plain"), text);
    }

    void html(const String& html, int code = 200) {
        send(code, F("text/html"), html);
    }

    void redirect(const String& location = F("/")) {
        sendHeader(F("Location"), location, true); text(emptyString, 303);
    }

    void forbidden() {
        text(F("Forbidden"), 403);
    }

    void notFound() {
        if (_notFoundHandler)
            _notFoundHandler();
        else text(F("Not found"), 404);
    }

    void file(const String& filepath, int code = 200) {
        File file = LittleFS.open(filepath, "r");
        if (file) {
            streamFile(file, getContentType(filepath)); // stream(file, getContentType(filepath));
            file.close();
        }
        else notFound();
    }

    String getContentType(const String& path) {
        for (size_t i = 0; i < mime::maxType; ++i)
            if (path.endsWith(FPSTR(mime::mimeTable[i].endsWith)))
                return FPSTR(mime::mimeTable[i].mimeType);
        return FPSTR(mime::mimeTable[mime::type::none].mimeType);
    }

    bool setAuthData(const String& user, const String& pass, const IPAddress& from, const IPAddress& last) {
        _addrfrom = from; _addrlast = last;
        String data = user + ':' + pass;
        if (base64_encode_expected_len(data.length()) < 64) {
            char buff[64];
            if (base64_encode_chars(data.c_str(), data.length(), buff)) {
                _authdata = String(F("Basic ")) + buff;
                return true;
            }
        }
        return false;
    }

    bool setAuthData(String&& data) {
        tokenizer st(std::move(data));
        String user = st.next(), pass = st.next();
        IPAddress from = IP::fromString(st.next()), last = IP::fromString(st.next());
        return setAuthData(user, pass, from, last);
    }

    bool isAuthenticated() {
        return IP::isInRange(client().remoteIP(), _addrfrom, _addrlast) &&
            (_authdata.isEmpty() || header(F("Authorization")).equalsConstantTime(_authdata));
    }
};

extern WebServerEx HTTP;