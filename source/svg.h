#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <string_view>
#include <optional>
#include <variant>

namespace svg {

    struct Point {
        Point() = default;
        Point(double x, double y)
                : x(x)
                , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct Rgb {
        Rgb() = default;
        Rgb(int r, int g, int b)
                : red(r)
                , green(g)
                , blue(b)
        {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(int r, int g, int b, double o)
                : red(r)
                , green(g)
                , blue(b)
                , opacity(o)
        {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы
    inline const Color NoneColor{"none"};

    struct ColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const {
            using namespace std::literals;
            out << "none"sv;
        }
        void operator() (std::string s) const {
            out << s;
        }
        void operator() (Rgb rgb) const {
            using namespace std::literals;
            out << "rgb("sv  << static_cast<uint16_t>(rgb.red) << ","sv
                << static_cast<uint16_t>(rgb.green) << ","sv
                << static_cast<uint16_t>(rgb.blue) << ")"sv;
        }
        void operator() (Rgba rgba) const {
            using namespace std::literals;
            out << "rgba("sv  << static_cast<uint16_t>(rgba.red) << ","sv
                << static_cast<uint16_t>(rgba.green) << ","sv
                << static_cast<uint16_t>(rgba.blue) << ","sv
                << rgba.opacity << ")"sv;
        }
    };

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
    struct RenderContext {
        RenderContext(std::ostream& out)
                : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
                : out(out)
                , indent_step(indent_step)
                , indent(indent) {
        }

        RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
    class Object {
    public:

        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:
	    template<typename T>
	    void Add(T obj) {
            AddPtr(std::make_unique<T>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    // Интерфейс Drawable задаёт объекты, которые можно нарисовать с помощью Graphics
    class Drawable {
    public:
        virtual void Draw(ObjectContainer& oc) const = 0;

        virtual	~Drawable() = default;
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap);
    std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join);

    template <typename Owner>
    class PathProps {
    public:
        //цвет заливки
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        //цвет контура
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        //толщину линии
        Owner& SetStrokeWidth(double width){
            stroke_width_ = width;
            return AsOwner();
        }
        //тип формы конца линии
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        }
        //тип формы соединения линий
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv;
                Color f_c = *fill_color_;
                visit(ColorPrinter{out}, f_c);
                out << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv;
                Color s_c = *stroke_color_;
                visit(ColorPrinter{out}, s_c);
                out << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double > stroke_width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
    class Circle : public Object, public PathProps<Circle> {
    public:

        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

        void RenderObject(const RenderContext& context) const override;
    private:
        Point center_;
        double radius_ = 1.0;
    };

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

        void RenderObject(const RenderContext& context) const override;
    private:
        std::vector<Point> points_;
    };

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
    class Text : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(const std::string& font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(const std::string& font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(const std::string& data);

        void RenderObject(const RenderContext& context) const override;
    private:
        Point position_ = { 0.0, 0.0 };
        Point offset_ = { 0.0, 0.0 };
        uint32_t font_size_ = 1;
        std::optional<std::string> font_family_ ;
        std::optional<std::string> font_weight_ ;
        std::string data_ = "";

    };

    class Document : public ObjectContainer{
    public:
        /*
         Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
         Пример использования:
         Document doc;
         doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
        */

        template <typename Obj>
        void Add(Obj obj) {
            objects_ptr_.emplace_back(std::make_unique<Obj>(std::move(obj)));
        }

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj);

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<svg::Object>> objects_ptr_;
    };

}  // namespace svg