#pragma once

#include <Arduino.h>

#include <util/noncopyable.h>
#include <util/format.h>

class WebPageBuilder {
    String _header, _title, _styles, _links, _scripts;

public:
    NONCOPYABLE(WebPageBuilder)
    WebPageBuilder(String&& header = F(""), String&& title = F(DEVICE_TITLE)) {
        _header = std::move(header); _title = std::move(title);
    }

    WebPageBuilder& addStyle(const String& style) {
        _styles += format(F("<style>%s</style>"), style.c_str());
        return *this;
    }

    WebPageBuilder& addLink(const String& text, const String& href) {
        _links += format(F("<a href=\"%s\">%s</a>"), href.c_str(), text.c_str());
        return *this;
    }

    WebPageBuilder& addIndexLink(const String& text = F("INDEX")) {
        return addLink(text, F("/"));
    }

    WebPageBuilder& addAdminLink() {
        return addLink(F("ADMIN"), F("/admin"));
    }

    WebPageBuilder& addReloadLink(const String& text = F("RELOAD")) {
        return addLink(text, F("javascript:window.location.href=window.location.href"));
    }

    WebPageBuilder& addScript(const String& code) {
        _scripts += format(F("<script>%s</script>"), code.c_str());
        return *this;
    }

    WebPageBuilder& addReloadScript(uint32_t millis, const String& location = emptyString) {
        String value = location.isEmpty() ? String(F("window.location.href")) : ('"' + location + '"');
        return addScript(format(F("setTimeout(e => window.location.href = %s, %lu)"), value.c_str(), millis));
    }

    String toString(const String& content) {
        String html = F(R"(
            <!DOCTYPE html>
            <head>
                <meta charset="utf-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <link rel="icon" href="data:,">
                <link rel="stylesheet" href="/styles.css">
                {{STYLES}}
                <title>{{TITLE}}</title>
            </head>
            <h1>{{HEADER}}</h1>
            <hr>
            <div class="links">{{LINKS}}</div>
            <div class="content">{{CONTENT}}</div>
            {{SCRIPTS}}
            )");
        html.replace(F("{{STYLES}}"), _styles);
        html.replace(F("{{TITLE}}"), _title);
        html.replace(F("{{HEADER}}"), _header.isEmpty() ? _title : _header);
        html.replace(F("{{LINKS}}"), _links);
        html.replace(F("{{CONTENT}}"), content);
        html.replace(F("{{SCRIPTS}}"), _scripts);
        return html;
    }
};
