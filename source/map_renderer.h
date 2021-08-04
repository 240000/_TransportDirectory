#pragma once

#include "svg.h"
#include "geo.h"
#include "domain.h"

#include <map>
#include <string_view>
#include <vector>
#include <sstream>
#include <string_view>
#include <algorithm>

using namespace std::literals;

namespace map_renderer {
	
	struct MapRendererResponse {
		bool svg_empty = true;
		bool json_empty = true;
		bool string_empty = true;
		std::stringstream json_response;
		std::stringstream svg_response;
		std::stringstream string_response;
	};

	struct RenderSettings {
		double width = 0.0;
		double height = 0.0;
		double padding = 0.0;
		
		double line_width = 0.0;
		double stop_radius = 0.0;
		
		int bus_label_font_size = 0;
		double bus_label_offset_dx = 0.0;
		double bus_label_offset_dy = 0.0;
		
		int stop_label_font_size = 0;
		double stop_label_offset_dx = 0.0;
		double stop_label_offset_dy = 0.0;
		
		svg::Color underlayer_color;
		double underlayer_width = 0.0;
		
		std::vector<svg::Color> color_palette;
		
		bool is_empty = true;
	};
	
	using coords_map = std::unordered_map<std::string_view, svg::Point, std::hash<std::string_view>>;
	
	class MapRenderer {
	public:
		void FillMapRenderer(const std::map<std::string_view, transport_catalogue::domain::Bus*>& buses,
								const std::map<std::string_view, transport_catalogue::domain::Stop*>& stops);
		
		void FillMapRenderer(RenderSettings&& rs, const std::map<std::string_view, transport_catalogue::domain::Bus*>& buses,
		                                              const std::map<std::string_view, transport_catalogue::domain::Stop*>& stops);
		
		
		void ColorPaletteCounter();
		
		void FillStopCoords(const std::map<std::string_view, transport_catalogue::domain::Stop*>& stops, coords_map& stop_coords);
		
		void AddRouteLinesLayer(const std::map<std::string_view, transport_catalogue::domain::Bus*>& buses, coords_map& stop_coords);
		
		void AddRouteNamesLayer(const std::map<std::string_view, transport_catalogue::domain::Bus*>& buses, coords_map& stop_coords);
		
		void AddStopCirclesLayer(const std::map<std::string_view, transport_catalogue::domain::Stop*>& stops, coords_map& stop_coords);
		
		void AddStopNamesLayer(const std::map<std::string_view, transport_catalogue::domain::Stop*>& stops, coords_map& stop_coords);
		
		svg::Point SetCoordinates(const transport_catalogue::domain::Stop* stop);
		
		void AddRouteName(const svg::Point& position, const transport_catalogue::domain::Bus* bus);
		
		void SetMinMaxCoordinates(const std::map<std::string_view, transport_catalogue::domain::Stop*>& stops);
		
		void SetZoomCoef();
		
		void RenderAsSVG(std::ostream& out);
		
		void RenderAsJSON(std::ostream& out);
		
		void RenderAsString();
		
		void SetRenderSettings(map_renderer::RenderSettings rs);
		
		bool IsRenderSettingsEmpty();
		
		MapRendererResponse cache;
		
	private:
		RenderSettings render_settings_;
		svg::Document rend_routes_;
		double zoom_coef_ = 0.0;
		double min_lat_ = 0.0;
		double min_lng_ = 0.0;
		double max_lat_ = 0.0;
		double max_lng_ = 0.0;
		uint color_palette_it_ = 0;
	};
	
	void StringToJSON(const std::string& s, std::ostream& out);


} // end namespace map_renderer


