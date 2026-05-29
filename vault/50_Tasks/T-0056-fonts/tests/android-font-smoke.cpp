#include <mpapp/fonts/font.hpp>
#include <mpapp/fonts/font_registry.hpp>
int main(){
  auto f = mpapp::font::of_size("OpenSans",16.0).with_weight(mpapp::font_weight::bold);
  mpapp::font_registry r; r.add_font("a.ttf","A");
  mpapp::configure_fonts(r,[](mpapp::font_registry& reg){ reg.add_font("b.ttf","B"); });
  return (f.is_bold()?0:1) + (r.has_alias("B")?0:1) + (int)r.resolve("A").has_value() - 1; }
