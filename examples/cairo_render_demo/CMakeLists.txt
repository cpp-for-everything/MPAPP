# Visible Cairo backend demo (ADR-0015). Uses <cairo/cairo.h> when the Cairo
# backend is selected, guarded by MPAPP_GRAPHICS_HAS_CAIRO (propagated from
# mpapp-core). Cross-platform.
mpapp_add_example(cairo_render_demo SOURCES cairo_render_demo.cpp)
