#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace json_reader {

	void Reader(transport_catalogue::TransportCatalogue& catalog, MapRenderer& map_renderer, std::istream& input, std::ostream& output);

	////////// base_requests //////////
	void CompleteCatalog(transport_catalogue::TransportCatalogue& catalog, json::Array& value);
	void AddStop(transport_catalogue::TransportCatalogue& catalog, const json::Dict& stop);
	void AddBus(transport_catalogue::TransportCatalogue& catalog, const json::Dict& bus);

	////////// stat_requests //////////
	void ProcessRequest(transport_catalogue::TransportCatalogue& catalog, MapRenderer& map_renderer, json::Array& value, std::ostream& output);

	////////// render_requests //////////
	void Render(MapRenderer& map_renderer, const json::Dict& value);
	void RenderSimpleSettings(MapRenderer& map_renderer, const json::Dict& value);
	void RenderArraySettings(MapRenderer& map_renderer, const json::Dict& value);
	void RenderVariantSettings(MapRenderer& map_renderer, const json::Dict& value);

}