#ifndef PLUGINS_OPENRDM_OPENRDMPLUGIN_H_
#define PLUGINS_OPENRDM_OPENRDMPLUGIN_H_

#include <set>
#include <string>
#include <vector>

#include "olad/Plugin.h"
#include "ola/plugin_id.h"

#include "plugins/openrdm/OpenRDMDevice.h"

namespace ola {
namespace plugin {
namespace openrdm {


class OpenRDMPlugin : public Plugin {
 public:
  explicit OpenRDMPlugin(ola::PluginAdaptor *plugin_adaptor)
      : Plugin(plugin_adaptor) {
  }

  ola_plugin_id Id() const { return OLA_PLUGIN_OPENRDM; }
  std::string Name() const { return PLUGIN_NAME; }
  std::string PluginPrefix() const { return PLUGIN_PREFIX; }
  // This plugin is disabled unless explicitly enabled by a user.
  bool DefaultMode() const { return false; }

  void ConflictsWith(std::set<ola_plugin_id> *conflict_set) const {
    conflict_set->insert(ola::OLA_PLUGIN_USBPRO);
    conflict_set->insert(ola::OLA_PLUGIN_OPENDMX);
    conflict_set->insert(ola::OLA_PLUGIN_FTDIDMX);
  }

  std::string Description() const;

 private:
  typedef std::vector<OpenRDMDevice*> OpenRDMDeviceVector;
  OpenRDMDeviceVector m_devices;

  void AddDevice(OpenRDMDevice *device);
  bool StartHook();
  bool StopHook();
  bool SetDefaultPreferences();

  unsigned int GetOutputPortCount() const;
  std::string GetPortDeviceString(unsigned int port) const;
  unsigned int GetPortRefreshMS(unsigned int port) const;
  bool GetPortRDMEnabled(unsigned int port) const;

  static const uint8_t DEFAULT_OUTPUT_PORTS = 1;
  static const char DEFAULT_PORT_DEVICE[];
  static const uint8_t DEFAULT_REFRESH_MS = 50;
  static const bool DEFAULT_RDM_ENABLED = true;

  static const char K_OUTPUT_PORTS[];
  static const char K_PORT_DEVICE_TEMPL[];
  static const char K_PORT_REFRESH_TEMPL[];
  static const char K_PORT_RDM_TEMPL[];

  static const char PLUGIN_NAME[];
  static const char PLUGIN_PREFIX[];
};
}  // namespace openrdm
}  // namespace plugin
}  // namespace ola
#endif  // PLUGINS_OPENRDM_OPENRDMPLUGIN_H_