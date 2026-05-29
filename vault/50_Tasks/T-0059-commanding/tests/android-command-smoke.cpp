#include <mpapp/binding/relay_command.hpp>
int main(){ int r=0; mpapp::relay_command c{[&]{++r;},[]{return true;}}; c.execute(); mpapp::relay_command_of<int> p{[](const int&){}}; p.execute(5); mpapp::command_base& b=c; b.execute(); return r-2; }
