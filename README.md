# QtQuickComputeItem

The idea behind the QtQuickComputeItem is to develop an Qt Quick Item that works similar to Qt Quick's ShaderItem.

# Build

Requirements: Qt6, cmake, Vulkan

```
git clone ...
mkdir build_QtQuickComputeItem && cd build_QtQuickComputeItem
cmake ../QtQuickComputeItem
make
```

# Example

You have to set the import path, before you can use the plugin. Tthe following command assumes, that your current directory is the build directory:

`export QML_IMPORT_PATH=$(pwd)/lib/`

Run the gray scott reaction diffusion example (with image buffers):

```
cd examples/grayscott_image
./qmlgrayscottimage
```

# Documentation

Yes.