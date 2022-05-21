#pragma once

#include "json.h"
#include "reguest_handler.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace json_reader {

	class JsonReader {
	public:
		JsonReader(transport_catalogue::TransportCatalogue& catalog, map_renderer::MapRenderer& map_renderer);
		void Reader(std::istream& input, std::ostream& output);

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

	private:
		transport_catalogue::TransportCatalogue& catalog_;
		map_renderer::MapRenderer& map_renderer_;
	};

}