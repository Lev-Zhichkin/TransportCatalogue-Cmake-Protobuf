#include "json_reader.h"
#include "map_renderer.h"

#include <sstream>
#include <fstream>
#include <string>
#include <iostream>

int main() {
	transport_catalogue::TransportCatalogue TC;
	map_renderer::MapRenderer MR;
	json_reader::JsonReader JR(TC, MR);
	JR.Reader(std::cin, std::cout);
}