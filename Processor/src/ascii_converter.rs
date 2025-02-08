use image::{DynamicImage, GenericImageView, ImageBuffer, Rgba};

pub struct ColorPalette {
    pub name: &'static str,
    pub colors: Option<Vec<[u8; 3]>>,
    pub fg: Option<[u8; 3]>,
    pub bg: Option<[u8; 3]>,
}

pub const PALETTES: &[ColorPalette] = &[
    ColorPalette {
        name: "original",
        colors: None,
        fg: None,
        bg: None,
    },
    ColorPalette {
        name: "bw",
        colors: None,
        fg: Some([0, 0, 0]),
        bg: Some([255, 255, 255]),
    },
    ColorPalette {
        name: "terminal",
        colors: Some(vec![
            [7, 54, 66],
            [88, 110, 117],
            [101, 123, 131],
            [131, 148, 150],
            [147, 161, 161],
            [238, 232, 213],
            [253, 246, 227],
        ]),
        bg: Some([0, 43, 54]),
        fg: None,
    },
    ColorPalette {
        name: "amber",
        colors: Some(vec![
            [255, 176, 0],
            [255, 192, 0],
            [255, 208, 0],
            [255, 224, 0],
        ]),
        bg: Some([0, 0, 0]),
        fg: None,
    },
    ColorPalette {
        name: "low_contrast",
        colors: Some(vec![
            [205, 205, 205],
            [180, 180, 180],
            [155, 155, 155],
            [130, 130, 130],
        ]),
        bg: Some([230, 230, 230]),
        fg: None,
    },
    ColorPalette {
        name: "nord",
        colors: Some(vec![
            [216, 222, 233],
            [229, 233, 240],
            [236, 239, 244],
        ]),
        bg: Some([46, 52, 64]),
        fg: None,
    },
    ColorPalette {
        name: "catppuccin",
        colors: Some(vec![
            [245, 224, 220],
            [242, 205, 205],
            [245, 194, 231],
            [203, 166, 247],
            [243, 139, 168],
            [235, 160, 172],
            [250, 179, 135],
            [249, 226, 175],
            [166, 227, 161],
            [148, 226, 213],
            [137, 220, 235],
            [116, 199, 236],
        ]),
        bg: Some([30, 30, 46]),
        fg: None,
    },
];

pub fn hex_to_rgb(hex: &str) -> [u8; 3] {
    let hex = hex.trim_start_matches('#');
    let r = u8::from_str_radix(&hex[0..2], 16).unwrap_or(0);
    let g = u8::from_str_radix(&hex[2..4], 16).unwrap_or(0);
    let b = u8::from_str_radix(&hex[4..6], 16).unwrap_or(0);
    [r, g, b]
}

pub fn get_color_for_brightness(brightness: u8, palette_name: &str, original_color: [u8; 3]) -> [u8; 3] {
    if palette_name == "original" {
        return original_color;
    }

    let palette = PALETTES.iter().find(|p| p.name == palette_name).unwrap_or(&PALETTES[0]);

    if let Some(colors) = &palette.colors {
        let index = (brightness as f32 / 255.0 * (colors.len() - 1) as f32) as usize;
        colors[index.min(colors.len() - 1)]
    } else if let (Some(fg), Some(bg)) = (palette.fg, palette.bg) {
        let factor = brightness as f32 / 255.0;
        [
            ((1.0 - factor) * bg[0] as f32 + factor * fg[0] as f32) as u8,
            ((1.0 - factor) * bg[1] as f32 + factor * fg[1] as f32) as u8,
            ((1.0 - factor) * bg[2] as f32 + factor * fg[2] as f32) as u8,
        ]
    } else {
        original_color
    }
}

pub fn to_grayscale(img: &DynamicImage) -> ImageBuffer<Rgba<u8>, Vec<u8>> {
    let (width, height) = img.dimensions();
    let mut output = ImageBuffer::new(width, height);

    for y in 0..height {
        for x in 0..width {
            let pixel = img.get_pixel(x, y);
            let gray = ((pixel[0] as f32 * 0.299) +
                       (pixel[1] as f32 * 0.587) +
                       (pixel[2] as f32 * 0.114)) as u8;
            output.put_pixel(x, y, Rgba([gray, gray, gray, pixel[3]]));
        }
    }
    output
}

