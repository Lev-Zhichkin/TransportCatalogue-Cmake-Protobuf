#include "json_reader.h"
#include "json_builder.h"

#include <iostream>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        transport_catalogue::TransportCatalogue catalog;
        map_renderer::MapRenderer map_renderer;
        json_reader::JsonReader reader(catalog, map_renderer);
        reader.MakeBase(std::cin);
    }
    else if (mode == "process_requests"sv) {
        serialize::Deserializer request_deserializer;
        request_deserializer.DeserializeToRequestHandler(std::cin, std::cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}