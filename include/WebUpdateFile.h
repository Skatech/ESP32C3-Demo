#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <Update.h>

#include <util/noncopyable.h>
#include <util/format.h>
#include <Log.h>
#include "WebPageBuilder.h"
#include "WebServerEx.h"

class WebUpdateFile {
    const __FlashStringHelper* _page;

    File _file;
    String _error;

public:
    NONCOPYABLE(WebUpdateFile)
    WebUpdateFile() {}

    // Arguments:
    //  uri - route for update GET and POST requests like "/admin/update-file"
    //  pageFilePath - name of GET request html file. like "/webui/pages/file-update.html",
    void init(const __FlashStringHelper* uri, const __FlashStringHelper* pageFilePath) {
        _page = pageFilePath;
        HTTP.on(uri, HTTP_GET, [&]() {
            if (HTTP.isAuthenticated())
                HTTP.file(_page);
            else HTTP.notFound();
        });

        HTTP.on(uri, HTTP_POST, [&]() {
            WebPageBuilder wpb(F("File UPDATE"));
            wpb.addIndexLink().addAdminLink().addLink(F("UPDATE-FILE"), HTTP.uri());
            if (_error.isEmpty()) {
                HTTP.html(wpb.addReloadScript(3000, "/admin").toString(F("<h2>SUCCEEDED</h2>")));
            }
            else  {
                LOG.annotateVal(F("File update FAILED, error"), _error);
                HTTP.html(wpb.toString(format(F("<h2>FAILED</h2><h4>ERROR: %s</h4>"), _error.c_str())));
            }

            // HTTP.html(WebPageBuilder().addReloadScript(3000, "/admin")
            //     .toString(_error.isEmpty() ? String(F("<h2>SUCCEEDED</h2>"))
            //         : format(F("<h2>FAILED</h2><h4>ERROR: %s</h4>"), _error.c_str()), F("File UPDATE")));
            // HTTP.sendMesssagePageHTML(F("File UPDATE"), F("File UPDATE"),
            //     F("<a href='/admin'>ADMIN</a>"), _error.isEmpty() ? F("SUCCEEDED") : F("FAILED"), _error);
        },
        [&]() {
            HTTPUpload& upload = HTTP.upload();
            if (upload.status == UPLOAD_FILE_START) {
                if (!HTTP.isAuthenticated()) {
                    setError(F("Unathentified update"));
                }
                else if (_file) {
                    setError(F("Another in progress"));
                }
                else if (upload.name != F("file")) {
                    setError(F("Unsupported source"));
                }
                else {
                    String path = HTTP.arg(F("path"));
                    if (path.startsWith(F("/webui/"))) {
                        LOG.annotateVal(F("Updating file"), path);
                        _file = LittleFS.open(path, "w");
                        if(!_file)
                            setError(F("Failed to create file"));
                    }
                    else setError(format(F("Invalid destination: '%s'"), HTTP.arg(F("path")).c_str()));
                }
            }
            else if (_file && upload.status == UPLOAD_FILE_WRITE) {
                _file.write(upload.buf, upload.currentSize);
            }
            else if (_file && upload.status == UPLOAD_FILE_END) {
                _file.close();
            }
            else if(upload.status == UPLOAD_FILE_ABORTED) {
                setError(F("Aborted"));
            }
            yield();
        });
    }

    void setError(String&& message) {
        _error = message;
        _file.close();
    }
};
