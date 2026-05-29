#include <chrono>
#include <memory>
#include <mpapp/animation/easing.hpp>
#include <mpapp/animation/animation.hpp>
#include <mpapp/animation/animation_manager.hpp>
#include <mpapp/animation/view_animations.hpp>
#include <mpapp/view.hpp>
using namespace std::chrono_literals;
namespace { class sv : public mpapp::view {}; }
int main(){
  (void)mpapp::ease(mpapp::easing_kind::cubic_in_out, 0.5);
  mpapp::animation a{[](double){},0.0,1.0,100ms,mpapp::easing_kind::sin_out};
  (void)a.advance(16ms);
  mpapp::animation_manager m; sv v;
  m.start(mpapp::fade_to(v,0.0,100ms));
  m.start(mpapp::translate_to(v,10,20,100ms));
  m.start(mpapp::scale_to(v,2.0,100ms));
  m.start(mpapp::rotate_to(v,90.0,100ms));
  m.tick(16ms);
  return (int)m.active_count(); }
