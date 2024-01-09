# renderer-old
*Note:* I am no longer working on this, just leaving it up.

As a hobby project, I wanted to make a renderer, so I used the Windows API to make one from absolute scratch.   

# Examples  
Each of these executables is an old build.  To move the camera, use WASD + SPACE/SHIFT.  Please note that these are not stable in any way, shape, or form, so be prepared for potential weirdness.

**cool shape:** This is just a cool shape I made by transforming a sphere.  

**cube jumping:** I was playing around with collision and made a little game-like thing where you can jump on and run into a bunch of cubes.  

*NOTE:* There is a strange bug I couldn't fix where you pass through the first cube if you don't move before falling on it.  

**wave:** Cool wavy thing that looks cool.  

**basic shading:**  This version draws the faces for a bunch of cubes and uses a flat shading techniques to draw them.

*NOTE:* The program with attempt to draw an infinitely large triangle and crash if the camera plane intersects a cube, because I did not add clipping.   
