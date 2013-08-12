#include <cxxtest/TestSuite.h>
#include <libgeodecomp/misc/soagrid.h>
#include <libgeodecomp/misc/stencils.h>

using namespace LibGeoDecomp;

class SoATestCell
{
public:
    typedef Stencils::Moore<3, 2> Stencil;

    SoATestCell(int v = 0) :
        v(v)
    {}

    bool operator==(const SoATestCell& other)
    {
        return v == other.v;
    }

    int v;
};

LIBFLATARRAY_REGISTER_SOA(SoATestCell, ((int)(v)))

namespace LibGeoDecomp {


class SoAGridTest : public CxxTest::TestSuite
{
public:

    void testBasic()
    {
        CoordBox<3> box(Coord<3>(10, 15, 22), Coord<3>(50, 40, 35));
        SoATestCell defaultCell(1);
        SoATestCell edgeCell(2);

        SoAGrid<SoATestCell, Topologies::Cube<3>::Topology> grid(box, defaultCell, edgeCell);
        grid.set(Coord<3>(1, 1, 1) + box.origin, 3);
        grid.set(Coord<3>(2, 2, 3) + box.origin, 4);

        TS_ASSERT_EQUALS(grid.actualDimensions, Coord<3>(54, 44, 39));
        TS_ASSERT_EQUALS(grid.boundingBox(), box);
        TS_ASSERT_EQUALS(grid.get(Coord<3>(0, 0, 0)), edgeCell);
        TS_ASSERT_EQUALS(grid.get(Coord<3>(0, 0, 0) + box.origin), defaultCell);
        TS_ASSERT_EQUALS(grid.get(Coord<3>(1, 1, 1) + box.origin), 3);
        TS_ASSERT_EQUALS(grid.get(Coord<3>(2, 2, 3) + box.origin), 4);

        edgeCell = -1;
        grid.setEdge(edgeCell);
        TS_ASSERT_EQUALS(grid.get(Coord<3>(0, 0, 0)), edgeCell);
        TS_ASSERT_EQUALS(grid.get(Coord<3>(0, 0, 0) + box.origin), defaultCell);
        TS_ASSERT_EQUALS(grid.get(Coord<3>(1, 1, 1) + box.origin), 3);
        TS_ASSERT_EQUALS(grid.get(Coord<3>(2, 2, 3) + box.origin), 4);
    }

    void test2d()
    {
        CoordBox<2> box(Coord<2>(10, 15), Coord<2>(50, 40));
        SoATestCell defaultCell(1);
        SoATestCell edgeCell(2);

        SoAGrid<SoATestCell, Topologies::Cube<2>::Topology> grid(box, defaultCell, edgeCell);

        grid.set(Coord<2>(1, 1) + box.origin, 3);
        grid.set(Coord<2>(2, 2) + box.origin, 4);

        TS_ASSERT_EQUALS(grid.actualDimensions, Coord<3>(54, 44, 1));
        TS_ASSERT_EQUALS(grid.boundingBox(), box);
        TS_ASSERT_EQUALS(grid.get(Coord<2>(0, 0)), edgeCell);
        TS_ASSERT_EQUALS(grid.get(Coord<2>(0, 0) + box.origin).v, defaultCell.v);
        TS_ASSERT_EQUALS(grid.get(Coord<2>(1, 1) + box.origin), 3);
        TS_ASSERT_EQUALS(grid.get(Coord<2>(2, 2) + box.origin), 4);
        TS_ASSERT_EQUALS(grid.get(Coord<2>(3, 3) + box.origin), 1);

        edgeCell = -1;
        grid.setEdge(edgeCell);
        TS_ASSERT_EQUALS(grid.get(Coord<2>(0, 0)), edgeCell);
        TS_ASSERT_EQUALS(grid.get(Coord<2>(0, 0) + box.origin), defaultCell);
        TS_ASSERT_EQUALS(grid.get(Coord<2>(1, 1) + box.origin), 3);
        TS_ASSERT_EQUALS(grid.get(Coord<2>(2, 2) + box.origin), 4);
    }

    void testDisplacementWithTopologicalCorrectness()
    {
        CoordBox<3> box(Coord<3>(20, 25, 32), Coord<3>(50, 40, 35));
        Coord<3> topoDim(60, 50, 50);
        SoATestCell defaultCell(1);
        SoATestCell edgeCell(2);

        SoAGrid<SoATestCell, Topologies::Torus<3>::Topology, true> grid(box, defaultCell, edgeCell, topoDim);
        for (CoordBox<3>::Iterator i = box.begin(); i != box.end(); ++i) {
            TS_ASSERT_EQUALS(grid.get(*i), defaultCell);
        }

        // here we check that topological correctness correctly maps
        // coordinates in the octant close to the origin to the
        // overlap of the far end of the grid delimited by topoDim.
        CoordBox<3> originOctant(Coord<3>(), box.origin + box.dimensions - topoDim);
        for (CoordBox<3>::Iterator i = originOctant.begin(); i != originOctant.end(); ++i) {
            TS_ASSERT_EQUALS(grid.get(*i), defaultCell);
        }

        SoATestCell dummy(4711);
        grid.set(Coord<3>(1, 2, 3), dummy);
        TS_ASSERT_EQUALS(grid.get(Coord<3>( 1,  2,  3)), dummy);
        TS_ASSERT_EQUALS(grid.get(Coord<3>(61, 52, 53)), dummy);
    }

    // fixme: check neighborhood may actually access edgecells
};

}