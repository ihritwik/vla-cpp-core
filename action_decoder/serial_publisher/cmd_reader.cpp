// cmd_reader — reads and pretty-prints /tmp/robot_cmd.json to stdout.
//
// Usage: ./cmd_reader [path]
//        Defaults to /tmp/robot_cmd.json if no path given.
//
// Compile: see action_decoder/CMakeLists.txt

#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string path = "/tmp/robot_cmd.json";
    if (argc >= 2) {
        path = argv[1];
    }

    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "cmd_reader: cannot open " << path << "\n";
        return 1;
    }

    std::cout << "=== " << path << " ===\n";
    std::string line;
    while (std::getline(ifs, line)) {
        std::cout << line << "\n";
    }
    return 0;
}
