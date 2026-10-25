#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace epp {

struct Element {
    std::string kind;
    std::vector<std::string> values;
};

struct Document {
    std::string title = "E++ App";
    std::vector<Element> elements;
};

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    bool quoted = false;
    bool escaped = false;

    for (char character : line) {
        if (escaped) {
            token += character;
            escaped = false;
        } else if (character == '\\' && quoted) {
            escaped = true;
        } else if (character == '"') {
            quoted = !quoted;
        } else if (!quoted && character == '#') {
            break;
        } else if (!quoted && std::isspace(static_cast<unsigned char>(character))) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += character;
        }
    }

    if (quoted) {
        throw std::runtime_error("unterminated quoted value");
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string lower(std::string value) {
    for (char& character : value) {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return value;
}

std::string urlScheme(const std::string& url) {
    const std::size_t separator = url.find(':');
    if (separator == std::string::npos || url.find('/') < separator || url.find('?') < separator || url.find('#') < separator) {
        return "";
    }
    return lower(url.substr(0, separator));
}

std::string htmlEscape(const std::string& value, bool quote = false) {
    std::ostringstream output;
    for (char character : value) {
        switch (character) {
            case '&': output << "&amp;"; break;
            case '<': output << "&lt;"; break;
            case '>': output << "&gt;"; break;
            case '"': output << (quote ? "&quot;" : "\""); break;
            case '\'': output << (quote ? "&#39;" : "'"); break;
            default: output << character; break;
        }
    }
    return output.str();
}

Document parse(const std::string& source) {
    Document document;
    std::istringstream input(source);
    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(input, line)) {
        ++lineNumber;
        std::vector<std::string> parts;
        try {
            parts = tokenize(line);
        } catch (const std::runtime_error& error) {
            throw std::runtime_error("line " + std::to_string(lineNumber) + ": " + error.what());
        }
        if (parts.empty()) {
            continue;
        }

        std::string command = parts[0];
        if (!command.empty() && command.back() == ':') {
            command.pop_back();
        }
        if (command == "title" && parts.size() == 2) {
            document.title = parts[1];
        } else if (command == "heading" && parts.size() == 2) {
            document.elements.push_back({"heading", {parts[1]}});
        } else if (command == "text" && parts.size() == 2) {
            document.elements.push_back({"text", {parts[1]}});
        } else if (command == "button" && parts.size() == 3) {
            document.elements.push_back({"button", {parts[1], parts[2]}});
        } else if (command == "link" && parts.size() == 3) {
            const std::string scheme = urlScheme(parts[2]);
            if (scheme != "" && scheme != "http" && scheme != "https" && scheme != "mailto") {
                throw std::runtime_error("line " + std::to_string(lineNumber) + ": unsupported link URL scheme");
            }
            document.elements.push_back({"link", {parts[1], parts[2]}});
        } else {
            throw std::runtime_error("line " + std::to_string(lineNumber) + ": invalid statement or arguments for '" + command + "'");
        }
    }
    return document;
}

std::string render(const Document& document) {
    std::ostringstream output;
    std::string body;
    for (const Element& element : document.elements) {
        if (element.kind == "heading") {
            body += "      <h1>" + htmlEscape(element.values[0]) + "</h1>\n";
        } else if (element.kind == "text") {
            body += "      <p>" + htmlEscape(element.values[0]) + "</p>\n";
        } else if (element.kind == "button") {
            body += "      <button type=\"button\" data-message=\"" + htmlEscape(element.values[1], true) + "\">" + htmlEscape(element.values[0]) + "</button>\n";
        } else if (element.kind == "link") {
            body += "      <a href=\"" + htmlEscape(element.values[1], true) + "\">" + htmlEscape(element.values[0]) + "</a>\n";
        }
    }

    output << "<!doctype html>\n"
           << "<html lang=\"en\">\n"
           << "  <head>\n"
           << "    <meta charset=\"utf-8\">\n"
           << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
           << "    <title>" << htmlEscape(document.title) << "</title>\n"
           << "    <style>\n"
           << "      :root { color-scheme: light; font-family: \"Trebuchet MS\", sans-serif; color: #183b35; background: #f3f6ed; }\n"
           << "      body { box-sizing: border-box; max-width: 760px; min-height: 100vh; margin: 0 auto; padding: 12vh 24px; }\n"
           << "      main { border-left: 5px solid #e15a36; padding: 8px 0 8px 28px; }\n"
           << "      h1 { font-family: Georgia, serif; font-size: 2.5rem; margin: 0 0 16px; }\n"
           << "      p { line-height: 1.6; max-width: 52ch; }\n"
           << "      button, a { display: inline-block; margin: 12px 12px 0 0; border: 0; border-radius: 4px; padding: 11px 16px; background: #176b5b; color: white; font: inherit; text-decoration: none; cursor: pointer; }\n"
           << "      a { background: #e15a36; }\n"
           << "      button:focus-visible, a:focus-visible { outline: 3px solid #183b35; outline-offset: 3px; }\n"
           << "    </style>\n"
           << "  </head>\n"
           << "  <body>\n"
           << "    <main>\n"
           << body
           << "    </main>\n"
           << "    <script>\n"
           << "      document.addEventListener(\"click\", (event) => {\n"
           << "        const button = event.target.closest(\"button[data-message]\");\n"
           << "        if (button) window.alert(button.dataset.message);\n"
           << "      });\n"
           << "    </script>\n"
           << "  </body>\n"
           << "</html>\n";
    return output.str();
}

} // namespace epp

#ifndef EPP_NO_MAIN
int main(int argc, char** argv) {
    std::string outputPath;
    std::string sourcePath;

    for (int index = 1; index < argc; ++index) {
        std::string argument = argv[index];
        if (argument == "--out") {
            if (++index >= argc) {
                std::cerr << "epp: --out requires a path\n";
                return 1;
            }
            outputPath = argv[index];
        } else if (argument == "--help" || argument == "-h") {
            std::cout << "Usage: epp <source.epp> [--out <output.html>]\n";
            return 0;
        } else if (argument.rfind("-", 0) == 0) {
            std::cerr << "epp: unknown option '" << argument << "'\n";
            return 1;
        } else {
            sourcePath = argument;
        }
    }

    if (sourcePath.empty()) {
        std::cerr << "epp: missing source file\nUsage: epp <source.epp> [--out <output.html>]\n";
        return 1;
    }

    try {
        std::ifstream sourceFile(sourcePath);
        if (!sourceFile) {
            throw std::runtime_error("cannot open source file: " + sourcePath);
        }
        std::ostringstream sourceBuffer;
        sourceBuffer << sourceFile.rdbuf();

        epp::Document document = epp::parse(sourceBuffer.str());
        if (outputPath.empty()) {
            std::filesystem::path source(sourcePath);
            outputPath = source.replace_extension(".html").string();
        }

        std::ofstream outputFile(outputPath);
        if (!outputFile) {
            throw std::runtime_error("cannot write output file: " + outputPath);
        }
        outputFile << epp::render(document);
        std::cout << "Built " << outputPath << "\n";
    } catch (const std::exception& error) {
        std::cerr << "epp: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
#endif
