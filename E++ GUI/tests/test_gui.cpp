#include <cassert>
#include <string>

#include "../src/main.cpp"

int main() {
    const epp_gui::Document document = epp_gui::parse(
        "title: \"Example\"\nheading \"Hello\"\ntext \"A & B\"\nbutton \"Open\" \"Opened\"");
    assert(document.title == "Example");
    assert(document.elements.size() == 3);
    const std::string html = epp_gui::render(document);
    assert(html.find("<title>Example</title>") != std::string::npos);
    assert(html.find("A &amp; B") != std::string::npos);
    assert(html.find("data-message=\"Opened\"") != std::string::npos);

    try {
        epp_gui::parse("link \"Bad\" \"javascript:alert(1)\"");
        assert(false);
    } catch (const std::exception& error) {
        assert(std::string(error.what()).find("unsupported link URL scheme") != std::string::npos);
    }
}
