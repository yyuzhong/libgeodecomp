#include <libgeodecomp/config.h>

#ifdef LIBGEODECOMP_WITH_MPI
#include <libgeodecomp/communication/mpilayer.h>
#endif

#include <fstream>
#include <libgeodecomp/misc/random.h>
#include <libgeodecomp/misc/stringops.h>
#include <libgeodecomp/misc/tempfile.h>
#include <unistd.h>

namespace LibGeoDecomp {

// ugly, but good enough for now
std::string TempFile::serial(const std::string& prefix)
{
    for (;;) {
        std::stringstream buf;
#ifdef __WIN32__
        std::string name = getenv("TMP");
        buf << '\\';
#else
        const char* tempDir = getenv("TMPDIR");
        std::string name = tempDir? tempDir : "/tmp";
        buf << '/';
#endif
        unsigned r = Random::genUnsigned();
        std::string filename = prefix + StringOps::itoa(r);
        buf << filename;
        name += buf.str();
        if (access(name.c_str(), F_OK) == -1) {
            return name;
        }
    }
}

#ifdef LIBGEODECOMP_WITH_MPI

std::string TempFile::parallel(const std::string& prefix)
{
    std::string ret;

    if (MPILayer().rank() == 0) {
        ret = serial(prefix);
        int len = ret.size();
        MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast((void*)ret.c_str(), len, MPI_CHAR, 0, MPI_COMM_WORLD);
    } else {
        int len;
        MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
        ret = std::string(len, 'X');
        MPI_Bcast((void*)ret.c_str(), len, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    return ret;
}

#endif

}
