#ifndef _libgeodecomp_parallelization_hiparsimulator_stepper_h_
#define _libgeodecomp_parallelization_hiparsimulator_stepper_h_

#include <boost/shared_ptr.hpp>

#include <libgeodecomp/io/initializer.h>
#include <libgeodecomp/parallelization/hiparsimulator/offsethelper.h>
#include <libgeodecomp/parallelization/hiparsimulator/partitionmanager.h>
#include <libgeodecomp/parallelization/hiparsimulator/patchaccepter.h>
#include <libgeodecomp/parallelization/hiparsimulator/patchprovider.h>
#include <libgeodecomp/misc/displacedgrid.h>
#include <libgeodecomp/misc/typetraits.h>

namespace LibGeoDecomp {
namespace HiParSimulator {

/**
 * Abstract interface class. Steppers contain some arbitrary region of
 * the grid which they can update. See StepperHelper for details on
 * accessing ghost zones. Do not inherit directly from Stepper, but
 * rather from StepperHelper.
 *
 * fixme: doxygen syntax to link to StepperHelper...
 */
template<typename CELL_TYPE>
class Stepper
{
    friend class StepperTest;
public:
    enum PatchType {GHOST=0, INNER_SET=1};
    typedef typename CELL_TYPE::Topology Topology;
    const static int DIM = Topology::DIMENSIONS;

    typedef DisplacedGrid<CELL_TYPE, Topology, true> GridType;
    typedef PartitionManager<DIM, Topology> MyPartitionManager;
    typedef boost::shared_ptr<PatchProvider<GridType> > PatchProviderPtr;
    typedef boost::shared_ptr<PatchAccepter<GridType> > PatchAccepterPtr;
    typedef std::deque<PatchProviderPtr> PatchProviderList;
    typedef std::deque<PatchAccepterPtr> PatchAccepterList;

    inline Stepper(
        const boost::shared_ptr<MyPartitionManager>& _partitionManager,
        Initializer<CELL_TYPE>  *_initializer) :
        partitionManager(_partitionManager),
        initializer(_initializer)
    {}

    virtual void update(int nanoSteps) = 0;

    virtual const GridType& grid() const = 0;

    // returns current step and nanoStep
    virtual std::pair<int, int> currentStep() const = 0;

    void addPatchProvider(
        const PatchProviderPtr& ghostZonePatchProvider, 
        const PatchType& patchType)
    {
        patchProviders[patchType].push_back(ghostZonePatchProvider);
    }

    void addPatchAccepter(
        const PatchAccepterPtr& ghostZonePatchAccepter, 
        const PatchType& patchType)
    {
        patchAccepters[patchType].push_back(ghostZonePatchAccepter);
    }

protected:
    boost::shared_ptr<MyPartitionManager> partitionManager;
    Initializer<CELL_TYPE> *initializer;
    PatchProviderList patchProviders[2];
    PatchAccepterList patchAccepters[2];

    /**
     * calculates a (mostly) suitable offset which (in conjuction with
     * a DisplacedGrid) avoids having grids with a size equal to the
     * whole simulation area on torus topologies.
     */
    inline void guessOffset(Coord<DIM> *offset, Coord<DIM> *dimensions)
    {
        const CoordBox<DIM>& boundingBox = 
            partitionManager->ownRegion().boundingBox();
        OffsetHelper<DIM - 1, DIM, Topology>()(
            offset,
            dimensions,
            boundingBox,
            initializer->gridBox(),
            partitionManager->getGhostZoneWidth());
    }
};

}
}

#endif
