#include <memory>
#include <mpapp/di/app_builder.hpp>
#include <mpapp/di/service_collection.hpp>
namespace { struct dep{int n=1;}; struct svc{ std::shared_ptr<dep> d; }; }
int main(){
  mpapp::app_builder b;
  b.services().add_singleton<dep>();
  b.services().add_transient<svc>(std::function<std::shared_ptr<svc>(mpapp::service_provider&)>{
    [](mpapp::service_provider& sp){ auto s=std::make_shared<svc>(); s->d=sp.get_required<dep>(); return s; }});
  auto sp=b.build();
  return sp.get<svc>()->d->n - 1; }
