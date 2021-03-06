#ifndef LIBGEODECOMP_IO_COLLECTINGWRITER_H
#define LIBGEODECOMP_IO_COLLECTINGWRITER_H

#include <libgeodecomp/config.h>
#ifdef LIBGEODECOMP_WITH_MPI

#include <libgeodecomp/communication/mpilayer.h>
#include <libgeodecomp/io/parallelwriter.h>
#include <libgeodecomp/misc/clonable.h>
#include <libgeodecomp/misc/sharedptr.h>

namespace LibGeoDecomp {

/**
 * Adapter class whose purpose is to use legacy Writer objects
 * together with a DistributedSimulator. Good for testing, but doesn't
 * scale, as all memory is concentrated on one node and IO is
 * serialized to that node. Use with care!
 */
template<typename CELL_TYPE>
class CollectingWriter : public Clonable<ParallelWriter<CELL_TYPE>, CollectingWriter<CELL_TYPE> >
{
public:
    typedef typename ParallelWriter<CELL_TYPE>::Topology Topology;
    typedef DisplacedGrid<CELL_TYPE, Topology> StorageGridType;
    typedef typename DistributedSimulator<CELL_TYPE>::GridType SimulatorGridType;

    using ParallelWriter<CELL_TYPE>::period;

    static const int DIM = Topology::DIM;

    explicit CollectingWriter(
        Writer<CELL_TYPE> *writer,
        int root = 0,
        MPI_Comm communicator = MPI_COMM_WORLD,
        MPI_Datatype mpiDatatype = APITraits::SelectMPIDataType<CELL_TYPE>::value()) :
        Clonable<ParallelWriter<CELL_TYPE>, CollectingWriter<CELL_TYPE> >("",  1),
        writer(writer),
        mpiLayer(communicator),
        root(root),
        datatype(mpiDatatype)
    {
        if ((mpiLayer.rank() != root) && (writer != 0)) {
            throw std::invalid_argument("can't call back a writer on a node other than the root");
        }

        if (mpiLayer.rank() == root) {
            if (writer == 0) {
                throw std::invalid_argument("delegate writer on root must not be null");
            }

            period = writer->getPeriod();
        }

        period = mpiLayer.broadcast(period, root);
    }

    virtual void stepFinished(
        const SimulatorGridType& grid,
        const Region<DIM>& validRegion,
        const Coord<DIM>& globalDimensions,
        unsigned step,
        WriterEvent event,
        std::size_t rank,
        bool lastCall)
    {
        if (mpiLayer.rank() == root) {
            if (globalGrid.boundingBox().dimensions != globalDimensions) {
                globalGrid.resize(CoordBox<DIM>(Coord<DIM>(), globalDimensions));
            }

            globalGrid.paste(grid, validRegion);
            globalGrid.setEdge(grid.getEdge());
        }

        CoordBox<DIM> box = grid.boundingBox();
        StorageGridType localGrid(box);

        for (typename CoordBox<DIM>::StreakIterator i = box.beginStreak(); i != box.endStreak(); ++i) {
            Streak<DIM> s(*i);
            grid.get(s, &localGrid[s.origin]);
        }

        for (int sender = 0; sender < mpiLayer.size(); ++sender) {
            if (sender != root) {
                if (mpiLayer.rank() == root) {
                    Region<DIM> recvRegion;
                    mpiLayer.recvRegion(&recvRegion, sender);
                    mpiLayer.recvUnregisteredRegion(
                        &globalGrid,
                        recvRegion,
                        sender,
                        MPILayer::PARALLEL_MEMORY_WRITER,
                        datatype);
                }
                if (mpiLayer.rank() == sender) {
                    mpiLayer.sendRegion(validRegion, root);
                    mpiLayer.sendUnregisteredRegion(
                        &localGrid,
                        validRegion,
                        root,
                        MPILayer::PARALLEL_MEMORY_WRITER,
                        datatype);
                }
            }
        }

        mpiLayer.waitAll();

        if (lastCall && (mpiLayer.rank() == root)) {
            writer->stepFinished(*globalGrid.vanillaGrid(), step, event);
        }
    }

private:
    typename SharedPtr<Writer<CELL_TYPE> >::Type writer;
    MPILayer mpiLayer;
    int root;
    StorageGridType globalGrid;
    MPI_Datatype datatype;
};

}

#endif

#endif
