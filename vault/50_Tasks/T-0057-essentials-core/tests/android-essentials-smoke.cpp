#include <mpapp/essentials/preferences.hpp>
#include <mpapp/essentials/secure_storage.hpp>
#include <mpapp/essentials/connectivity.hpp>
#include <mpapp/essentials/device_info.hpp>
int main(){
  mpapp::in_memory_preferences p; p.set("k", 3L); (void)p.get("k", 0L);
  mpapp::in_memory_secure_storage s; s.set("t","v"); (void)s.get("t");
  mpapp::mock_connectivity c{mpapp::network_access::internet}; c.set_access(mpapp::network_access::none);
  auto d = mpapp::current_device_info();
  return (int)d.platform; }
