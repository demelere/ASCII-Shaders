use image::{DynamicImage, GenericImageView, ImageBuffer, Rgba};

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