#include <cassert>
#include <iostream>
#include <string>

#include "../src/main.cpp"

void testParsesInterfaceStatements() {
    const epp::Document document = epp::parse(
        "title \"Example app\"\nheading \"Welcome\"\ntext \"A short note\"\n"
        "button \"Open\" \"Opened\"\nlink \"Guide\" \"https://example.org\"");

    assert(document.title == "Example app");
    assert(document.elements.size() == 4);
    assert(document.elements[0].kind == "heading");
    assert(document.elements[0].values[0] == "Welcome");
    assert(document.elements[1].values[0] == "A short note");
    assert(document.elements[2].values == std::vector<std::string>({"Open", "Opened"}));
    assert(document.elements[3].values[1] == "https://example.org");
}

void testAcceptsOptionalCommandColons() {
    const epp::Document document = epp::parse("text: \"hello!\"");
    assert(document.elements.size() == 1);
    assert(document.elements[0].kind == "text");
    assert(document.elements[0].values[0] == "hello!");
}

void testRejectsUnknownStatement() {
    try {
        epp::parse("title \"Example\"\nunknown \"value\"");
        assert(false);
    } catch (const std::exception& error) {
        assert(std::string(error.what()).find("line 2:") != std::string::npos);
        assert(std::string(error.what()).find("unknown") != std::string::npos);
    }
}

void testRejectsExecutableLinkSchemes() {
    try {
        epp::parse("link \"Click\" \"javascript:alert(1)\"");
        assert(false);
    } catch (const std::exception& error) {
        assert(std::string(error.what()).find("unsupported link URL scheme") != std::string::npos);
    }
}

void testEscapesUserText() {
    const epp::Document document = epp::parse("heading \"<Hello>\"\ntext \"A & B\"");
    const std::string output = epp::render(document);
    assert(output.find("&lt;Hello&gt;") != std::string::npos);
    assert(output.find("A &amp; B") != std::string::npos);
}

int main() {
    testParsesInterfaceStatements();
    testAcceptsOptionalCommandColons();
    testRejectsUnknownStatement();
    testRejectsExecutableLinkSchemes();
    testEscapesUserText();
    std::cout << "All E++ compiler tests passed\n";
    return 0;
}
