#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <utility>
#include <map>


#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "json_builder.h"
#include "transport_router.h"
#include "serialization.h"

#include "domain.h"

namespace request_handler {

	class RequestHandler {
	public:
		RequestHandler(transport_catalogue::TransportCatalogue& catalog, map_renderer::MapRenderer& map_renderer, transport_router::RouterSettings router_settings);
		void ProcessRequest(json::Array& value, std::ostream& output);

		const transport_router::TransportRouter& GetTransportRouter();

	private:
		void PrintStopInformation(const json::Dict& value, std::ostream& output);
		void PrintBusInformation(const json::Dict& value, std::ostream& output);
		void PrintMap(const json::Dict& value, std::ostream& output);
		void PrintRoute(const json::Dict& value, std::ostream& output);

	private:
		transport_catalogue::TransportCatalogue& catalog_;
		map_renderer::MapRenderer& map_renderer_;
		transport_router::TransportRouter transport_router_;
	};

}