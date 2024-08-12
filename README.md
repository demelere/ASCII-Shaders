Adapting Acerola FX's ASCII shader suite to pre-rendering video clips as ASCII animations for web assets. 

# Workflow 
How to apply the shader to each frame of the video.

* Download the video: use yt-dlp `yt-dlp "url" -f "bestvideo[ext=mp4]+bestaudio[ext=m4a]/mp4" -o "name.mp4" `
* Extract frames: use ffmpeg to extract frames from a video called "name" `ffmpeg -i name.mp4 -vf fps=30 data/frame%04d.png` into a directory called `data`
* Set up rendering environment: need a graphics programming environment that supports shader programming and apply HLSL shaders to images. Some options would be: 
    * DirectX-based application (HLSL is typically used with DirectX).  But DirectX only has native support in Windows, unless you use Wine or DirectX-to-OpenGL.
    * Unity game engine (with shader support)
    * GLSL shaders can be used with OpenGL, which is well-supported on macOS.  Use OpenGL Processing framework
        * use glslCanvas to run standalone GLSL shaders directly in HTML elements
        * use Three.js to handle 2D graphics and shaders
        * adapt OpenFrameworks and SFML for web via WebAssembly
    * Vulkan may be too complex?  Initializing the project involves creating Vulkan headers and libraries, instance, device, and setting up pipeline, then rewriting shader code in GLSL or SPIR-V or compiling GLSL to SPIR-V.  
    * Custom C++ Open GL application to load images, apply the shader, and save the output?
    * OR use WebGL and WASM to render the ASCII animation as a series of textures in a WebGL context (on a canvas)
* Use Processing sketch to apply shader to frames:
    * Load each frame as a texture
    * Apply the ASCII shader
    * Save the resulting image

* Compile processed frames: use ffmpeg to compile the processed images back into a video `ffmpeg -framerate 30 -i processed_frame%04d.png -c:v libx264 -pix_fmt yuv420p output_ascii.mp4`

* GLSL shader file `ascii_shader.glsl`
* Processing main sketch `ascii_video_generator.pde`
* ASCII texture generation `ascii_texture_generator.pde`
* `ascii_config.json`
* Create bash scripts


## Build Process

1. Clean the build directory (if it exists): `rm -rf build`
2. Create a new build directory and navigate into it: `mkdir build && cd build`
3. Generate the build files using CMake: `cmake ..`
4. Build the project: `make`
5. Run the executable: `./AsciiShader`
## Inserting/Linking the Image File
1. Place your input image file in the `assets` directory within the project root.
2. In the `main.cpp` file, locate the `loadTexture` function call and update the file path: `unsigned int inputTexture = loadTexture("../data/your_image_file.png");`
3. If you want to process multiple images or use a different directory, you can modify the `loadTexture` function call accordingly:
4. Rebuild the project after making changes:
   ```
   cd build
   make
   ```
5. Run the executable again to process the new image:`./AsciiShader`

Note: Make sure the image file format is supported by the `stb_image` library (e.g., PNG, JPG, BMP). If you encounter any issues loading the image, check the console output for error messages.

# Transcript
## Introduction

Back in the golden era of gaming, the 2000s, if you wanted help with a game, you would consult GameFAQs. These guides had a culture of fancy artistic headers composed of only text, a graphic design technique known as ASCII Art. If you're around my age, these GameFAQs guides may have been your first introduction to ASCII Art, but the technique goes all the way back to 1966 and even further back to 1867 if you include typewriters.

## The Future of ASCII Art in Gaming

But what I'm thinking about is the future. Could we render a game as ASCII Art? And if we did, would it even look good? Would it even be playable? Today, we'll be turning characters into characters. As far as I'm aware, there aren't any games that use an ASCII Art Shader for its main art direction, but some games have basic ASCII Art Shaders available in their photo mode, such as Returnal.

## Issues with Current ASCII Art Shaders

Returnal's ASCII Art Shader is what inspired me, because quite frankly, it doesn't look very good. While it does accomplish the simple task of converting the image to text, so much detail is lost that everything becomes formless blobs, requiring your brain to fill in a lot of information that is now absent. To mitigate this loss of detail, the shader uses the underlying image to color the overlaid text characters, which helps a bit to separate the shapes from each other, but I think it looks kind of ugly and deviates too far from the traditionally monochromatic style of ASCII Art.

## Enhancing the ASCII Art Shader

Unfortunately, this loss of detail is a requirement for the shader, so we must rely on the characters alone to communicate as much detail as possible. If we compare the Returnal ASCII Shader to some real ASCII Art, we can see one pretty big difference: defined contour constructed with slashes, underscores, and vertical bars to communicate edge flow and visual boundaries. If Returnal had these edge lines, then the formless blob dilemma would be solved. But is it possible to make a shader that draws edges with the proper ASCII characters?

## Creating a Custom ASCII Art Shader

### The Basic ASCII Shader

