PShader asciiShader;
PImage currentFrame;
int frameCount = 0;

void setup() {
  size(1280, 720, P2D);
  asciiShader = loadShader("ascii.glsl");
  asciiShader.set("resolution", float(width), float(height));
}

void draw() {
  // Load next frame
  currentFrame = loadImage("frame" + nf(frameCount, 4) + ".png");
  
  if (currentFrame == null) {
    // No more frames, exit
    exit();
    return;
  }
  
  // Apply shader
  shader(asciiShader);
  asciiShader.set("texture", currentFrame);
  asciiShader.set("time", millis() / 1000.0);
  
  // Draw the image with the shader applied
  image(currentFrame, 0, 0, width, height);
  
  // Save the result
  save("processed_frame" + nf(frameCount, 4) + ".png");
  
  frameCount++;
}