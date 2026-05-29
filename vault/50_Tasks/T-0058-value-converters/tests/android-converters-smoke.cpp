#include <mpapp/binding/converters.hpp>
#include <mpapp/binding/binding.hpp>
#include <mpapp/observable.hpp>
int main(){ mpapp::invert_bool_converter c; mpapp::bool_to_visibility_converter v; (void)c.convert(true); (void)v.convert(false); auto f=mpapp::invert_bool(); auto g=mpapp::bool_to_visibility(); return (int)f(true)+(int)(g(false)==mpapp::visibility::collapsed)-1; }
