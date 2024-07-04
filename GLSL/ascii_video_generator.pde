import java.io.File;

PShader asciiShader;
PImage currentFrame;
File[] frames;
int frameIndex = 0;
PGraphics pg;
PImage edgesASCII, fillASCII, normalsTexture;

void setup() {
  size(1280, 720, P2D);
  println("Setup started");
  
  // Load the shader
  try {
    asciiShader = loadShader("ascii.glsl");
    println("Shader loaded successfully");
  } catch (Exception e) {
    println("Error loading shader: " + e.getMessage());
  }
  
  // Get list of frames
  File dir = new File(dataPath(""));
  frames = dir.listFiles(file -> file.getName().startsWith("frame") && file.getName().endsWith(".png"));
  if (frames != null) {
    println("Found " + frames.length + " frames");
  } else {
    println("No frames found in data directory");
  }
  
  // Create a PGraphics object for offscreen rendering
  pg = createGraphics(width, height, P2D);
  
  // Load ASCII textures
  try {
    edgesASCII = loadImage("edgesASCII.png");
    fillASCII = loadImage("fillASCII.png");
    normalsTexture = createImage(width, height, RGB); // Create a dummy normals texture
    
    // Set up shader uniforms
    asciiShader.set("EdgesASCII", edgesASCII);
    asciiShader.set("FillASCII", fillASCII);
    asciiShader.set("Normals", normalsTexture);
    
    // Set other uniforms
    asciiShader.set("_Zoom", 1.0f);
    asciiShader.set("_Offset", 0.0f, 0.0f);
    asciiShader.set("_KernelSize", 2);
    asciiShader.set("_Sigma", 2.0f);
    asciiShader.set("_SigmaScale", 1.6f);
    asciiShader.set("_Tau", 1.0f);
    asciiShader.set("_Threshold", 0.005f);
    asciiShader.set("_UseDepth", true);
    asciiShader.set("_DepthThreshold", 0.1f);
    asciiShader.set("_UseNormals", true);
    asciiShader.set("_NormalThreshold", 0.1f);
    asciiShader.set("_DepthCutoff", 0.0f);
    asciiShader.set("_EdgeThreshold", 8);
    asciiShader.set("_Edges", true);
    asciiShader.set("_Fill", true);
    asciiShader.set("_Exposure", 1.0f);
    asciiShader.set("_Attenuation", 1.0f);
    asciiShader.set("_InvertLuminance", false);
    asciiShader.set("_ASCIIColor", 1.0f, 1.0f, 1.0f);
    asciiShader.set("_BackgroundColor", 0.0f, 0.0f, 0.0f);
    asciiShader.set("_BlendWithBase", 0.0f);
    asciiShader.set("_DepthFalloff", 0.0f);
    asciiShader.set("_DepthOffset", 0.0f);
    
    println("Shader uniforms set");
  } catch (Exception e) {
    println("Error loading textures or setting uniforms: " + e.getMessage());
  }
  
  println("Setup completed");
}

void draw() {
  if (frameIndex < frames.length) {
    // Load the next frame
    currentFrame = loadImage(frames[frameIndex].getAbsolutePath());
    
    pg.beginDraw();
    pg.shader(asciiShader);
    asciiShader.set("texture", currentFrame);
    pg.image(currentFrame, 0, 0, width, height);
    pg.endDraw();
    
    image(pg, 0, 0);
    
    // Save the processed frame
    pg.save("output/ascii_frame_" + nf(frameIndex, 4) + ".png");
    
    println("Processed frame " + (frameIndex + 1) + " / " + frames.length);
    
    frameIndex++;
  } else {
    println("All frames processed. Exiting.");
    exit();
  }
}