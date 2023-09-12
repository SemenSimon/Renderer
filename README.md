# What is this?
*Note:* I am no longer working on this project; I made a better renderer [here](https://github.com/ebajec/general-visualizer)
As a hobby project, I wanted to try my hand at creating a simplified 3D rendering pipeline, so I used some tools from the Win32 API to create a blank window on which I could edit pixel colors.  From scratch, I developed the tools required for everything here, from some linear algebra stuff to the functions for drawing 2D shapes.  

# Examples  
Each of these executables is an old build.  To move the camera, use WASD + SPACE/SHIFT.  Please note that these are not stable in any way, shape, or form, so be prepared for potential weirdness.

**cool shape:** This is just a cool shape I made by transforming a sphere.  

**cube jumping:** I was playing around with collision and made a little game-like thing where you can jump on and run into a bunch of cubes.  

*Note:* There is a strange bug I couldn't fix where you pass through the first cube if you don't move before falling on it.  

**wave:** I made this by overlapping a bunch of 2D wave functions on a surface.  

**basic shading:**  This version draws the faces for a bunch of cubes and uses a flat shading techniques to draw them.  There are still some issues with calculating the surface normals correctly, and I have not yet finished the clipping for faces.  

*Note:* The program with attempt to draw an infinitely large quadrilateral and crash if the camera plane intersects a cube, so maybe avoid doing that.   
