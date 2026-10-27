#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace epp_gui {

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
        if (escaped) { token += character; escaped = false; }
        else if (character == '\\' && quoted) escaped = true;
        else if (character == '"') quoted = !quoted;
        else if (!quoted && character == '#') break;
        else if (!quoted && std::isspace(static_cast<unsigned char>(character))) {
            if (!token.empty()) { tokens.push_back(token); token.clear(); }
        } else token += character;
    }
    if (quoted) throw std::runtime_error("unterminated quoted value");
    if (!token.empty()) tokens.push_back(token);
    return tokens;
}

std::string escape(const std::string& value, bool attribute = false) {
    std::string result;
    for (char character : value) {
        switch (character) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += attribute ? "&quot;" : "\""; break;
            case '\'': result += attribute ? "&#39;" : "'"; break;
            default: result += character;
        }
    }
    return result;
}

Document parse(const std::string& source) {
    Document document;
    std::istringstream input(source);
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        std::vector<std::string> parts;
        try { parts = tokenize(line); }
        catch (const std::exception& error) { throw std::runtime_error("line " + std::to_string(lineNumber) + ": " + error.what()); }
        if (parts.empty()) continue;
        std::string command = parts[0];
        if (!command.empty() && command.back() == ':') command.pop_back();
        if (command == "title" && parts.size() == 2) document.title = parts[1];
        else if ((command == "heading" || command == "text") && parts.size() == 2) document.elements.push_back({command, {parts[1]}});
        else if (command == "button" && parts.size() == 3) document.elements.push_back({command, {parts[1], parts[2]}});
        else if (command == "link" && parts.size() == 3) {
            std::string url = parts[2];
            std::size_t colon = url.find(':');
            std::string scheme = colon == std::string::npos ? "" : url.substr(0, colon);
            for (char& character : scheme) character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
            if (scheme != "" && scheme != "http" && scheme != "https" && scheme != "mailto")
                throw std::runtime_error("line " + std::to_string(lineNumber) + ": unsupported link URL scheme");
            document.elements.push_back({command, {parts[1], url}});
        } else throw std::runtime_error("line " + std::to_string(lineNumber) + ": invalid GUI statement or arguments for '" + command + "'");
    }
    return document;
}

std::string render(const Document& document) {
    std::ostringstream body;
    for (const Element& element : document.elements) {
        if (element.kind == "heading") body << "      <h1>" << escape(element.values[0]) << "</h1>\n";
        else if (element.kind == "text") body << "      <p>" << escape(element.values[0]) << "</p>\n";
        else if (element.kind == "button") body << "      <button type=\"button\" data-message=\"" << escape(element.values[1], true) << "\">" << escape(element.values[0]) << "</button>\n";
        else if (element.kind == "link") body << "      <a href=\"" << escape(element.values[1], true) << "\">" << escape(element.values[0]) << "</a>\n";
    }
    std::ostringstream output;
    output << "<!doctype html>\n<html lang=\"en\">\n<head>\n"
           << "  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
           << "  <title>" << escape(document.title) << "</title>\n"
           << "  <style>\n"
           << "    :root { color-scheme: light; font-family: \"Trebuchet MS\", sans-serif; color: #183b35; background: #f3f6ed; }\n"
           << "    body { box-sizing: border-box; max-width: 760px; min-height: 100vh; margin: 0 auto; padding: 12vh 24px; }\n"
           << "    main { border-left: 5px solid #e15a36; padding: 8px 0 8px 28px; }\n"
           << "    h1 { font-family: Georgia, serif; font-size: 2.5rem; margin: 0 0 16px; }\n"
           << "    p { line-height: 1.6; max-width: 52ch; }\n"
           << "    button, a { display: inline-block; margin: 12px 12px 0 0; border: 0; border-radius: 4px; padding: 11px 16px; background: #176b5b; color: white; font: inherit; text-decoration: none; cursor: pointer; }\n"
           << "    a { background: #e15a36; }\n"
           << "  </style>\n</head>\n<body>\n  <main>\n" << body.str()
           << "  </main>\n<script>\n  document.addEventListener(\"click\", (event) => {\n"
           << "    const button = event.target.closest(\"button[data-message]\");\n"
           << "    if (button) window.alert(button.dataset.message);\n  });\n</script>\n</body>\n</html>\n";
    return output.str();
}

} // namespace epp_gui

#ifndef EPP_GUI_NO_MAIN
int main(int argc, char** argv) {
    if (argc < 2 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
        std::cout << "Usage: e++-gui <source.epp> [--out <output.html>]\n";
        return argc < 2 ? 1 : 0;
    }
    std::string sourcePath = argv[1];
    std::string outputPath;
    for (int index = 2; index < argc; ++index) {
        if (std::string(argv[index]) == "--out" && index + 1 < argc) outputPath = argv[++index];
        else { std::cerr << "e++-gui: unknown option '" << argv[index] << "'\n"; return 1; }
    }
    try {
        std::ifstream sourceFile(sourcePath);
        if (!sourceFile) throw std::runtime_error("cannot open source file: " + sourcePath);
        std::ostringstream source;
        source << sourceFile.rdbuf();
        if (outputPath.empty()) {
            std::filesystem::path output(sourcePath);
            outputPath = output.replace_extension(".html").string();
        }
        std::ofstream outputFile(outputPath);
        if (!outputFile) throw std::runtime_error("cannot write output file: " + outputPath);
        outputFile << epp_gui::render(epp_gui::parse(source.str()));
        std::cout << "Built " << outputPath << "\n";
    } catch (const std::exception& error) {
        std::cerr << "e++-gui: " << error.what() << "\n";
        return 1;
    }
    return 0;
}
#endif
