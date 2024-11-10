#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <libnest2d/libnest2d.hpp>

#include "../tools/printer_parts.hpp"
#include "../tools/svgtools.hpp"


namespace py = pybind11;

using Point = libnest2d::Point;
using Box = libnest2d::Box;
using Circle = libnest2d::Circle;
using PolygonImpl = libnest2d::PolygonImpl;
using Item = libnest2d::Item;
using PackGroup = libnest2d::PackGroup;
using SVGWriter = libnest2d::svg::SVGWriter<libnest2d::PolygonImpl>;
using Coord = libnest2d::Coord;

template<class Bin>
std::tuple<size_t, py::object> nestImpl(std::vector<Item>& input, const Bin& bin, Coord dist, const py::kwargs& kwargs) {
    using NfpPlacer = libnest2d::_NfpPlacer<Bin>;
    using FirstFitSelection = libnest2d::FirstFitSelection;
    using Radians = libnest2d::Radians;
    using Config = typename NfpPlacer::Config;
    using NestConfig = libnest2d::NestConfig<NfpPlacer, FirstFitSelection>;
    
    Config cfg;

    if(kwargs) {
        for(auto item : kwargs) {
            if(std::string(py::str(item.first)) == "rotations") {
                std::vector<double_t> temp = item.second.cast<std::vector<double_t>>();
                std::vector<Radians> rotations(temp.begin(), temp.end());

                cfg.rotations = rotations;
            }
        }
    }

    NestConfig ncfg(cfg);
    size_t bins = libnest2d::nest<NfpPlacer, FirstFitSelection>(input, bin, dist, ncfg);

    py::object obj = py::cast(input);
    return std::make_tuple(bins, obj);
}

PYBIND11_MODULE(nest2D, m)
{
    m.doc() = "2D irregular bin packaging and nesting for python";

    py::class_<Point>(m, "Point", "2D Point")
        .def(py::init<int, int>(),  py::arg("x"), py::arg("y"))
        //.def_property_readonly("x", &Point::X)
        .def_property_readonly("x", [](const Point &p) { return p.X; })
        .def_property_readonly("y", [](const Point &p) { return p.Y; })
        .def("__repr__",
             [](const Point &p) {
                 std::string r("Point(");
                 r += boost::lexical_cast<std::string>(p.X);
                 r += ", ";
                 r += boost::lexical_cast<std::string>(p.Y);
                 r += ")";
                 return r;
             }
        )
        .def("__eq__",
            [](const Point &p, const Point & q) {
                return p == q;
            }
        );

    // see lib/libnest2d/include/libnest2d/geometry_traits.hpp
    py::class_<Circle>(m, "Circle", "2D Circle point pair")
        //.def(py::init<int, int>())
        // custom constructor to define circle center
        .def(py::init([](Point &center, double r) {
            return std::unique_ptr<Circle>(new Circle(center, r));
        }))
        ;
    
    py::class_<Box>(m, "Box", "2D Box point pair")
        //.def(py::init<int, int>())
        // custom constructor to define box center
        .def(py::init([](int x, int y) {
            return std::unique_ptr<Box>(new Box(x, y, {x/2, y/2}));
        }))
        ;
    
    py::class_<PolygonImpl>(m, "PolygonImpl", "2D Polygon")
        .def_readwrite("Contour", &PolygonImpl::Contour)
        .def_readwrite("Holes", &PolygonImpl::Holes);
    
    // Item is a shape defined by points
    // see lib/libnest2d/include/libnest2d/nester.hpp
    py::class_<Item>(m, "Item", "An item to be placed on a bin.")
        .def(py::init<std::vector<Point>>())
        .def("binId", [](const Item &i) { return i.binId(); })
        .def("area", [](const Item &i) { return i.area(); })
        .def("vertexCount", [](const Item &i) { return i.vertexCount(); })
        .def("inflation", [](const Item &i) { return i.inflation(); })
        .def("translation", [](const Item &i) { return i.translation(); })
        .def("rotation", [](const Item &i) { return double(i.rotation()); })
        .def("transformedShape", [](const Item &i) { return i.transformedShape(); })
        .def("__repr__",
             [](const Item &i) {
                 std::string r("Item(area: ");
                 r += boost::lexical_cast<std::string>(i.area());
                 r += ", bin_id: ";
                 r += boost::lexical_cast<std::string>(i.binId());
                 r += ", vertices: ";
                 r += boost::lexical_cast<std::string>(i.vertexCount());
                 r += ", inflation: ";
                 r += boost::lexical_cast<std::string>(i.inflation());
                 r += ", translation: ";
                 r += boost::lexical_cast<std::string>(i.translation());
                 r += ", rotation: ";
                 r += boost::lexical_cast<std::string>(double(i.rotation()));
                 r += ")";
                 return r;
             }
        );

    // see lib/libnest2d/include/libnest2d/libnest2d.hpp
    m.def("nest", &nestImpl<Box>,
        py::arg("input"),
        py::arg("box"),
        py::arg("dist") = Coord(0),
        "Nest and pack the input items into the box bin. \n\n"
        "kwargs: key-value config pair for NestConfig. Valid key-value pairs are \n"
        "   'rotations': List[double]. Item rotation constrain in radian metric. \n"
        )
        ;
    
    m.def("nest", &nestImpl<Circle>,
        py::arg("input"),
        py::arg("circle"),
        py::arg("dist") = Coord(0),
        "Nest and pack the input items into the circle bin. \n\n"
        "kwargs: key-value config pair for NestConfig. Valid key-value pairs are \n"
        "   'rotations': List[double]. Item rotation constrain in radian metric. \n"
        )
        ;
    
    py::class_<SVGWriter>(m, "SVGWriter", "SVGWriter tools to write pack_group to SVG.")
        .def(py::init([]() {
            // custom constructor
            SVGWriter::Config conf;
            conf.mm_in_coord_units = libnest2d::mm();
            return std::unique_ptr<SVGWriter>(new SVGWriter(conf));
        }))
        .def("write_packgroup", [](SVGWriter & sw, std::vector<Item>& input) {
            size_t bins = 0;

            for (Item &itm : input) {
                if (itm.binId() >= bins) bins = itm.binId() + 1;
            }

            PackGroup pgrp(bins);

            for (Item &itm : input) {
                if (itm.binId() >= 0) pgrp[size_t(itm.binId())].emplace_back(itm);
                //py::print("bin_id: ", itm.binId());
                //py::print("vertices: ", itm.vertexCount());
            }

            sw.setSize(Box(libnest2d::mm(250), libnest2d::mm(210)));  // TODO make own call
            sw.writePackGroup(pgrp);
        })
        .def("save", [](SVGWriter & sw) {
            sw.save("out");
        })
        .def("__repr__",
             [](const SVGWriter &sw) {
                 std::string r("SVGWriter(");
                 r += ")";
                 return r;
             }
        );

}

