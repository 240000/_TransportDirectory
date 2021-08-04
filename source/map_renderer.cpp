#include "map_renderer.h"

namespace map_renderer {
	
	void MapRenderer::FillMapRenderer(const std::map<std::string_view, transport_catalogue::domain::Bus*>& buses, const std::map<std::string_view, transport_catalogue::domain::Stop*>& stops)
	{
		SetMinMaxCoordinates(stops);
		SetZoomCoef();
		
		coords_map stop_coords;
		FillStopCoords(stops, stop_coords);
		
		AddRouteLinesLayer(buses, stop_coords);
		AddRouteNamesLayer(buses, stop_coords);
		AddStopCirclesLayer(stops, stop_coords);
		AddStopNamesLayer(stops, stop_coords);
	}
	
	void MapRenderer::FillMapRenderer(RenderSettings&& rs, const std::map<std::string_view, transport_catalogue::domain::Bus*>& buses, const std::map<std::string_view, transport_catalogue::domain::Stop*>& stops)
	{
		render_settings_ = std::move(rs);
		FillMapRenderer(buses, stops);
	}
	
	void MapRenderer::FillStopCoords(const std::map<std::string_view, transport_catalogue::domain::Stop*>& stops, coords_map & stop_coords) {
		for(auto& [_, stop] : stops) {
			if(!stop->buses.empty()) {
				stop_coords[stop->name] = SetCoordinates(stop);
			}
		}
	}
	
	void MapRenderer::ColorPaletteCounter() {
		if (color_palette_it_ == render_settings_.color_palette.size() - 1) {
			color_palette_it_ = 0;
		}
		else {
			++color_palette_it_;
		}
	}
	
	void MapRenderer::AddRouteLinesLayer(const std::map<std::string_view, transport_catalogue::domain::Bus*>& buses, coords_map & stop_coords) {
		color_palette_it_ = 0;
		for (auto &[_, bus_ptr] : buses) {
			if (!bus_ptr->route.empty()) {
				svg::Polyline route;
				for (auto & it : bus_ptr->route) {
					route.AddPoint(stop_coords.at(it->name));
				}
				if (!bus_ptr->is_roundtrip) {
					for (auto it = next(bus_ptr->route.rbegin()); it != bus_ptr->route.rend(); ++it) {
						route.AddPoint(stop_coords.at((*it)->name));
					}
				}
				route.SetFillColor("none"s)
						.SetStrokeColor(render_settings_.color_palette[color_palette_it_])
						.SetStrokeWidth(render_settings_.line_width)
						.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
						.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				rend_routes_.Add(std::move(route));
				ColorPaletteCounter();
			}
		}
	}
		
	void MapRenderer::AddRouteNamesLayer(const std::map<std::string_view, transport_catalogue::domain::Bus*>& buses, coords_map & stop_coords) {
		color_palette_it_ = 0;
		for (auto &[_, bus_ptr] : buses) {
			if (!bus_ptr->route.empty()) {
				AddRouteName(stop_coords.at((*bus_ptr->route.begin())->name), bus_ptr);
				if (!bus_ptr->is_roundtrip) {
					if(bus_ptr->route.front()->name != bus_ptr->route.back()->name) {
						AddRouteName(stop_coords.at((*bus_ptr->route.rbegin())->name), bus_ptr);
					}
				}
				ColorPaletteCounter();
			}
		}
	}
	
	void MapRenderer::AddStopCirclesLayer(const std::map<std::string_view, transport_catalogue::domain::Stop*>& stops, coords_map & stop_coords) {
		for (const auto &[_, stop] : stops) {
			if (stop_coords.count(stop->name) != 0) {
				svg::Circle c;
				c.SetCenter(stop_coords.at(stop->name)).SetRadius(render_settings_.stop_radius).SetFillColor("white"s);
				rend_routes_.Add(std::move(c));
			}
		}
	}
	
	void MapRenderer::AddStopNamesLayer(const std::map<std::string_view, transport_catalogue::domain::Stop*>& stops, coords_map & stop_coords) {
		for (const auto &[_, stop] : stops) {
			if (stop_coords.count(stop->name) != 0) {
				svg::Text stop_name_background;
				svg::Text stop_name;
				stop_name_background.SetPosition({ stop_coords.at(stop->name).x, stop_coords.at(stop->name).y })
						.SetOffset({ render_settings_.stop_label_offset_dx, render_settings_.stop_label_offset_dy })
						.SetFontSize(render_settings_.stop_label_font_size)
						.SetFontFamily("Verdana"s)
						.SetData(stop->name)
						.SetFillColor(render_settings_.underlayer_color)
						.SetStrokeColor(render_settings_.underlayer_color)
						.SetStrokeWidth(render_settings_.underlayer_width)
						.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
						.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				stop_name.SetPosition({ stop_coords.at(stop->name).x, stop_coords.at(stop->name).y })
						.SetOffset({ render_settings_.stop_label_offset_dx, render_settings_.stop_label_offset_dy })
						.SetFontSize(render_settings_.stop_label_font_size)
						.SetFontFamily("Verdana"s)
						.SetData(stop->name)
						.SetFillColor("black"s);
				rend_routes_.Add(std::move(stop_name_background));
				rend_routes_.Add(std::move(stop_name));
			}
		}
	}
		
