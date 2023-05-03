# What is this?
As a hobby project, I wanted to try my hand at creating a simplified 3D rendering pipeline, so I used some tools from the Win32 API to create a blank window on which I could edit pixel colors.  From scratch, I developed the tools required for everything shown in the examples, from a linear algebra library to the functions for drawing 2D shapes.  

In the near future, I plan on recreating this using OpenGL, as the current solution is incredibly inefficient.  Pretty much everything I've done will get a full rework.

View the code here: https://github.com/ebajec/Renderer

# Examples  
Each of these executables is an old build of this project, showing a different aspect.  To move the camera, use WASD + SPACE/SHIFT.  Please note that these are not stable in any way, shape, or form, so be prepared for potential weirdness.

**cool shape:** This is just a cool shape I made by applying a vector field to a sphere.  

**cube jumping:** I was playing around with collision and made a little game-like thing where you can jump on and run into a bunch of cubes.  

*Note:* There is a strange bug I couldn't fix where you pass through the first cube if you don't move before falling on it.  

**wave:** I made this by overlapping a bunch of 2D wave functions on a surface.  

**basic shading:**  This version draws the faces for a bunch of cubes and uses a very basic shading technique to color them.  There are still some issues with calculating the surface normals correctly, and I have not yet finished the clipping for faces.  

*Note:* The program with attempt to draw an infinitely large quadrilateral and crash if the camera plane intersects a cube, so maybe avoid doing that.   
