#ifndef CELL_H
#define CELL_H

#include <Kokkos_Core.hpp>
#include "Kokkos_Macros.hpp"
#include "Kokkos_MathematicalFunctions.hpp"
#include "../../util/src/id.h"
#include "../../util/src/field.h"
#include "grid_io.h"
#include "vertex.h"

template <typename T> struct Cells;

template <typename T>
struct CellView {
public:
    CellView(Cells<T> *cells_, int id_) 
        : cells_(cells_), id_(id_) {}

    KOKKOS_FORCEINLINE_FUNCTION
    auto vertex_ids() {return cells_->vertex_ids()[id_];}

private:
    Cells<T> *cells_;
    int id_;
};

template <typename T>
struct Cells {
public:
    Cells () {}

    Cells(Id vertices, Id interfaces, std::vector<ElemType> shapes) 
        : interface_ids_(interfaces), vertex_ids_(vertices) 
    {
        shape_ = Field<ElemType>("Cell::shape", shapes.size());
        for (unsigned int i = 0; i < shapes.size(); i++) {
            shape_(i) = shapes[i]; 
        }

        volume_ = Field<T>("Cell::Volume", shapes.size());
    }

    bool operator == (const Cells &other) const {
        return (interface_ids_ == other.interface_ids_) &&
               (vertex_ids_ == other.vertex_ids_);
    }

    KOKKOS_FORCEINLINE_FUNCTION
    CellView<T> operator[] (const int i) {
        assert(i < size());
        return CellView<T>(this, i);
    }

    KOKKOS_FORCEINLINE_FUNCTION
    Id &vertex_ids() {return vertex_ids_;}

    KOKKOS_FORCEINLINE_FUNCTION
    Id &interface_ids() {return interface_ids_;}

    KOKKOS_FORCEINLINE_FUNCTION
    int size() const {return interface_ids_.size();}

    T & volume(const int i) const {return volume_(i);}

    void compute_volumes(Vertices<T> vertices) {
        // TODO: It would be nicer to move each case in the switch 
        // to a function sitting somewhere else to keep the amount
        // of code in this method down, and avoid duplication with
        // computing the area of interfaces. However, this won't
        // be trivial for the GPU.
        Kokkos::parallel_for("Cells::compute_volume", volume_.size(), KOKKOS_LAMBDA(const int i) {
            switch (shape_(i)) {
                case ElemType::Line:
                    throw std::runtime_error("Invalid cell shape");
                    break;
                case ElemType::Tri: {
                    auto vertex_ids = vertex_ids_[i];
                    T x1 = vertices.position(vertex_ids(0), 0);
                    T x2 = vertices.position(vertex_ids(1), 0);
                    T x3 = vertices.position(vertex_ids(2), 0);
                    T y1 = vertices.position(vertex_ids(0), 1);
                    T y2 = vertices.position(vertex_ids(1), 1);
                    T y3 = vertices.position(vertex_ids(2), 1);
                    T area = x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2);
                    volume_(i) = 0.5 * Kokkos::fabs(area);
                    break;
                }
                case ElemType::Quad: {
                    auto vertex_ids = vertex_ids_[i];
                    T x1 = vertices.position(vertex_ids(0), 0);
                    T x2 = vertices.position(vertex_ids(1), 0);
                    T x3 = vertices.position(vertex_ids(2), 0);
                    T x4 = vertices.position(vertex_ids(3), 0);
                    T y1 = vertices.position(vertex_ids(0), 1);
                    T y2 = vertices.position(vertex_ids(1), 1);
                    T y3 = vertices.position(vertex_ids(2), 1);
                    T y4 = vertices.position(vertex_ids(3), 1);
                    T area = x1*y2 + x2*y3 + x3*y4 + x4*y1 - 
                                 x2*y1 - x3*y2 - x4*y3 - x1*y4;
                    volume_(i) = 0.5 * Kokkos::fabs(area);
                    break;
                }
                case ElemType::Hex:
                    throw std::runtime_error("Not implemented");
                    break;
                case ElemType::Wedge:
                    throw std::runtime_error("Not implemented");
                    break;
                case ElemType::Pyramid:
                    throw std::runtime_error("Not implemented");
                    break;
            }
        }); 
    }

private:
    Id interface_ids_;
    Id vertex_ids_;
    Field<ElemType> shape_;
    Field<T> volume_;
    Field<int> outsign_;
};

#endif
