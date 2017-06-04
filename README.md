# Led Cube 8x8x8

Led Cube is built of 512 LEDs. It is based on Atmega16 and has USB interface for connection to a PC. Connection is done though FT232R micro chip.

![Cube outside view](files/cube_outside.jpg?raw=true "Cube outside")
![Cube inside view](files/cube_inside.jpg?raw=true "Cube inside")
[![Cube demonstration](https://img.youtube.com/vi/Hagad9muHUU/0.jpg)](https://www.youtube.com/watch?v=Hagad9muHUU)

## Firmware
The firmware contains a screen saver function. It is displayed, when there is no connection established with the management program. The firmware is built using WinAVR compiler.

## Management program
Management program is also called VoxelEditor. It is a Windows program. VoxelEditor includes libraries ftd2xx.lib for connection with FT232R, OpenGL for displaying voxels and bass.lib for playing music. Voxel is analog of pixel, but in volume. Voxel map is stored in the VoxelEditor. It is displayed on the PC using OpenGL. Simultaniously it is converted to the cube state and cube state is transmitted to the cube. Voxel map may be edited, saved to file or loaded from file.
