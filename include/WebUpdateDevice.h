#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <Update.h>

#include <util/noncopyable.h>
#include <util/format.h>
#include <Log.h>
#include <DelayedRestart.h>
// #include "WebServerEx.h"
#include "WebPageBuilder.h"

class WebUpdateDevice {
    const __FlashStringHelper* _page;
    String _error;

public:
    NONCOPYABLE(WebUpdateDevice)
    WebUpdateDevice() {}

    // Arguments:
    //  uri - route for OTA update GET and POST requests like "/admin/update"
    //  pageFilePath - name of GET request html file. like "/webui/pages/device-update.html",
    void init(const __FlashStringHelper* uri, const __FlashStringHelper* pageFilePath) {
        _page = pageFilePath;
        HTTP.on(uri, HTTP_GET, [&]() {
            if (HTTP.isAuthenticated())
                HTTP.file(_page);
            else HTTP.notFound();
        });

        HTTP.on(uri, HTTP_POST, [&]() {
            // HTTP.sendHeader(F("Connection"), F("close"));
            WebPageBuilder wpb(F("Device UPDATE"));
            wpb.addIndexLink().addAdminLink().addLink(F("UPDATE"), HTTP.uri());
            if (_error.isEmpty()) {
                LOG.println(F("Update succeeded, restarting device..."));
                HTTP.client().setNoDelay(true);
                HTTP.html(wpb.addReloadScript(3000, "/admin").toString(
                    F("<h2>SUCCEEDED</h2><h4>Wait until device restarted...</h4>")));
                RESTART.after(500);
            }
            else {
                LOG.println(String(F("Update FAILED, error: ")) + _error);
                HTTP.html(wpb.toString(format(F("<h2>FAILED</h2><h4>ERROR: %s</h4>"), _error.c_str())));
            }
        },
        [&]() {
            HTTPUpload& upload = HTTP.upload();
            if (upload.status == UPLOAD_FILE_START) {
                _error = emptyString;
                if (!HTTP.isAuthenticated()) {
                    setError(F("Unathentified update"));
                }
                else if (!upload.filename.startsWith(upload.name)) {
                    setError(F("Unsupported source"));
                }
                else if (upload.name == F("littlefs")) {
                    LOG.println(F("Starting filesystem update... "));
                    if(!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS)) // LittleFS.totalBytes()
                        setError(F("Update begin failed"));
                }
                else if (upload.name == F("firmware")) {
                    LOG.println(F("Starting firmware update... "));
                    uint32_t size = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
                    LOG.println(format(F("UpdateFW CALC size: %lu"), size));
                    LOG.println(format(F("UploadFW size: %lu"), upload.totalSize));
                    if (!Update.begin(size, U_FLASH))
                        setError(F("Update begin failed"));
                    LOG.println(format(F("UpdateFW.size(): %lu"), Update.size()));
                }
                else setError(F("Unsupported destination"));
            }
            else if (_error.isEmpty() && Update.isRunning() && upload.status == UPLOAD_FILE_WRITE) {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                    setError(F("Update write failed"));
            }
            else if (_error.isEmpty() && Update.isRunning() && upload.status == UPLOAD_FILE_END) {
                if (!Update.end(true))
                    setError(F("Update end failed"));
            }
            else if(upload.status == UPLOAD_FILE_ABORTED) {
                setError(F("Upload aborted"));
                Update.end();
            } 
            yield();
        });
    }
    
    void setError(const __FlashStringHelper* message) {
        _error = Update.hasError() ? Update.errorString() : String(message);
    }

    String resultPageHTML() {
        String html(F(R"(
            <!DOCTYPE html>
            <head>
                <meta charset="utf-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <link rel="icon" href="data:,">
                <link rel="stylesheet" href="/styles.css">
                <title>Update</title>
            </head>
            <h1>Update {TITLE}</h1>
            <hr>
            <div class="links">
                <a href="/">INDEX</a>
                <a href="javascript:window.location.href=window.location.href">UPDATE</a>
            </div>
            <h3>{DESCR}</h3>)"));
        html.replace(F("{TITLE}"), _error.isEmpty() ? F("SUCCEEDED") : F("FAILED"));
        html.replace(F("{DESCR}"), _error.isEmpty() ? F("Restarting device...") : _error);
        return html;
    }
};
