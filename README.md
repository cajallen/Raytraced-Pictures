## Collaboration
I set up this project with John Swenson.

## Compiling
I use Windows and VS. All dependencies are submodules in src/lib. 

## Features
https://www.youtube.com/watch?v=WZ3-YIwMkxs  
Video includes (with timestamps)...  
- "Realtime" rendering for low res images (No acceleration structure... yet)  
- Loading Scenes  
- Arbitrary Camera Setup  
- Geometry Manipulation  
- Material Manipulation  
- Specular Cosine Exponent  
- IOR   
- Instancing Objects  
- Deleting Objects  
- Normal Triangles  
- Spot Lights  
- Background Color  

#### Shadows
![shadows](https://github.com/cajallen/Raytracer/blob/main/output/multi_light_demo.png)  

#### Bounded Recursion
![bounded recursion](https://github.com/cajallen/Raytracer/blob/main/output/recursion_test.png)  
Two triangles, facing each other with a light inbetween. They are both purely specular, the one that we are looking at is perfectly reflective, where as the one we are looking at the back of has half of a red component. We can see the reflection get more red as it reflects, then stop.

#### Jittered Sampling  
![sampling](https://github.com/cajallen/Raytracer/blob/main/output/SamplingDemo.png) 

#### Parallelization
Tested on bottle.p3, 500x500 res. Maybe we could get someone with a decent CPU to test this with more than 4 threads...   
1 thread: ~30 seconds  
2 threads: ~16 seconds  
4 threads: ~9 seconds  

This is about what expected, maybe slightly better than I expected. To go from 1 to 2 threads and gain almost no extra cost (30/2=15, barely <16), and the same for 2 to 4 means there is very little overhead, or actual cost to the parallelization. This is expected because raytracers are prime candidates for parallelization. 
