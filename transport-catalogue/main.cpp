#include "json_reader.h"
#include "map_renderer.h"

#include <sstream>
#include <fstream>
#include <string>
#include <iostream>

int main() {
	transport_catalogue::TransportCatalogue TC;
	MapRenderer MR;
	json_reader::Reader(TC, MR, std::cin, std::cout);
}