serenity_component(
    Mandelbrot
    TARGETS Mandelbrot
)

set(SOURCES
    Mandelbrot.cpp
)

serenity_app(Mandelbrot ICON app-mandelbrot)
target_link_libraries(Mandelbrot PRIVATE LibGUI LibCore LibGfx LibMain)
