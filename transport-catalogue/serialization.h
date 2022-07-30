#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_router.h"
#include "graph.h"
#include "request_handler.h"
#include "transport_catalogue.pb.h"

#include <fstream>
#include <vector>

namespace serialize
{
	class Serializer
	{
	public:
		Serializer(const transport_catalogue::TransportCatalogue& trans_cat, const map_renderer::MapRenderer& map_renderer, const transport_router::TransportRouter& trans_rote)
			: trans_cat_(trans_cat), map_renderer_(map_renderer), trans_rote_(trans_rote) {}

		void Serialize(const std::string& filename);

	private:
		const transport_catalogue::TransportCatalogue& trans_cat_;
		const map_renderer::MapRenderer& map_renderer_;
		const transport_router::TransportRouter& trans_rote_;
		transport_catalogue_serialize::TransportCatalogue trans_cat_ser_;

		void SerializeStop(transport_catalogue_serialize::TransportCatalogue& trans_cat_ser);
		void SerializeBus(transport_catalogue_serialize::TransportCatalogue& trans_cat_ser);
		void SerializeDistance(transport_catalogue_serialize::TransportCatalogue& trans_cat_ser);
		void SerealizeRenderSettings(transport_catalogue_serialize::TransportCatalogue& trans_cat_ser);
		transport_catalogue_serialize::Color SerealizeColor(const svg::Color& color);
		void SerealizeRoutingSettings(transport_catalogue_serialize::TransportCatalogue& trans_cat_ser);
	};

	class Deserializer
	{
	public:
		void DeserializeToRequestHandler(std::istream& input, std::ostream& output);

	private:
		void DeserializeCatalogAndSetMapSettings(transport_catalogue::TransportCatalogue& trans_cat, map_renderer::MapRenderer& map_renderer, const std::string& filename);
		transport_router::RouterSettings DeserealizeRouterSettings(const std::string& filename);
		std::pair<std::pair<const transport_catalogue::Stop*, const transport_catalogue::Stop*>, int> DeserializeDistance(const transport_catalogue_serialize::Distance& distance_ser, const transport_catalogue::TransportCatalogue& trans_cat);
		map_renderer::MapRenderer::Settings DeserealizeRenderSettings(const transport_catalogue_serialize::RenderSettings& render_settings_ser);
		svg::Color DserealizeColor(const transport_catalogue_serialize::Color& color_ser);
	};

} // namespace serialize