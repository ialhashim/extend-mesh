# extend-mesh
A proposed interactive and simple method that creates model variations by applying stretching on 3D models. The method replicates the geometric details and synthesizes extensions by applying texture synthesis techniques on surface details.

![](http://www.cs.sfu.ca/~iaa7/personal/stretch_idea.png)

##Tools needed
 * C++ and OpenGL
 * [Qt 4.7](http://qt.nokia.com/)
 * [libQGLViewer (2.3.9)](http://www.libqglviewer.com/)
 * [GLee](http://elf-stone.com/glee.php) or [GLEW](http://glew.sourceforge.net/)

##Installation & usage
 * Developed on Windows, Linux port is experimental
 * Compile using MSVC 2008 with Qt add-in (qmake / Qt creator on Linux). Before that, compile the solver in the "Solver" directory using MSVC (make on Linux).
 * Example models are provided. Open a mesh and click 'SHIFT' at two different parts to select a region of interest. Then press 'ALT' and draw a curve to extend that part.

##Real-Time Editing Sessions
<wiki:video url="http://www.youtube.com/watch?v=VAXm7Wm-R7c"/>

##Publication

Ibraheem Alhashim, Hao Zhang, and Ligang Liu, ["Detail-Replicating Shape Stretching,"](http://www.springerlink.com/content/e1812qvx45121783/) the Visual Computer, 2012.

*BibTeX*
```
@article {springerlink:10.1007/s00371-011-0665-9,
   author = {Alhashim, Ibraheem and Zhang, Hao and Liu, Ligang},
   affiliation = {Simon Fraser University, Burnaby, Canada},
   title = {Detail-replicating shape stretching},
   journal = {The Visual Computer},
   publisher = {Springer Berlin / Heidelberg},
   issn = {0178-2789},
   keyword = {Computer Science},
   pages = {1-14},
   url = {http://dx.doi.org/10.1007/s00371-011-0665-9},
   note = {10.1007/s00371-011-0665-9},
}
```
