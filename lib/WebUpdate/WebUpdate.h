#pragma once

#include <Arduino.h>

#include <util/noncopyable.h>

class WebUpdate {
    const __FlashStringHelper* _page;
    enum Status { READY, ERROR, UPDATING_FILE, UPDATING_FIRMWARE, UPDATING_FILESYSTEM };
    Status _status = READY;
    String _error;
    File _file;

    void setError(String&& message);

public:
    NONCOPYABLE(WebUpdate)
    WebUpdate() {}

    // Arguments:
    //  uri - route for update GET and POST requests
    //  pageFilePath - path to GET request html file
    void init(const __FlashStringHelper* uri = F("/admin/update"),
        const __FlashStringHelper* pageFilePath = F("/webui/pages/device-update.html")); 
};
