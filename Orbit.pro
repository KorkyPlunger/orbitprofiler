TEMPLATE = subdirs

SUBDIRS += \
    OrbitBase \
    OrbitCore \
    OrbitGl \
    OrbitQt \

OrbitCore.depends = OrbitBase
OrbitGl.depends   = OrbitCore
OrbitQt.depends   = OrbitGl
