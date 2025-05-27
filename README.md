**What is it?**

- This is a demo of realtime diffuse global illumination where every pixel on the canvas can be a light source at O(1) complexity
- This is made possible through the use of Radiance Cascades as described by this [paper](https://github.com/Raikiri/RadianceCascadesPaper) by Alexander Sannikov
- This implementation uses 2x angular resolution scaling and supports bilinear fix to minimise light leakage
- The performance of this demo is limited by the amount of DDA raycasts it does, swapping this out for raymarching would be ideal but is beyond the scope of my goals with this project


**Demo Settings/Controls:**

- Edit compile time settings from the Constants.h file. Such as canvas size, probe spacing etc.
- Use the interface to play with the runtime settings
- Left Click and hold on the canvas to paint on it
- Right Click and hold on the canvas to erase drawn pixels


**Beauty Shots:**

![FlatlandRCDemo](https://github.com/user-attachments/assets/6fa9f524-39fa-4032-8376-f8445301290c)

![image](https://github.com/user-attachments/assets/7ab806a5-3e9f-47e7-952b-8e315a847305)

![image](https://github.com/user-attachments/assets/76372174-f03a-405c-95fa-b9905c0a1a63)


**Resources:**

 - Alexander Sannikov's [paper](https://github.com/Raikiri/RadianceCascadesPaper) on Radiance Cascades
 - MAXs [article](https://m4xc.dev/articles/fundamental-rc/) has a great breakdown with visualisations to build intuition
 - XorDev & Yaazarai's articles, [part 1](https://mini.gmshaders.com/p/radiance-cascades) & [part 2](https://mini.gmshaders.com/p/radiance-cascades2)
 - SimonDev's [video](https://youtu.be/3so7xdZHKxw)
 - Jason's [blog post](https://jason.today/rc)
 - Mytino's [Shadertoy implementation](https://www.shadertoy.com/view/4clcWn)
 - The Radiance Cascades [discord](https://discord.com/invite/WQ4hCHhUuU) with an extremely helpful community!
