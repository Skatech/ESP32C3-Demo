#include <LittleFS.h>
#include <Update.h>

#include <util/format.h>
#include <Log.h>
#include <DelayedRestart.h>
#include <WebServerEx.h>
#include "WebUpdate.h"

void WebUpdate::setError(String&& message) {
    if (Update.isRunning()) {
        Update.end(true);
    }
    if (_file) {
        _file.close();
    }
    _status = ERROR;
    _error = message;
}

void WebUpdate::init(const __FlashStringHelper* uri, const __FlashStringHelper* pageFilePath) {
    _page = pageFilePath;
    HTTP.on(uri, HTTP_GET, [&]() {
        if (HTTP.isAuthenticated())
            HTTP.file(_page);
        else HTTP.notFound();
    });

    HTTP.on(uri, HTTP_POST, [&]() {
        if (_status == UPDATING_FIRMWARE || _status == UPDATING_FILESYSTEM) {
            String msg(F("Update OK, restarting device..."));
            HTTP.text(msg);
            SYS::restartAfter(500);
        }
        else if (_status == UPDATING_FILE) {
            String msg(F("Update OK"));
            HTTP.text(msg);
        }
        else  {
            String msg(format(F("Update FAILED: %s"), _error.c_str()));
            LOG.println(msg);
            HTTP.text(msg);
        }
        _status = READY;
    },
    [&]() {
        HTTPUpload& upload = HTTP.upload();
        if (upload.status == UPLOAD_FILE_START) {
            if (!HTTP.isAuthenticated()) {
                setError(F("Not authentified"));
            }
            else if (_status != READY) {
                setError(F("Not ready"));
            }
            else if (upload.name != F("file")) {
                setError(F("Bad source"));
            }
            else {
                if (upload.filename.startsWith(F("firmware")) && upload.filename.endsWith(F(".bin"))) {
                    uint32_t size = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
                    if (Update.begin(size, U_FLASH)) {
                        LOG.annotateVal(F("Updating firmware"), upload.filename);
                        _status = UPDATING_FIRMWARE;
                    }
                    else setError(F("Begin failed"));
                }
                else if (upload.filename.startsWith(F("littlefs")) && upload.filename.endsWith(F(".bin"))) {                        
                    if (Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS)) { // LittleFS.totalBytes()
                        LOG.annotateVal(F("Updating filesystem"), upload.filename);
                        _status = UPDATING_FILESYSTEM;
                    }
                    else setError(F("Begin failed"));
                }
                else {
                    String path = HTTP.arg(F("path"));
                    File file = LittleFS.open(path, "w");
                    if (file) {
                        LOG.annotateVal(F("Updating file"), path);
                        _status = UPDATING_FILE;
                        _file = file;
                    }
                    else setError(F("Bad path"));
                }
            };
        }
        else if (upload.status == UPLOAD_FILE_WRITE) {
            if (_status == UPDATING_FIRMWARE || _status == UPDATING_FILESYSTEM) {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                    setError(F("Write error"));
            }
            else if (_status == UPDATING_FILE) {
                if (_file.write(upload.buf, upload.currentSize) != upload.currentSize)
                    setError(F("Write error"));
            }
        }
        else if (upload.status == UPLOAD_FILE_END) {
            if (_status == UPDATING_FIRMWARE || _status == UPDATING_FILESYSTEM) {
                if (!Update.end(true))
                    setError(F("End error"));
            }
            else if (_status == UPDATING_FILE) {
                _file.close();
            }
        }
        else if(upload.status == UPLOAD_FILE_ABORTED) {
            setError(F("Aborted"));
        }
        yield();
    });
}
