serenity_component(
    WidgetGallery
    TARGETS WidgetGallery
)

compile_gml(./GalleryGML/Window.gml WindowGML.h window_gml)
compile_gml(./GalleryGML/BasicsTab.gml BasicsTabGML.h basics_tab_gml)
compile_gml(./GalleryGML/SlidersTab.gml SlidersTabGML.h sliders_tab_gml)
compile_gml(./GalleryGML/CursorsTab.gml CursorsTabGML.h cursors_tab_gml)
compile_gml(./GalleryGML/IconsTab.gml IconsTabGML.h icons_tab_gml)
compile_gml(./GalleryGML/WizardsTab.gml WizardsTabGML.h wizards_tab_gml)
compile_gml(DemoWizardPage1.gml DemoWizardPage1GML.h demo_wizard_page_1_gml)
compile_gml(DemoWizardPage2.gml DemoWizardPage2GML.h demo_wizard_page_2_gml)

set(SOURCES
    main.cpp
    GalleryWidget.cpp
    DemoWizardDialog.cpp
)

set(GENERATED_SOURCES
    WindowGML.h
    BasicsTabGML.h
    SlidersTabGML.h
    CursorsTabGML.h
    IconsTabGML.h
    WizardsTabGML.h
    DemoWizardPage1GML.h
    DemoWizardPage2GML.h
)

serenity_app(WidgetGallery ICON app-widget-gallery)
target_link_libraries(WidgetGallery PRIVATE LibCore LibGfx LibGUI LibMain LibFileSystemAccessClient LibIPC)
