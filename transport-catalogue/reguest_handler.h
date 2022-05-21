#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include<stdio.h>
#include <utility>
#include <map>


#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "json_builder.h"

#include "domain.h"

namespace reguest_handler {

	void PrintStopInformation(transport_catalogue::TransportCatalogue& catalog, const json::Dict& value, std::ostream& output);
	void PrintBusInformation(transport_catalogue::TransportCatalogue& catalog, const json::Dict& value, std::ostream& output);
	void PrintMap(transport_catalogue::TransportCatalogue& transport_catalogue, MapRenderer& map_renderer, const json::Dict& value, std::ostream& output);

}