Before we get to the fancy edges, we need to recreate the basic Returnal ASCII shader which forms the foundation of the effect. So, how does Returnal convert this base image into text? First, we need to decide on the size of our text characters or the area that they will take up. In order to conserve as much image detail as we can, we want our characters to be as small as possible but just big enough to be legible. For some reason, Returnal uses 10x10 characters, which is pretty strange for reasons that'll make more sense later. I chose to go with 8x8 characters because it's a pleasant power of 2 and it's what I found to be the smallest possible resolution for legible text.

Now that we know the size of our characters, the idea is that the pixels of the image are going to be replaced with text. As you can see though, the pixels aren't the same size as the letters. A pixel is 1x1 but our text is 8x8, so we need to modify the pixels to match the size of the characters. We could super sample the image and increase the size of the image by 8 times, but in a game context, we'd go right back to square one as the render gets downscaled to fit your monitor. Unfortunately, this means we need to downscale the image and destroy enough image detail to create pixels that are the same size as the text. This is pretty easy: we just divide the dimensions of the image by the resolution of the text. In our case, we divide by 8, which, as you can see, results in a lot of lost detail.

Once we upscale back, the pixels are now equivalent in size to the ASCII characters. Now we just need to establish a relationship between the pixel and ASCII character. This could be any possible relation, but the simplest and best option for us here is relating the luminance or brightness of the pixel to a specific character. De-saturating the image gives us the luminance, but this brings up another problem. There's a lot of different possible luminance values, way more than there are ASCII characters, so to make our lives easier, we want to reduce or quantize the luminance to a smaller set of possible values. This will also let us control how many ASCII characters there are. I wanted 10, so we need to take all the possible luminance values and translate them to 1 of 10 values. Since luminance is already a value between 0 and 1, we can quantize it down to 10 possible outputs by multiplying by 10, taking the floor, and dividing by 10. Now our image is composed of only 10 luminance values, and it's ready to be replaced with ASCII characters.

To do this, we need a texture that contains the text. We want our text to communicate luminance, so a dark pixel should be a tiny letter or symbol, and a bright pixel should be a large letter or symbol. This is the ASCII texture I created for my effect. A near-black pixel is represented with a space, which is why you see only 9 characters. Now, to replace the pixels with the corresponding ASCII character, we use the luminance as the coordinate for where to start in the texture. We take the pixel's screen position, mod by 8, to put it in the 8x8 range of the ASCII character size, and add the luminance to the horizontal texture coordinate to get the proper character. That's really all there is to the basic ASCII shader. To make it look more like Returnal, all we do is multiply by the downscaled color data.

While it certainly looks cool already, it's kind of just a glorified halftoning shader, even though it's technically all text. To fix this, I wanted to try adding those edge lines, but how do we even start with something like that?

### Adding Edge Detection

Before we try to solve the edge problem, let's clarify what exactly we need. Just edges alone won't satisfy the needs of the ASCII shader. We also need to draw the edge with the proper symbol that conforms to the contour. For example, a flat edge should be drawn with an underscore, a vertical edge should be drawn with a vertical bar, and for angled edges, we should use the corresponding slash. This means we don't just need edge data, we also need to know the angle of the edge, which makes things a bit more complicated.

I've presented numerous edge detection methods in my previous videos already, so as a quick recap, here are our options. For detecting edges on images alone, we have the Sobel filter, the Canny edge detector, and the difference of Gaussians. In a games context, where we have more data to work with, we have edge detection through depth and normal difference, and the inverse hull method. This is not a comprehensive list of all edge detection methods, just the ones I've talked about before. Thankfully, the one we need is already present, because only one of these also gives us the direction of the edge, the Sobel filter. That's because the Sobel filter isn't really an edge detector; it's an approximation of the image gradient. When you hear gradient, you might think of a blend between two colors, but in vector calculus, the gradient of a function is a vector field, where each vector points in the direction of greatest change. The Sobel filter, when convolved with our image, gives us this gradient vector for each pixel.

As you can see, there is only change detected along the border of our circle, with no change on the flat color areas. Since these are vectors in two-dimensional space, we can get the angle of the vector with the wonderful ATAN2 function, giving us an angle between negative pi and pi, which we can convert to 0 to 1 for simplicity. We now have the same problem we had earlier with the luminance: there's a lot more angle values than there are edge ASCII characters, so we again want to reduce the possible angle results to four values. Then, we downscale the image to the size of the ASCII characters like before, and use the quantized angle as the texture coordinate for the edge ASCII texture, and we have amazing ASCII edges.

## Enhancing the Edge Detection

It works! Kind of. The edges may be lackluster, but the proof of concept is there. The edges follow the contour of the original image. Unfortunately, we can hardly call these edges though. There's just too many gaps and holes in the edge line, which is unacceptable for a simple circle example. Our shader should be able to draw a circle just fine. Using the GPU to downscale and upscale the quantized angle buffer results in poorly conserved edges. Clearly, we'd like a little more control over the output to keep the downscaled edges as cohesive as possible.

To get that control, we can use a handy dandy compute shader. As a quick review, Compute Shaders are GPU programs for generic computation. The work is executed in groups, and you, the programmer, get to decide on how many groups are dispatched and how large the groups are. An ideal group size is 8x8, for 64 total threads, coincidentally the same size as our ASCII text. Then we simply dispatch enough groups to cover the full resolution edge data, and our compute shader will decide if a tile should be counted as an edge in the downscaled output or not.

