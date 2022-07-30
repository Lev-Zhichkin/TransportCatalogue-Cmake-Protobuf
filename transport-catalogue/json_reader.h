#pragma once

#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace json_reader {

	class JsonReader {
	public:
		JsonReader(transport_catalogue::TransportCatalogue& catalog, map_renderer::MapRenderer& map_renderer);
		void Reader(std::istream& input, std::ostream& output); // DEPRECATED
		void MakeBase(std::istream& input);

	private:
		////////// base_requests //////////
		void CompleteCatalog(json::Array& value);
		void AddStop(const json::Dict& stop);
		void AddBus(const json::Dict& bus);

		////////// render_requests //////////
		void Render(const json::Dict& value);
		void RenderSimpleSettings(const json::Dict& value);
		void RenderArraySettings(const json::Dict& value);
		void RenderVariantSettings(const json::Dict& value);

		////////// router_settings //////////
		transport_router::RouterSettings SetRouterSettings(json::Dict& routing_settings);  ////////////////////////////////////////////////////

	private:
		transport_catalogue::TransportCatalogue& catalog_;
		map_renderer::MapRenderer& map_renderer_;
	};

}