PGraphics createASCIITexture() {
  String chars = " .:-=+*#%@";
  PGraphics pg = createGraphics(chars.length() * 8, 8);
  pg.beginDraw();
  pg.background(0);
  pg.fill(255);
  pg.textSize(8);
  for (int i = 0; i < chars.length(); i++) {
    pg.text(chars.charAt(i), i * 8, 7);
  }
  pg.endDraw();
  return pg;
}