I did this by having each thread determine which kind of edge its pixel is, writing that to group shared memory. Then the first thread of the group would scan the group shared memory using the identified edge as an index for another array, adding to that spot in the array. Afterwards, it would find the max value in that array and use that as the edge that fills the downscaled pixel. Basically, we're constructing a local histogram of the edge data to figure out which kind of edge is most common in the 8x8 tile, and using that as the simplified edge output. I added a threshold parameter so that the tile needs a certain amount of edge pixels within it to be counted as an edge, and the result is a downscaled edge buffer that has far more cohesive edge lines than the original GPU downscaled. The circle is pretty much perfectly converted into edges that follow the contour, but what about a more complicated picture? Yeah, that looks pretty bad.

## Improving Edge Quality

This is because our Sobel filter isn't actually edge detecting. To convert the Sobel filter into an edge detector, we have to threshold the magnitude of the gradient. Essentially, if a high change is detected, then it's an edge; otherwise, it's not. This fixes the render, but the problem with this is that the edges look like complete ass. The Sobel filter is not meant to produce aesthetically pleasing edges; it's meant to extract features for data analysis. We can fix this by adding another pre-processing step to our effect. With a simple difference of Gaussians, we can extract high-frequency details of the image in a way that looks more pleasant, and then run the Sobel filter on that for edge direction data. As you can see, the difference of Gaussians edge lines looks far better than the Sobel filter edges. In another video of mine, I demonstrated how the difference of Gaussians can be used for a wide variety of different art styles just by itself. But here it takes a backseat as a simple pre-processing step to improve the visuals and reliability of the ASCII shader.

## Finalizing the ASCII Shader

Now all we have to do is layer the edges on top of the base ASCII pass, and the Ace Rola ASCII shader is all finished. This is about as far as we can take the ASCII shader from a pure image processing standpoint, but these few example renders show the promise of the effect. Static images are kind of boring though; I wanted to see the effect in motion, so I implemented what we have so far as a reshade shader so we can use the effect on some games. Thankfully, we can make the effect look even cooler in a 3D context.

## Testing the Shader in Games

Over in Final Fantasy XIV, the effect looks exactly the same, because it would be weird if it didn't, but we can see that in motion the edges stay visually cohesive over time and it looks surprisingly really good. In a 3D context, we can make the edges even more reliable by checking for substantial differences in depth and normal values on neighboring pixels, which will add some more edge lines that the difference of Gaussian fails to capture. This makes the clothing of my character look a lot better, so it's definitely worth the extra edge detection step.

## Addressing Eye Fatigue

One significant issue with the shader though is that the high contrast causes eye fatigue pretty quickly. Staring at this effect for a while just does not feel good, especially when you're walking around and trying to actually parse the visual information. Because of this, the ASCII shader is probably best left as a photo mode effect like how Returnal uses it, but there are some things we can do to remedy the eye fatigue. The color of the ASCII letters and the background can be easily controlled; instead of using black and white, we can use a lower contrast color combo. Another idea I had was to fade the letters out based on depth, which I think looks pretty good and could make it more useful for an actual game. The last option I added was for not drawing edges after a certain distance, which has a sort of depth of field effect. At this point, I was satisfied with the effect and didn't need anything more. It also runs pretty fast already, so we don't need to do any real optimizing.

## Experimenting with Different Effects

So now we can experiment with the shader and see what effects it looks good with, as well as where the shader looks good and where it looks bad. We've looked at Final Fantasy a lot, so let's look at something a bit more modern, Elden Ring, and see how the ASCII shader handles some more complex visuals. Because ASCII Art is a computer thing, it's probably best combined with other effects that evoke tech aesthetics. You could go the retro route with color quantization to try and improve the colored ASCII art from Returnal, then layer a CRT filter on top, and we already have a trendy indie game filter that would go crazy on Reddit. You could keep it minimalist and try to make the game look like it's printed in the command prompt, which really is just the base ASCII shader already with different colors.

## Advanced Visual Effects

I wanted to go the epic neon cyberpunk route though, because the ASCII shader is just asking to be bloomed. I didn't want it to look like it's glowing, I wanted it to look burnt, so I added some tone mapping to increase the contrast, sharpened it to make it crispier, and then color burnt the image. I really like how this looks, and it makes me think of Deus Ex for some reason. With some creative framing and vignettes, you can make the game look even more novel. This would look pretty cool for like a point-and-click adventure game. The shader looks pretty nice on people; the edge detection makes hair look really cool, and I particularly love how eyes look with the shader applied.

## Limitations and Challenges

But when does the shader not work? You've probably noticed that all my example photos have been close-ups or very zoomed in. This is because the effect looks best at this close distance. When you zoom out, the ASCII characters get a lot noisier and everything jumbles together. Stuff like faces at this distance are impossible to convert to ASCII because, remember, we are losing this much detail no matter what. But the edges do help identify separate shapes, so with the depth follow I mentioned earlier, the game isn't entirely unplayable. If you were to further separate entities with flat colors, you could probably create some cool Tron or super hot like visuals.