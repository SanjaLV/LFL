#include <filesystem>
#include <iostream>
#include <string>

#include "Database/Models.h"
#include "XMLParser/Parser.h"

void process_single_xml_file(const std::string &name)
{
    double start_time = clock();

    auto game = LFL::XMLParser::parse_game_file(name);
    LFL::Database::process_game_info(game);

    double time = (clock() - start_time) / CLOCKS_PER_SEC * 1000;  // in ms
    std::cout << "Processed '" << name << "' in " << time << " ms" << std::endl;
}

static void help()
{
    std::cout << "Usage: lfl [options] file\n";
    std::cout << "Options:\n";

    std::vector<std::pair<std::string, std::string>> options = {
        std::make_pair("--help", "Display this information."),
        std::make_pair("--single <file>", "Process single XML file."),
        std::make_pair("--dir <directory>",
                       "Process all XML files in given directory."),
        std::make_pair("--generate <file>", "Generate statistics to give file."),
        std::make_pair("--max-player <N>", "Truncate generated tables after N-th player."),
    };

    for (auto &o : options) {
        while (o.first.size() < 16)
            o.first += ' ';
        std::cout << "\t" << o.first << o.second << "\n";
    }
}

int main(int argc, char *argv[])
{
    if (argc <= 1 or argc > 3) {
        help();
        return 1;
    }

    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    int truncate_after = 0;

    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == "--help") {
            help();
            return 0;
        }

        // All other commands expect next token
        if (i + 1 >= args.size()) {
            std::cout << "One more token expected!" << std::endl;
            std::cout << "Run --help to list all options!" << std::endl;
            return 1;
        }

        const std::string &next_token = args[i + 1];

        if (args[i] == "--single") {
            process_single_xml_file(next_token);
        }
        else if (args[i] == "--dir") {
            const std::filesystem::path dir{next_token};

            for (auto const &dir_entry :
                 std::filesystem::directory_iterator{dir}) {
                if (dir_entry.path().extension() == ".xml") {
                    process_single_xml_file(dir_entry.path().string());
                }
                else {
                    std::cout << "Skipping non XML file '" << dir_entry << "'."
                              << std::endl;
                }
            }  // dir iterator
        }
        else if (args[i] == "--generate") {
            LFL::Database::generate_html_output(next_token);
        }
        else if (args[i] == "--max-player") {
            truncate_after = std::stoi(next_token);
        }
        else {
            std::cout << "Unknown option '" << args[i] << "';" << std::endl;
            std::cout << "Run --help to list all options!" << std::endl;
            return 1;
        }

        i++;  // all comands expect one more tooken
    }

    return 0;
}