pub fn detect_edges(img: &DynamicImage, sigma1: f32, sigma2: f32) -> ImageBuffer<Rgba<u8>, Vec<u8>> {
    let gray = to_grayscale(img);
    let (width, height) = gray.dimensions();
    let mut output = ImageBuffer::new(width, height);

    // Sobel operators
    let sobel_x = [[-1.0, 0.0, 1.0],
                   [-2.0, 0.0, 2.0],
                   [-1.0, 0.0, 1.0]];
    
    let sobel_y = [[-1.0, -2.0, -1.0],
                   [0.0,  0.0,  0.0],
                   [1.0,  2.0,  1.0]];

    for y in 1..height-1 {
        for x in 1..width-1 {
            let mut gx = 0.0;
            let mut gy = 0.0;

            // Apply Sobel operators
            for i in 0..3 {
                for j in 0..3 {
                    let pixel = gray.get_pixel(x + (i as u32) - 1, y + (j as u32) - 1)[0] as f32;
                    gx += pixel * sobel_x[i][j];
                    gy += pixel * sobel_y[i][j];
                }
            }

            let g = ((gx * gx + gy * gy).sqrt() * sigma1).min(255.0) as u8;
            output.put_pixel(x, y, Rgba([g, g, g, 255]));
        }
    }

    output
}

pub fn get_ascii_char(brightness: u8, chars: &str, invert: bool) -> char {
    let index = if invert {
        (brightness as f32 / 255.0 * (chars.len() - 1) as f32) as usize
    } else {
        ((255 - brightness) as f32 / 255.0 * (chars.len() - 1) as f32) as usize
    };
    chars.chars().nth(index).unwrap_or(' ')
}

pub fn apply_dithering(img: &mut ImageBuffer<Rgba<u8>, Vec<u8>>) {
    let (width, height) = img.dimensions();
    for y in 0..height-1 {
        for x in 0..width-1 {
            let old_pixel = img.get_pixel(x, y)[0] as i32;
            let new_pixel = if old_pixel > 127 { 255 } else { 0 };
            let error = old_pixel - new_pixel;

            // Floyd-Steinberg dithering
            if x < width-1 {
                let p = img.get_pixel(x+1, y)[0] as i32 + (error * 7 / 16);
                img.put_pixel(x+1, y, Rgba([p.clamp(0, 255) as u8; 4]));
            }
            if x > 0 && y < height-1 {
                let p = img.get_pixel(x-1, y+1)[0] as i32 + (error * 3 / 16);
                img.put_pixel(x-1, y+1, Rgba([p.clamp(0, 255) as u8; 4]));
            }
            if y < height-1 {
                let p = img.get_pixel(x, y+1)[0] as i32 + (error * 5 / 16);
                img.put_pixel(x, y+1, Rgba([p.clamp(0, 255) as u8; 4]));
            }
            if x < width-1 && y < height-1 {
                let p = img.get_pixel(x+1, y+1)[0] as i32 + (error * 1 / 16);
                img.put_pixel(x+1, y+1, Rgba([p.clamp(0, 255) as u8; 4]));
            }

            img.put_pixel(x, y, Rgba([new_pixel as u8; 4]));
        }
    }
}

pub fn adjust_brightness(img: &mut ImageBuffer<Rgba<u8>, Vec<u8>>, brightness: f32) {
    for pixel in img.pixels_mut() {
        for c in 0..3 {
            pixel[c] = (pixel[c] as f32 * brightness).clamp(0.0, 255.0) as u8;
        }
    }
}

pub fn auto_adjust_brightness(img: &mut ImageBuffer<Rgba<u8>, Vec<u8>>) {
    let mut min = 255;
    let mut max = 0;

    // Find min and max values
    for pixel in img.pixels() {
        let value = pixel[0];
        min = min.min(value);
        max = max.max(value);
    }

    // Apply contrast stretching
    let range = (max - min) as f32;
    if range > 0.0 {
        for pixel in img.pixels_mut() {
            for c in 0..3 {
                pixel[c] = (((pixel[c] as f32 - min as f32) / range) * 255.0) as u8;
            }
        }
    }
} 