#include <cassert>
#include <iostream>
#include <string>

#include "../src/main.cpp"

void testRuntimeVariablesAndExpressions() {
    epp::Runtime runtime;
    std::ostringstream output;
    runtime.run("let name = \"E++\"\nlet count = 2 + 3 * 4\nprint name\nprint count", output);
    assert(output.str() == "E++\n14\n");
}

void testRuntimeControlFlow() {
    epp::Runtime runtime;
    std::ostringstream output;
    runtime.run("let count = 0\nrepeat 3\nlet count = count + 1\nprint count\nendrepeat\nif count == 3\nprint \"done\"\nelse\nprint \"wrong\"\nendif", output);
    assert(output.str() == "1\n2\n3\ndone\n");
}

int main() {
    testRuntimeVariablesAndExpressions();
    testRuntimeControlFlow();
    std::cout << "All E++ language tests passed\n";
    return 0;
}
