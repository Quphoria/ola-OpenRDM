#ifndef PLUGINS_OPENRDM_OPENRDMDEVICE_H_
#define PLUGINS_OPENRDM_OPENRDMDEVICE_H_

#include <string>
#include <sstream>
#include <memory>
#include "ola/DmxBuffer.h"
#include "olad/Device.h"
#include "olad/Port.h"
#include "olad/Preferences.h"
#include "plugins/openrdm/OpenRDMWidget.h"
#include "plugins/openrdm/OpenRDMThread.h"

namespace ola {
namespace plugin {
namespace openrdm {

class OpenRDMDevice : public Device {
 public:
  OpenRDMDevice(AbstractPlugin *owner,
                const unsigned int &port_id
                const std::string &device_string,
                const unsigned int &dmx_refresh_ms,
                const bool &rdm_enabled);
  ~OpenRDMDevice();

  std::string DeviceId() const { return m_dev_str; }
  OpenRDMWidget* GetWidget() { return m_widget.get(); }

 protected:
  bool StartHook();

 private:
  std::auto_ptr<OpenRDMWidget> m_widget;
  unsigned int m_port_id;
  const std::string m_dev_str;
  unsigned int m_dmx_refresh_ms;
  bool m_rdm_enabled;

  DISALLOW_COPY_AND_ASSIGN(OpenRDMDevice);
};

class OpenRDMOutputPort : public ola::BasicOutputPort {
 public:
  OpenRDMOutputPort(OpenRDMDevice *parent,
                    unsigned int id,
                    OpenRDMWidget *widget,
                    const std::string &device_string,
                    unsigned int dmx_refresh_ms,
                    bool rdm_enabled);
  ~OpenRDMOutputPort();

  bool WriteDMX(const ola::DmxBuffer &buffer, uint8_t);
  void SendRDMRequest(ola::rdm::RDMRequest *request,
                      ola::rdm::RDMCallback *callback);
  void RunFullDiscovery(ola::rdm::RDMDiscoveryCallback *callback);
  void RunIncrementalDiscovery(ola::rdm::RDMDiscoveryCallback *callback);

  std::string Description() const { return "OpenRDM Device:" + m_dev_str; }

  std::string UniqueId() const { return "OpenRDM:" + m_dev_str; }

 private:
  OpenRDMWidget *m_widget;
  OpenRDMThread m_thread;
  const std::string m_dev_str;
  unsigned int m_dmx_refresh_ms;
  bool m_rdm_enabled;

  DISALLOW_COPY_AND_ASSIGN(OpenRDMOutputPort);
};
}  // namespace openrdm
}  // namespace plugin
}  // namespace ola
#endif  // PLUGINS_OPENRDM_OPENRDMDEVICE_H_
