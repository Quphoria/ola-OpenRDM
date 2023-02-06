
#include <algorithm>
#include <thread>
#include <chrono>
#include <memory>

#include "ola/Logging.h"
#include "plugins/openrdm/OpenRDMWidget.h"

namespace ola {
namespace plugin {
namespace openrdm {

using ola::rdm::UID;
using ola::rdm::UIDSet;

OpenRDMWidget::OpenRDMWidget()
        :uid(UID(0, 0)) {
    this->ftdi_description = "";
    this->verbose = 0;
    this->rdm_enabled = false;
    this->rdm_debug = false;
    this->dev_mutex = std::unique_ptr<std::mutex>(new std::mutex());
}

OpenRDMWidget::OpenRDMWidget(std::string ftdi_description, bool verbose, bool rdm_enabled, bool rdm_debug)
        :uid(UID(0, 0)) {
    this->ftdi_description = ftdi_description;
    this->verbose = verbose;
    this->rdm_enabled = rdm_enabled;
    this->rdm_debug = rdm_debug;
    this->dev_mutex = std::unique_ptr<std::mutex>(new std::mutex());
}

OpenRDMWidget::~OpenRDMWidget() {
    deinit();
}

bool OpenRDMWidget::init() {
    if (initOpenRDM(verbose, &ftdi, ftdi_description.c_str())) {
        uid = generateUID(ftdi_description);
        discovery_in_progress = false;
        rdm_transaction_number = 0;
        tod = UIDSet();
        lost = UIDSet();
        proxies = UIDSet();
        initialized = true;
        return true;
    }
    deinit();
    return false;
}

void OpenRDMWidget::deinit() {
    if (!initialized) return;
    deinitOpenRDM(verbose, &ftdi);
    initialized = false;
}

bool OpenRDMWidget::isInitialized() { return initialized; }

void OpenRDMWidget::findDevices() {
    struct ftdi_context ftdi;
    struct ftdi_device_list *devlist;

    OLA_DEBUG << "OPENRDM Finding Devices";
    
    ftdi_init(&ftdi);
    int ret = ftdi_usb_find_all(&ftdi, &devlist, OPENRDM_VID, OPENRDM_PID);
    if (ret < 0) {
        OLA_WARN << "OPENRDM FTDI ERROR " << ret << ": " << ftdi.error_str;
        ftdi_deinit(&ftdi);
        ftdi_list_free(&devlist);
        return;
    }
    ftdi_deinit(&ftdi);

    struct ftdi_device_list *dp = devlist;
    while (dp) {
        char manufacturer[51];
        char description[51];
        char serial[51];

        ftdi_init(&ftdi);

        ret = ftdi_usb_get_strings(&ftdi, dp->dev,
            manufacturer, 50,
            description, 50,
            serial, 50);

        if (ret != 0) {
            OLA_WARN << "OPENRDM FTDI ERROR " << ret << ": " << ftdi.error_str;
        } else {
            // 100 is over max possible length
            char dev_str[100];
            sprintf(dev_str, "s:0x%04x:0x%04x:%s", OPENRDM_VID, OPENRDM_PID, serial);
            OLA_DEBUG << "OpenRDM Device Found: device_string='" << dev_str << "'";
        }

        ftdi_deinit(&ftdi);

        dp = dp->next;
    }

    ftdi_list_free(&devlist);
}

void OpenRDMWidget::writeDMX(uint8_t *data, int len) {
    if (!initialized) return;
    this->dev_mutex->lock();
    int ret = writeDMXOpenRDM(verbose, &ftdi, data, len, ftdi_description.c_str());
    this->dev_mutex->unlock();
    if (ret < 0) { // Error occurred
        // -666: USB device unavailable, wait a bit to avoid spam
        if (ret == -666) std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

std::pair<int, RDMData> OpenRDMWidget::writeRDM(uint8_t *data, int len) {
    if (!initialized) return std::make_pair(0, RDMData());
    auto resp = RDMData();
    this->dev_mutex->lock();
    int resp_len = writeRDMOpenRDM(verbose, &ftdi, data, len, false, resp.begin(), ftdi_description.c_str());
    this->dev_mutex->unlock();
    if (resp_len < 0) { // Error occurred
        // -666: USB device unavailable, wait a bit to avoid spam
        if (resp_len == -666) std::this_thread::sleep_for(std::chrono::seconds(1));
        return std::make_pair(0, RDMData());
    }
    return std::make_pair(resp_len, resp);
}

UIDSet OpenRDMWidget::fullRDMDiscovery() {
    if (!initialized) return UIDSet();
    if (discovery_in_progress || !rdm_enabled) return UIDSet();

    discovery_in_progress = true;
    lost = UIDSet();
    proxies = UIDSet();

    bool NA = false;
    sendMute(UID(RDM_UID_BROADCAST), true, NA); // Unmute everything
    tod = discover(0, RDM_UID_MAX);

    if (verbose) {
        for (auto it = tod.Begin(); it != tod.End(); it++) {
            auto uid = *it;
            OLA_DEBUG << "RDM Device Discovered: " << uid.ToString();
        }
    }

    discovery_in_progress = false;
    return tod;
}

std::pair<UIDSet, UIDSet> OpenRDMWidget::incrementalRDMDiscovery() {
    if (!initialized) return std::make_pair(UIDSet(), UIDSet());
    if (discovery_in_progress || !rdm_enabled) return std::make_pair(UIDSet(), UIDSet());
    discovery_in_progress = true;
    auto found = UIDSet();
    auto new_lost = UIDSet();
    auto new_proxies = UIDSet();
    bool NA;
    sendMute(UID(RDM_UID_BROADCAST), true, NA); // Unmute everything
    // Check tod devices are still there and lost devices are still lost
    // This also mutes devices we know about
    for (auto it = tod.Begin(); it != tod.End(); it++) {
        auto uid = *it;
        bool is_proxy = false;
        if (!sendMute(uid, false, is_proxy)) {
            new_lost.AddUID(uid);
            proxies.RemoveUID(uid);
        } else {
            if (is_proxy) {
                new_proxies.AddUID(uid);
                proxies.AddUID(uid);
            } else {
                proxies.RemoveUID(uid);
            }
        }
    }
    for (auto it = lost.Begin(); it != lost.End(); it++) {
        auto uid = *it;
        bool is_proxy = false;
        if (sendMute(uid, false, is_proxy)) {
            found.AddUID(uid);
            if (is_proxy) {
                if (!proxies.Contains(uid)) {
                    new_proxies.AddUID(uid);
                    proxies.AddUID(uid);
                }
            }
        }
    }

    auto discovered = discover(0, RDM_UID_MAX);
    
    for (auto it = proxies.Begin(); it != proxies.End(); it++) {
        auto proxy_uid = *it;
        // If proxy is not in new_proxies, don't bother checking if its TOD has changed as we want to scan anyway
        if (!new_proxies.Contains(proxy_uid)) {
            if (!hasProxyTODChanged(proxy_uid)) continue;
        }
        auto new_proxy_tod = getProxyTOD(proxy_uid);
        discovered = discovered.Union(new_proxy_tod);
    }

    for (auto it = discovered.Begin(); it != discovered.End(); it++) {
        auto uid = *it;
        // If we find a device that has been found as lost, but it isn't, remove it from lost
        new_lost.RemoveUID(uid);
        // Merge into found if unique and new
        if (!tod.Contains(uid)) {
            found.AddUID(uid);
        }
    }

    // Apply changes to tod and lost
    tod = tod.SetDifference(new_lost).Union(found);
    lost = lost.Union(new_lost).SetDifference(found);

    if (verbose) {
        for (auto it = new_lost.Begin(); it != new_lost.End(); it++) {
            auto uid = *it;
            OLA_DEBUG << "RDM Device Lost: " << uid.ToString();
        }
        for (auto it = found.Begin(); it != found.End(); it++) {
            auto uid = *it;
            OLA_DEBUG << "RDM Device Discovered: " << uid.ToString();
        }
    }

    discovery_in_progress = false;
    return std::make_pair(found, new_lost);
}

UIDSet OpenRDMWidget::discover(uint64_t start, uint64_t end) {
    UID mute_uid = UID(start);
    if (start != end) {
        auto disc_msg_data = RDMPacketData();
        UID(start).Pack(disc_msg_data.begin(), disc_msg_data.size());
        UID(end).Pack(disc_msg_data.begin()+RDM_UID_LENGTH, disc_msg_data.size()-RDM_UID_LENGTH);
        auto disc_msg = RDMPacket(UID(RDM_UID_BROADCAST), uid, rdm_transaction_number++, 0x1, 0, 0,
            RDM_CC_DISCOVER, RDM_PID_DISC_UNIQUE_BRANCH, RDM_UID_LENGTH*2, disc_msg_data);
        auto disc_msg_packet = RDMData();
        size_t msg_len = disc_msg.writePacket(disc_msg_packet);

        auto response = RDMData();
        this->dev_mutex->lock();
        int resp_len = writeRDMOpenRDM(verbose, &ftdi,
            disc_msg_packet.begin(), msg_len, true, response.begin(), ftdi_description.c_str());
        this->dev_mutex->unlock();
        if (resp_len <= 0) { // Error occurred or no data
            // -666: USB device unavailable, wait a bit to avoid spam
            if (resp_len == -666) std::this_thread::sleep_for(std::chrono::seconds(1));
            return UIDSet();
        }

        if (this->rdm_debug) {
            printf("Discovery Resp:");
            for (int i = 0; i < resp_len; i++) {
                printf(" %02x", response[i]);
            }
            printf("\n");
        }

        auto resp = DiscoveryResponseRDMPacket(response, resp_len);
        if (!resp.isValid()) {
            if (this->rdm_debug) {
                printf("Invalid discovery response\n");
            }
            uint64_t lower_half_size = (end-start+1) / 2; // Start and end inclusive
            uint64_t lower_half_max = start+lower_half_size-1; // Start inclusive
            auto lower_half = discover(start, lower_half_max);
            auto upper_half = discover(lower_half_max+1, end);
            return lower_half.Union(upper_half);
        }
        mute_uid = resp.getUID();
    }
    bool is_proxy = false;
    // If we don't get a mute response, there is no device with that uid
    if (!sendMute(mute_uid, false, is_proxy)) return UIDSet();
    auto discovered_uids = UIDSet();
    discovered_uids.AddUID(mute_uid);

    if (!is_proxy) return discovered_uids;

    return discovered_uids.Union(getProxyTOD(mute_uid));
}

UIDSet OpenRDMWidget::getProxyTOD(UID addr) {
    auto proxy_tod_msg = RDMPacket(addr, uid, rdm_transaction_number++, 0x1, 0, 0,
        RDM_CC_GET_COMMAND, RDM_PID_PROXIED_DEVICES, 0, RDMPacketData());

    auto proxy_tod = UIDSet();
    
    auto resp = sendRDMPacket(proxy_tod_msg);
    if (resp.size() == 0) return proxy_tod;

    for (auto &r : resp) {
        if (r.pdl > 0xe4) continue;
        for (size_t i = 0; i < r.pdl; i += RDM_UID_LENGTH)
            proxy_tod.AddUID(UID(&r.pdata[i]));
    }

    return proxy_tod;
}

bool OpenRDMWidget::hasProxyTODChanged(UID addr) {
    auto proxy_tod_changed_msg = RDMPacket(addr, uid, rdm_transaction_number++, 0x1, 0, 0,
        RDM_CC_GET_COMMAND, RDM_PID_PROXY_DEV_COUNT, 0, RDMPacketData());

    auto resp = sendRDMPacket(proxy_tod_changed_msg);
    if (resp.size() == 0) return false;
    if (resp[0].pdl != 0x03) return false;
    
    return resp[0].pdata[2] != 0;
}

bool OpenRDMWidget::sendMute(UID addr, bool unmute, bool &is_proxy) {
    auto mute_msg = RDMPacket(addr, uid, rdm_transaction_number++, 0x1, 0, 0,
        RDM_CC_DISCOVER, unmute ? RDM_PID_DISC_UNMUTE : RDM_PID_DISC_MUTE,
        0, RDMPacketData());
    if (this->rdm_debug) {
        if (unmute) OLA_DEBUG << "Sending UNMUTE to " << addr;
        else OLA_DEBUG << "Sending MUTE to " << addr;
    }

    auto resp = sendRDMPacket(mute_msg);
    if (resp.size() == 0) return false;
    if (resp[0].getSrc() != addr) return false;

    if (resp[0].pdl == 0x02 || resp[0].pdl == 0x08) {
        uint16_t control_field = ((uint16_t)resp[0].pdata[0] << 8) | (uint16_t)resp[0].pdata[1];
        is_proxy = (control_field & RDM_CONTROL_MANAGED_PROXY_BITMASK) != 0;
    }

    if (this->rdm_debug) {
        if (unmute) OLA_DEBUG << "UNMUTE Response from " << addr;
        else OLA_DEBUG << "MUTE Response from " << addr;
    }

    return true;
}

std::vector<RDMPacket> OpenRDMWidget::sendRDMPacket(RDMPacket pkt, unsigned int retries, double max_time_ms) {
    auto resp_packets = std::vector<RDMPacket>();
    double retry_time_ms = max_time_ms;
    auto msg = RDMData();

    auto t_start = std::chrono::high_resolution_clock::now();
    auto pkt_pid = pkt.pid;

    // Don't count first try as a retry
    for (unsigned int pkt_try = 0; pkt_try <= retries; pkt_try++) {
        if (pkt_try != 0) {
            pkt.transaction_number = rdm_transaction_number++;
        }
        size_t msg_len = pkt.writePacket(msg);
        auto t_now = std::chrono::high_resolution_clock::now();
        double elapsed_time_ms = std::chrono::duration<double, std::milli>(t_now-t_start).count();
        if (pkt_try > 0 && elapsed_time_ms > max_time_ms) break;

        auto response = RDMData();
        this->dev_mutex->lock();
        int resp_len = writeRDMOpenRDM(verbose, &ftdi,
            msg.begin(), msg_len, false, response.begin(), ftdi_description.c_str());
        this->dev_mutex->unlock();
        if (resp_len < 0) { // Error occurred
            // -666: USB device unavailable, wait a bit to avoid spam
            if (resp_len == -666) std::this_thread::sleep_for(std::chrono::seconds(1));
            return std::vector<RDMPacket>();
        }
        if (resp_len == 0) continue;

        auto resp = RDMPacket(uid, response, resp_len);
        if (!resp.isValid()) continue;
        if (resp.transaction_number != pkt.transaction_number) continue; // Check transaction numbers's match
        if (resp.pid != pkt_pid) continue; // Check PID is correct (so we ignore stray queued messages)

        if (resp.cc == RDM_CC_DISCOVER_RESP || pkt.cc == RDM_CC_DISCOVER) {
            if (resp.getRespType() == RDM_RESP_ACK) {
                resp_packets.push_back(resp);
                break;
            }
        } else if (resp.cc == RDM_CC_GET_COMMAND_RESP || resp.cc == RDM_CC_SET_COMMAND_RESP) {
            switch (resp.getRespType()) {
                case RDM_RESP_ACK:
                    resp_packets.push_back(resp);
                    return resp_packets;
                case RDM_RESP_ACK_OVERFL:
                    resp_packets.push_back(resp);
                    break;
                case RDM_RESP_ACK_TIMER:
                    if (resp.pdl != 2) continue;
                    retry_time_ms = 100 * (double)(((uint16_t)resp.pdata[0] << 8) | (uint16_t)resp.pdata[1]);
                    pkt.cc = RDM_CC_GET_COMMAND;
                    pkt.pid = RDM_PID_QUEUED_MESSAGE;
                    pkt.pdl = 1;
                    pkt.pdata[0] = RDM_STATUS_ERROR;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1)*std::min(max_time_ms, retry_time_ms));
                    break;
                case RDM_RESP_NACK:
                    break;
            }
        }
    }

    return resp_packets;
}

}  // namespace openrdm
}  // namespace plugin
}  // namespace ola