	svg::Point MapRenderer::SetCoordinates(const transport_catalogue::domain::Stop* stop) {
		double x = (stop->coordinates.lng - min_lng_) * zoom_coef_ + render_settings_.padding;
		double y = (max_lat_ - stop->coordinates.lat) * zoom_coef_ + render_settings_.padding;
		return { x, y };
	}
	
	void MapRenderer::AddRouteName(const svg::Point& position, const transport_catalogue::domain::Bus* bus) {
		svg::Text route_name_background;
		svg::Text route_name;
		route_name_background.SetPosition({ position.x, position.y })
				.SetOffset({ render_settings_.bus_label_offset_dx, render_settings_.bus_label_offset_dy })
				.SetFontSize(render_settings_.bus_label_font_size)
				.SetFontFamily("Verdana"s)
				.SetFontWeight("bold"s)
				.SetData(bus->name)
				.SetFillColor(render_settings_.underlayer_color)
				.SetStrokeColor(render_settings_.underlayer_color)
				.SetStrokeWidth(render_settings_.underlayer_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		rend_routes_.Add(std::move(route_name_background));
		route_name.SetPosition({ position.x, position.y })
				.SetOffset({ render_settings_.bus_label_offset_dx, render_settings_.bus_label_offset_dy })
				.SetFontSize(render_settings_.bus_label_font_size)
				.SetFontFamily("Verdana"s)
				.SetFontWeight("bold"s)
				.SetData(bus->name)
				.SetFillColor(render_settings_.color_palette[color_palette_it_]);
		rend_routes_.Add(std::move(route_name));
	}
	
	void MapRenderer::SetMinMaxCoordinates(const std::map<std::string_view, transport_catalogue::domain::Stop*>& stops) {
		for(auto& stop : stops) {
			if(!stop.second->buses.empty()) {
				if (min_lat_ == 0.0 || min_lat_ > stop.second->coordinates.lat) {
					min_lat_ = stop.second->coordinates.lat;
				}
				if (min_lng_ == 0.0 || min_lng_ > stop.second->coordinates.lng) {
					min_lng_ = stop.second->coordinates.lng;
				}
				if (max_lat_ == 0.0 || max_lat_ < stop.second->coordinates.lat) {
					max_lat_ = stop.second->coordinates.lat;
				}
				if (max_lng_ == 0.0 || max_lng_ < stop.second->coordinates.lng) {
					max_lng_ = stop.second->coordinates.lng;
				}
			}
		}
	}
	
	void MapRenderer::SetZoomCoef() {
		double width_zoom_coef = (render_settings_.width - 2 * render_settings_.padding) / (max_lng_ - min_lng_);
		double height_zoom_coef = (render_settings_.height - 2 * render_settings_.padding) / (max_lat_ - min_lat_);
		if(width_zoom_coef != 0.0 && height_zoom_coef != 0.0){
			zoom_coef_ = std::min(width_zoom_coef, height_zoom_coef);
		}
		else if(width_zoom_coef == 0.0) {
			zoom_coef_ = height_zoom_coef;
		}
		else if(height_zoom_coef == 0.0){
			zoom_coef_ = width_zoom_coef;
		}
	}
	
	void MapRenderer::RenderAsSVG(std::ostream& out) {
		if(cache.svg_empty) {
			rend_routes_.Render(cache.svg_response);
			cache.svg_empty = false;
		}
		out << cache.svg_response.str();
	}
	
	void MapRenderer::RenderAsJSON(std::ostream& out) {
		if(cache.json_empty) {
			std::stringstream ss;
			rend_routes_.Render(ss);
			cache.json_response.put('"');
			rend_routes_.Render(cache.json_response);
			StringToJSON(ss.str(), cache.json_response);
			cache.json_response.put('"');
			cache.json_empty = false;
		}
		out << cache.json_response.str();
	}

	void MapRenderer::RenderAsString() {
		if(cache.string_empty) {;
			cache.json_response.put('"');
			rend_routes_.Render(cache.string_response);
			cache.json_response.put('"');
			cache.json_empty = false;
		}
	}
	
	void MapRenderer::SetRenderSettings(RenderSettings rs) {
		render_settings_ = std::move(rs);
	}
	
	bool MapRenderer::IsRenderSettingsEmpty() {
		return render_settings_.is_empty;
	}
	
	void StringToJSON(const std::string& s, std::ostream& out) {
		for (const char c : s) {
			switch (c) {
				case '\r':
					out << "\\r";
					break;
				case '\n':
					out << "\\n";// << std::endl;
					break;
				case '"':
					// Символы " и \ выводятся как \" или \\, соответственно
					[[fallthrough]];
				case '\\':
					out.put('\\');
					[[fallthrough]];
				default:
					out.put(c);
					break;
			}
		}
	}
		
} // end namespace map_renderer

