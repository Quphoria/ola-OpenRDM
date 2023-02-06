
#ifndef PLUGINS_OPENRDM_OPENRDMWIDGET_H_
#define PLUGINS_OPENRDM_OPENRDMWIDGET_H_

#include <string>
#include <vector>
#include <mutex>

#include "ola/rdm/UID.h"
#include "ola/rdm/UIDSet.h"
#include "plugins/openrdm/OpenRDMDriver.h"
#include "plugins/openrdm/rdm.h"

namespace ola {
namespace plugin {
namespace openrdm {

using ola::rdm::UID;
using ola::rdm::UIDSet;

class OpenRDMWidget {
    public:
        bool verbose, rdm_enabled, rdm_debug;
        OpenRDMWidget();
        OpenRDMWidget(std::string ftdi_description, bool verbose, bool rdm_enabled, bool rdm_debug);
        ~OpenRDMWidget();
        bool init();
        void deinit();
        bool isInitialized();
        static void findDevices();
        void writeDMX(uint8_t *data, int len);
        std::pair<int, RDMData> writeRDM(uint8_t *data, int len);
        UIDSet fullRDMDiscovery(); // Returns full TOD
        std::pair<UIDSet, UIDSet> incrementalRDMDiscovery(); // Returns pair: added devices, removed devices
    protected:
        UIDSet discover(uint64_t start, uint64_t end);
        UIDSet getProxyTOD(UID addr);
        bool hasProxyTODChanged(UID addr);
        bool sendMute(UID addr, bool unmute, bool &is_proxy);
        std::vector<RDMPacket> sendRDMPacket(RDMPacket pkt, unsigned int retries = 10, double max_time_ms = 2000);
    private:
        bool initialized = false;
        bool discovery_in_progress;
        struct ftdi_context ftdi;
        std::string ftdi_description;
        UID uid;
        uint8_t rdm_transaction_number = 0;
        UIDSet tod, lost, proxies;
        std::unique_ptr<std::mutex> dev_mutex;
};

}  // namespace openrdm
}  // namespace plugin
}  // namespace ola

#endif // PLUGINS_OPENRDM_OPENRDMWIDGET_H_