#ifndef LIBGEODECOMP_PARALLELIZATION_NESTING_MPIUPDATEGROUP_H
#define LIBGEODECOMP_PARALLELIZATION_NESTING_MPIUPDATEGROUP_H

#include <libgeodecomp/config.h>
#ifdef LIBGEODECOMP_WITH_MPI

#include <libgeodecomp/communication/mpilayer.h>
#include <libgeodecomp/communication/patchlink.h>
#include <libgeodecomp/parallelization/nesting/updategroup.h>

namespace LibGeoDecomp {

class HiParSimulatorTest;

/**
 * This is an implementation of the UpdateGroup for MPI-based
 * hiearchical Simulators, e.g. the HiParSimulator.
 */
template<class CELL_TYPE>
class MPIUpdateGroup : public UpdateGroup<CELL_TYPE, PatchLink>
{
public:
    friend class LibGeoDecomp::HiParSimulatorTest;
    friend class UpdateGroupPrototypeTest;
    friend class UpdateGroupTest;

    typedef typename UpdateGroup<CELL_TYPE, PatchLink>::PatchAccepterVec PatchAccepterVec;
    typedef typename UpdateGroup<CELL_TYPE, PatchLink>::PatchProviderVec PatchProviderVec;
    typedef typename UpdateGroup<CELL_TYPE, PatchLink>::PatchLinkAccepter PatchLinkAccepter;
    typedef typename UpdateGroup<CELL_TYPE, PatchLink>::PatchLinkProvider PatchLinkProvider;
    typedef typename UpdateGroup<CELL_TYPE, PatchLink>::InitPtr InitPtr;
    typedef typename UpdateGroup<CELL_TYPE, PatchLink>::PartitionPtr PartitionPtr;
    typedef typename UpdateGroup<CELL_TYPE, PatchLink>::PatchLinkAccepterPtr PatchLinkAccepterPtr;
    typedef typename UpdateGroup<CELL_TYPE, PatchLink>::PatchLinkProviderPtr PatchLinkProviderPtr;

    using UpdateGroup<CELL_TYPE, PatchLink>::init;
    using UpdateGroup<CELL_TYPE, PatchLink>::rank;

    const static int DIM = UpdateGroup<CELL_TYPE, PatchLink>::DIM;

    template<typename STEPPER>
    MPIUpdateGroup(
        PartitionPtr partition,
        const CoordBox<DIM>& box,
        unsigned ghostZoneWidth,
        InitPtr initializer,
        STEPPER *stepperType,
        PatchAccepterVec patchAcceptersGhost = PatchAccepterVec(),
        PatchAccepterVec patchAcceptersInner = PatchAccepterVec(),
        PatchProviderVec patchProvidersGhost = PatchProviderVec(),
        PatchProviderVec patchProvidersInner = PatchProviderVec(),
        bool enableFineGrainedParallelism = false,
        MPI_Comm communicator = MPI_COMM_WORLD) :
        UpdateGroup<CELL_TYPE, PatchLink>(ghostZoneWidth, initializer, MPILayer(communicator).rank()),
        mpiLayer(communicator)
    {
        init(
            partition,
            box,
            ghostZoneWidth,
            initializer,
            stepperType,
            patchAcceptersGhost,
            patchAcceptersInner,
            patchProvidersGhost,
            patchProvidersInner,
            enableFineGrainedParallelism);
    }

private:
    MPILayer mpiLayer;

    std::vector<CoordBox<DIM> > gatherBoundingBoxes(
        const CoordBox<DIM>& ownBoundingBox,
        PartitionPtr partition) const
    {
        std::vector<CoordBox<DIM> > boundingBoxes(mpiLayer.size());
        mpiLayer.allGather(ownBoundingBox, &boundingBoxes);
        return boundingBoxes;
    }

    virtual PatchLinkAccepterPtr makePatchLinkAccepter(int target, const Region<DIM>& region)
    {
        return PatchLinkAccepterPtr(
            new PatchLinkAccepter(
                region,
                target,
                MPILayer::PATCH_LINK,
                SerializationBuffer<CELL_TYPE>::cellMPIDataType(),
                mpiLayer.communicator()));

    }

    virtual PatchLinkProviderPtr makePatchLinkProvider(int source, const Region<DIM>& region)
    {
        return PatchLinkProviderPtr(
            new PatchLinkProvider(
                region,
                source,
                MPILayer::PATCH_LINK,
                SerializationBuffer<CELL_TYPE>::cellMPIDataType(),
                mpiLayer.communicator()));
    }
};

}

#endif
#endif
